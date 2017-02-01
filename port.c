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
#include <arpa/inet.h>
//Globals
//MAX CONNECTIONS
#define N 5

//Array of strings, 20 strings of 25 length
struct tag{
    int id; 
    char name[128];
    char ip[128];
    char port[10];
};
struct tag connects[N];
int amountCon;

char hostIP[30];
char hostName[128];
char hostPort[10];

//TODO: GO OVER REQUIREMENTS
//TODO: ERROR MESSAGES
//TODO: README

//deletes connection, c is index in connects
void deleteConnection(int c){
    //delete it in the connections list
    connects[c].id = -1;
    strcpy(connects[c].name, "");
    strcpy(connects[c].ip, "");
    strcpy(connects[c].port,"");
    amountCon--;
    if (amountCon != 0){
        printf("HEYHEY\n");
        //move shit down 
        for (int y = 0; y<N; ++y){
        for (int x = 0; x < N-1; ++x) {
            if(connects[x].id == -1 && connects[x+1].id !=-1) {
                printf("YOUYOU\n");
                //sift down
                connects[x] = connects[x+1];
                connects[x].id--;
                connects[x+1].id = -1;
                strcpy(connects[x+1].ip,"");
                strcpy(connects[x+1].name,"");
                strcpy(connects[x+1].port,"");

            }
       }
       }
    }
}

void *listening(void* data){
    int *port = (int*)data;
    int listenF; 
    int response;
    char word[1000];

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
    listen(listenF,N);
    
    //This will be in the loop
    while(1){
        response = accept(listenF, (struct sockaddr*) NULL, NULL);
        bzero(word, 1000);
        read(response,word, 1000);
        //Bypass, checking if stream is alive
        if (strcmp(word,"Alive")==0) {
            write(response,"ALIVE",5);
        }
        else if (strstr(word,"Delete-") != NULL) {
           //Delete Instance in Connects 
           char *w = word;
           char *forName, *forIP;
           w += 7;
           forName = w;
           int flag = 0;
           while (*w != 0) {
                if (*w == '-') {
                    *w = 0;
                    if (flag == 0) {
                        forIP = (w+1);
                        flag++;
                    }
                }
                ++w;
            }
            //armed with knowledge, delete it from the pool!
            for (int x = 0; x < N; ++x){
                if (strcmp(connects[x].name,forName) ==0) {
                    printf("SERVER: Connection %d (%s) Deleted!\n",x+1, connects[x].name);
                    deleteConnection(x);
                    break;
                }
            }
        } else if (amountCon >=N){
            //Check max connections
            printf("Foreign Connection Rejected: Full Connections");
            bzero(word,10000);
            strcpy(word,"Denied");
            write(response,word, strlen(word));
        } else {
            //If there are connections to be had
            write(response,"Accepted",8);
            //This is the part were we parse the command and write back correct information
            char *w = word;
            //Get the info sent
            //RECIEVED: NAME-IP-PORT-, add to list
            //increase amount connected
            amountCon++; 
            //variables for foriegn name, ip and port
            char *forName, *forIP, *forPort;
            int flag =0;
            forName = w; 
            while (*w != 0) {
                if (*w == '-') {
                    *w = 0;
                    if (flag == 0) {
                        forIP = (w+1);
                        flag++;}
                    else if (flag == 1){
                        forPort = (w+1);
                        flag++;}
                }
                ++w;
            }
            printf("SERVER: Foreign Connection (%s) Accepted\n", forName);
            //ADD DATA TO THE LIST
            for (int x = 0; x < N; ++x) {
                //find first empty one
                if (connects[x].id == -1) {
                    connects[x].id = x;
                    strcpy(connects[x].ip,forIP);
                    strcpy(connects[x].name,forName);
                    strcpy(connects[x].port,forPort);
                    //stop loop early
                    break;
                }
            }
        }
    } 
    return NULL;
}
//connect to an IP/Port
//Return 1 on success, 0 on fail
int connect(){
    //Check amount of connections
    if (amountCon >= N) {
        //5 connectsions, stop!
        printf("Max Connections Reached!\n");
        return 0;
    }

    char conN[30];
    char portNum[30];
    printf("Enter Host Name:\n");
    scanf("%s",conN);
    printf("Enter Port Number:\n");
    scanf("%s",portNum);
    printf("Inputted Host Name: %s\nInputted Port Number: %s\n",conN,portNum);

    //if you inputed your own name or ip
    if (strcmp(conN,hostName)==0 || strcmp(conN,hostIP) == 0){
        printf("Error, You cannot connect to yourself\n");
        return 0;
    }
    //Check current connections to see if you are repeating
    for (int x = 0; x < N; ++x) {
        //if there is a connection
        if (connects[x].id != -1) {
            if (strcmp(conN,connects[x].name)==0 || strcmp(conN,connects[x].ip)==0){
                printf("Error, You cannot duplicate Connections\n");
                return 0;
            }
        }
    }
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
    server = gethostbyname(conN); 
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
    
    //Write info
    char info[1000];
    //INFO: NAME IP PORT 
    bzero(info,1000);
    strcat(info,hostName);
    
    strcat(info,"-");
    strcat(info,hostIP);
    strcat(info,"-");
    strcat(info,hostPort);
    strcat(info,"-");
    int a = write(sockfd,info,strlen(info));
    if (a<0)
        return 0;
    bzero(info,1000);
    //get info
    a = read(sockfd, info,9999);
    if (a<0)
        return 0;

    //Check if accepted
    if (strcmp(info,"Accepted") != 0){
        printf("Target Connection Busy\n");
        return 0;
    }

    //If it reaches here, it was accepted!
    printf("Connection Accepted\n"); 

    struct hostent *h = gethostbyname(conN);
    struct in_addr addr;
    // update counter
    amountCon++;
    //add to list
    int x;
    for (x = 0; x < N;++x){
        //find first empty one
        if (connects[x].id == -1) {
            connects[x].id = x;
            memcpy(&addr,h->h_addr_list[0],sizeof(struct in_addr));
            strcpy(connects[x].ip,inet_ntoa(addr));
            strcpy(connects[x].name,conN);
            sprintf(connects[x].port,"%d",portn);
            //stop loop early
            break;
        }
    }
    return 1;
}

