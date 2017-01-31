#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>

//Globals
//Array of strings, 20 strings of 25 length
struct tag{
    int id; 
    char name[30];
    char ip[30];
    char port[10];
};
struct tag connects[5];
int amountCon;
char hostIP[30];
//Listening thread
void *listening(void* data){
    int *port = (int*)data;
    int listenF; 
    int response;
    char word[100];

    //create Socket
    struct sockaddr_in servaddr;
    listenF = socket(AF_INET, SOCK_STREAM, 0);
    //clear struct
    bzero(&servaddr, sizeof(servaddr));
    //set up
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    //added port to it
    servaddr.sin_port = htons(*port);

    bind(listenF, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //Start Listening!
    listen(listenF,5);
    
    //This will be in the loop
    while(1){
        response = accept(listenF, (struct sockaddr*) NULL, NULL);
        bzero(word, 100);
        //read(response,word, 100);
        printf("SERVER: Connection Accepted\n");
        //This is the part were we parse the command and write back correct information
        //write(response, word,strlen(word) +1);
        
        //TODO: Update List, update max connections
        //TODO: check if max connections
        //TODO: chat between?
    } 
    return NULL;
}
//connect to an IP/Port
//Return 1 on success, 0 on fail
int connect(){
    //Check amount of connections
    if (amountCon >= 5) {
        //5 connectsions, stop!
        printf("Max Connections Reached!\n");
        return 0;
    }
    char hostN[30];
    char portNum[30];
    printf("Enter Host Name:\n");
    scanf("%s",hostN);
    printf("Enter Port Number:\n");
    scanf("%s",portNum);
    printf("Inputted Host Name: %s\nInputted Port Number: %s\n",hostN,portNum);

    //TODO: Check duplicate connection attempt and if IP is localhost
    //start socket
    //socket vars
    int sockfd, portn, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    //Get port num
    try{
        portn = atoi(portNum);
    } catch (...){
        printf("Inputed Port does not work, using 8005\n");
        portn = 8005;
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0){
        printf("ERROR opening socket\n");
        int again = 0;
        while (sockfd < 0 && again <5){
            printf("Attempting another connection\n");
            again++;
            sockfd = socket(AF_INET, SOCK_STREAM,0);
            if (sockfd >= 0)
                break;
        }
        return 0; 
    }
    //get server name
    server = gethostbyname(hostN); 
    if (server==NULL){
        printf("ERROR, no such host\n");
        return 0;
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portn);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return 0;
    printf("Connection Accepted"); 

    // update counter
    amountCon++;
    //add to list
    int x;
    for (x = 0; x < 5;++x){
        //find first empty one
        if (connects[x].id == -1) {
            connects[x].id = x;
            strcpy(connects[x].name,server->h_name);
            strcpy(connects[x].ip,(char*)server->h_addr);
            strcpy(connects[x].port,portNum);
            //stop loop early
            break;
        }
    }
    return 1;
}

void fetchIP(){
    FILE *fp;
    //write ip to file, read file, delete file
    system("ifconfig | grep inet | awk '{print $2}'  | head -1 >> ip ");
    fp = fopen("ip","r");
    char myIp[30];
    fgets(myIp,30,fp);
    char *ip = myIp;
    system("rm -f ip");
    //remove ip file
    strcpy(hostIP,myIp);
}
//Starts listening Code, Starts UI thread
int main(int argc, char *argv[]){
    int port = 0;
    amountCon = 0;
    hostIP[0] = 0;
    // Get Port from user, checks it
    if (argc <= 1) {
        printf("No port given, assuming port 8005\n");
        port = 8005; 
    } else {
            port = atoi(argv[1]);
        if (port == 0 || port < 1024){
            printf("Illegal Port Entered, assuming port 8005\n");
            port = 8005;
        }
        printf("Assigned Port is %d\n",port);
    }
    
    //init array of connections
    int x = 0;
    for (x = 0; x < 5; ++x){
        connects[x].id = -1;
        strcpy(connects[x].name,"");
        strcpy(connects[x].ip,"");
    }
    //Create pthread objects
    pthread_t listen;
    //Starts listening Thread
    pthread_create(&listen, NULL,listening,&port);

    //Start UI
    while (1) {
        char *input;
        *input = 0;
        printf("Choose an option, input corresponding number:\n(1) HELP\n(2) EXIT\n(3) MYIP\n(4) MYPORT\n(5) CONNECT\n(6) LIST\n(7) TERMINATE\n(8) ISALIVE\n(9) CREATOR\n");

        scanf("%c",input); 
        switch(*input){
            case '1':
                //DONE
                printf("Enter 1-9 corresponding to commands\n" 
                        "1: Help\n" 
                        "2: Exit\n"
                        "3: Fetches IP Address of current server\n"
                        "4: Fetches Port of Current program\n"
                        "5: Attemps a Connection, promps for IP and Port address\n"
                        "6: Lists current Connections To and From this Server\n"
                        "7: \n"
                        "8: \n"
                        "9: Prints Creator Information\n");
                break;
            case '2':
                //TODO get rid of all the streams
                //call terminate on everything in list
                printf("Thank you for using this program\n");
                exit(0);
                break;
            case '3':
                //DONE
                fetchIP();
                printf("This server is running on the IP: %s\n",hostIP);
                break;
            case '4':
                //DONE
                printf("Server port is %d\n",port);
                break;
            case '5':
                //Connect
                int response;
                response= connect();
                if (response == 0)
                    printf("Connection Failed\n");
                break;
            case '6':
                //TODO: Error when printing list
                printf("Printing List of Connections:\n");
                for (int x = 0; x < 5; ++x) {
                    //print list
                    if (connects[x].id != -1){
                        printf("Connection %d: %s : %s\n",
                            connects[x].id,connects[x].name,connects[x].ip);
                    }
                }
                break;
            case '7':
                //TODO: get user input
                break;
            case '8':
                //TODO: get user input
                break;
            case '9':
                //DONE
                printf("CREATOR: Georges Brantley, EMAIL: gbrantle@purdue.edu\n");
                break;
            default:
                printf("Input not recognized. Please insert a number 1-9!\n");
        }
        scanf("%c",input);
        printf("\n");
    }


    pthread_join(listen, NULL);
    return 0;
}

