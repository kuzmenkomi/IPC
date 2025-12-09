#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

int sem;
volatile sig_atomic_t exit_flag = 0;

void sigusr2_handler(int sig) {exit_flag = 1;}

static int sem_v(){
    struct sembuf sem_oper;
    sem_oper.sem_op = 1;
    sem_oper.sem_num = 1;
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
    sem_oper.sem_num = 0;
    sem_oper.sem_flg = SEM_UNDO;
    if(semop(sem, &sem_oper, 1) == -1) {
        perror("Semafor chyba");
        return(0);
    }
    return(1);
}

int main(int argc, char const *argv[]){
    signal(SIGUSR2, sigusr2_handler);
    
    char string_buf[256] = {'\0'};
    char temp;
    int count = 0;
    
    sem = atoi(argv[3]);
    char*shm_memory = (char *)shmat(atoi(argv[2]),NULL,0);
    kill(getppid(),SIGUSR1);

    int input_pipe = atoi(argv[1]);
    
    while(!exit_flag){
        count = 0;
        while(read(input_pipe,&temp,1) > 0 && count < 255){
            if(temp == '\n') break;
            else{
                string_buf[count] = temp;
                string_buf[count+1] = '\0';
                count++;
            }
        }
        
        if (count > 0) {
            if(!sem_v()) exit(EXIT_FAILURE);
            strcpy(shm_memory,string_buf);
            if(!sem_p()) exit(EXIT_FAILURE);
        }
        sleep(1);
    }
    
    printf("Proces T is closed\n");
    shmdt(shm_memory);
    return 0;
}