void fetchIP(){
    gethostname(hostName,sizeof(hostName));

    printf("Host Name: %s\n",hostName);
    struct hostent *h =  gethostbyname(hostName);
    struct in_addr addr;
    memcpy(&addr,h->h_addr_list[0],sizeof(struct in_addr));
    strcpy(hostIP,inet_ntoa(addr));
}

//prompts for connection number, 
//pings server with Alive, expects response
int isAlive(){
    printf("Please input a valid connection number between 1-%d:\n",N);
    int con;
    con = 0;
    scanf("%d",&con);
    //fix for indexing
    con--;
    //check if connection is valid
    if (connects[con].id == -1) {
        //invalid
        printf("Invalid input\n");
        return -1;
    }
    //valid input
    //create socket to ping server
    int sockfd, portn, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    portn = atoi(connects[con].port);
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0){
        printf("Error Opening Socket");
        return con;
    }
    server = gethostbyname(connects[con].name);
    if (server ==NULL) {
        printf("Connection is Dead\n");
        return con;
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portn);

    //connect!
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Connection is Dead\n");
        return con;
    }

    write(sockfd,"Alive",5);
    char resp[1000];
    read(sockfd,resp,1000);
    if (strstr(resp,"ALIVE") != NULL) {
        printf("Connection is Alive\n");
        return -1;
    }
}

//SendDelete tells the host to delete its connection
//Also deletes home connection
int bigDelete(int c) {
    //send ping to host
    //create socket to ping server
    int sockfd, portn, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    //check for out of bounds
    if (c < 0 || c > N ) {
        printf("invalid Connection Number\n");
        return 0;
    }
    //check for valid connection
    if (connects[c].id == -1) {
        printf("Invalid Connection Number\n");
        return 0;
    }
    portn = atoi(connects[c].port);
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0){
        printf("Error Opening Socket");
        return 0;
    }
    server = gethostbyname(connects[c].name);
    if (server ==NULL) {
        printf("Connection is Dead\n");
        return 0;
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portn);

    //connect!
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Connection is Dead\n");
        return 0;
    }
    char send[1000];
    bzero(send,1000);
    strcat(send,"Delete-");
    strcat(send,hostName);
    strcat(send,"-");
    strcat(send,hostIP);

    //delete it from Server
    //SENT: Delete-Name-IP
    write(sockfd,send,sizeof(send));

    //delete it from home
    deleteConnection(c);
    return 1; 
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
    //make port global
    sprintf(hostPort,"%d",port);
    //init array of connections
    int x = 0;
    for (x = 0; x < N; ++x){
        connects[x].id = -1;
        strcpy(connects[x].name,"");
        strcpy(connects[x].ip,"");
        strcpy(connects[x].port,"");
    }
    //Create pthread objects
    pthread_t listen;
    //Starts listening Thread
    pthread_create(&listen, NULL,listening,&port);
    fetchIP();
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
                        "7: Deletes A Connection, Promps for Connection Number\n"
                        "8: Promps for Connection, Checks if it is still alive\n"
                        "9: Prints Creator Information\n");
                break;
            case '2':
                //DONE
                //call terminate on everything in list
                for (int x = 0; x < N; ++x) {
                    if (connects[x].id != -1) 
                        bigDelete(x);
                }
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
                //List all connections into and outof
                //DONE
                printf("Printing List of Connections:\n");
                for (int x = 0; x < N; ++x) {
                    //print list
                    if (connects[x].id != -1){
                        printf("Connection %d: %s - %s\n",
                            connects[x].id+1,connects[x].name,connects[x].ip);
                    }
                }
                break;
            case '7':
                //Terminate
                int in;
                in= -1;
                printf("Enter the Conneciton Number of the Stream you wish to Delete. 1-%d:\n",N);
                scanf("%d",&in);
                in--;//put it in index
                int r;
                r=bigDelete(in); 
                if (r){
                    in++;//put it to human location
                    printf("Connection %d Has Been Deleted",in);
                }
                break;
            case '8':
                int i;
                i= isAlive();
                if (i != -1)
                    deleteConnection(i);
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

