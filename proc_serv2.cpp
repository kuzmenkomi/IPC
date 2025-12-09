#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

int fileID;
int exit_flag = 0;
int message_count = 0;

void sigusr2_handler(int sig) {exit_flag = 1;}

int main(int argc,char* argv[]){
    printf("Server 2 spusteny[%d]:Port=%s.\n",getpid(),argv[1]);

    signal(SIGUSR2, sigusr2_handler);
    
    int socket_fd;
    struct sockaddr_in servAddr,client_Addr;
    char buffer[256] = {'\0'};

    socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(socket_fd < 0) {
        perror("Server 2 chyba");
        exit(EXIT_FAILURE);
    }

    int flag = 1;
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&flag,sizeof(int));

    bzero((char*)&servAddr,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons((unsigned short)atoi(argv[1]));

    printf("Server 2 socket pripaja adresu a port\n");
    if(bind(socket_fd,(struct sockaddr*)&servAddr,sizeof(servAddr)) < 0) {
        perror("Server 2 chyba");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    fileID = open("serv2.txt",O_CREAT|O_WRONLY|O_TRUNC,0666);
    if(fileID < 0) {
        perror("Server 2 otvorenie serv2.txt zlyhalo!");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    socklen_t client_AddrLen = sizeof(struct sockaddr_in);
    kill(getppid(),SIGUSR1);
    
    while(!exit_flag  && message_count < 10) {

        struct timeval t;
        t.tv_sec = 1;
        t.tv_usec = 0;
        setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
        
        ssize_t received = recvfrom(socket_fd,buffer,256,0,(struct sockaddr*)&client_Addr,&client_AddrLen);
        
        if (received > 0) {
            buffer[received] = '\0';
            
            if(write(fileID,buffer,strlen(buffer)) < 0) perror("Server 2 chyba");
            if(write(fileID,"\n",1) < 0) perror("Server 2 chyba");
            printf("Server 2 write:%s. [%d/10]\n", buffer, message_count + 1);
            message_count++;
        }
    }
    
    close(fileID);
    close(socket_fd);
    kill(getppid(), SIGUSR2);
    return 0;
}