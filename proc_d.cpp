#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

int sem;
volatile sig_atomic_t exit_flag = 0;

void sigusr2_handler(int sig) {exit_flag = 1;}

static int sem_v(){
    struct sembuf sem_oper;
    sem_oper.sem_op = 1;
    sem_oper.sem_num = 0;
    sem_oper.sem_flg = SEM_UNDO;
    if(semop(sem, &sem_oper, 1) == -1) {
        perror("Semafor chyba");
        return(0);
    }
    return(1);
}

static int sem_p(){
    struct sembuf sem_oper;
    sem_oper.sem_op = -1;
    sem_oper.sem_num = 1;
    sem_oper.sem_flg = SEM_UNDO;
    if(semop(sem, &sem_oper, 1) == -1) {
        perror("Semafor chyba");
        return(0);
    }
    return(1);
}

int main(int argc,char *argv[]){
    signal(SIGUSR2, sigusr2_handler);
    
    kill(getppid(),SIGUSR1);
    struct sockaddr_in serv_info;
    sem = atoi(argv[2]);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0) {
        perror("Proces D chyba");
        exit(EXIT_FAILURE);
    }

    char string_buf[256] = {'\0'};

    bzero((char*)&serv_info,sizeof(serv_info));
    serv_info.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&serv_info.sin_addr.s_addr);
    serv_info.sin_port = htons(atoi(argv[3]));

    int connected = 0;
    int attempts = 0;
    
    for (attempts = 0; attempts < 20 && !connected; attempts++) {
        if (connect(socket_fd,(struct sockaddr*)&serv_info,sizeof(serv_info)) == 0) {
            connected = 1;
            printf("Proces D: Uspesne pripojeny k serveru na porte %s (pokus %d)\n", argv[3], attempts + 1);
        } else {
            if (attempts % 5 == 0) {
                printf("Proces D: Cakam na server... pokus %d/20\n", attempts + 1);
            }
            sleep(1);
        }
    }

    char*shm_memory = (char *)shmat(atoi(argv[1]), NULL, 0);

    while (!exit_flag){
        if(!sem_v()) exit(EXIT_FAILURE);
        strcpy(string_buf, shm_memory);
        if (string_buf[0] != '\0') write(socket_fd, string_buf, (int)strlen(string_buf)+1);
        if(!sem_p()) exit(EXIT_FAILURE);
        sleep(1);
    }
    
    printf("Proces D is closed\n");
    close(socket_fd);
    shmdt(shm_memory);
    return 0;
}