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
int top;

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
    response = accept(listenF, (struct sockaddr*) NULL, NULL);
    bzero(word, 100);
    read(response,word, 100);
    printf("Command Recieved: %s\n", word);
    //This is the part were we parse the command and write back correct information
    write(response, word,strlen(word) +1);
    
    return NULL;
}
//connect to an IP/Port
//Return 1 on success, 0 on fail
int connect(){
    char hostN[30];
    char portNum[30];
    printf("Enter Host Name:\n");
    scanf("%s",hostN);
    printf("Enter Port Number:\n");
    scanf("%s",portNum);

    printf("Inputted Host Name: %s\nInputted Port Number: %s\n",hostN,portNum);
}
//Starts listening Code, Starts UI thread
int main(int argc, char *argv[]){
    int port = 0;
    top = 0;
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
    for (x = 0; x < sizeof(connects)/sizeof(struct tag); ++x){
        connects[x].id = 0;
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
                printf("Enter 1-9 corresponding to commands\n");
                break;
            case '2':
                //TODO get rid of all the streams
                //call terminate on everything in list
                printf("Thank you for using this program\n");
                exit(0);
                break;
            case '3':
                //TODO: get IP address
                printf("This server is running on the IP: __\n");
                break;
            case '4':
                printf("This server is listening on %d\n",port);
                break;
            case '5':
                //Connect
                connect();
                break;
            case '6':
                printf("Printing List of Connections:\n");
                for (int x = 0; x < sizeof(connects)/sizeof(struct tag); ++x) {
                    //print list
                    if (connects[x].id != 0){
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

