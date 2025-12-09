#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void make_strings(char* s_p1R, char* s_p2R, char* s_p1W, char* s_p2W, char* s_sem1, char* s_sem2, char* s_sm1, char* s_sm2,
                  char* s_port1, char* s_port2, int pr1, int pr2, int pw1, int pw2,
                  int sem1, int sem2, int shm1, int shm2, int port1, int port2);
void clean();
void kill_all(int in);
void message(int in);

int Serv1,Serv2,D,P1,P2,Pr,T,S;
int sem1, sem2, shm1, shm2;

int main(int argc,char* argv[]){
	if(argc != 3)perror("Zle spustene");
	else printf("Zadanie spustene\n");

    time_t start_time = time(NULL);
    alarm(115);

    (void)signal(SIGUSR2,kill_all);
    (void)signal(SIGUSR1,message);
    (void)signal(SIGALRM, kill_all);

    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    sem1 = semget(IPC_PRIVATE, 2, 0666 | IPC_CREAT);
    sem2 = semget(IPC_PRIVATE, 2, 0666 | IPC_CREAT);
    if(sem1 ==-1 || sem2 == -1) {
        perror("semget()");
        exit(EXIT_FAILURE);
    }

    semctl(sem1,0,SETVAL,0);
    semctl(sem2,0,SETVAL,0);
    semctl(sem1,1,SETVAL,0);
    semctl(sem2,1,SETVAL,0);

    int port1 = atoi(argv[1]), port2 = atoi(argv[2]);

    shm1 = shmget(IPC_PRIVATE,256,0666|IPC_CREAT);
    shm2 = shmget(IPC_PRIVATE,256,0666|IPC_CREAT);

    int pr1 = dup(pipe1[0]);
    int pr2 = dup(pipe2[0]);
    int pw1 = dup(pipe1[1]);
    int pw2 = dup(pipe2[1]);

    char s_p1R[10], s_p2R[10], s_p1W[10], s_p2W[10];
    char s_sem1[10], s_sem2[10], s_sm1[10], s_sm2[10];
    char s_port1[10], s_port2[10], s_p1[10], s_p2[10];

    make_strings(s_p1R, s_p2R, s_p1W, s_p2W, s_sem1, s_sem2, s_sm1, s_sm2, s_port1, s_port2, pr1, pr2, pw1, pw2, sem1, sem2, shm1, shm2, port1, port2);

    Serv1 = fork();
    if(Serv1 == 0){
        if((execl("proc_serv1","proc_serv1",s_port1,s_port2,(char*)NULL)) < 0) perror("Server 1 nespusteny");
        exit(EXIT_SUCCESS);
    }
    if(Serv1 <= -1) perror("Chyba Server 1!");
    usleep(500000);

    Serv2 = fork();
    if(Serv2 == 0){
        if((execl("proc_serv2","proc_serv2",s_port2,(char*)NULL)) < 0) perror("Server 2 nespusteny");
        exit(EXIT_SUCCESS);
    }
    if(Serv2 <= -1) perror("Chyba pServ2!");
    usleep(500000);

    D = fork();
    if(D == 0){
        if((execl("proc_d","proc_d",s_sm2,s_sem2,s_port1,(char*)NULL)) < 0) perror("Proces D nespusteny");
        exit(EXIT_SUCCESS);
    }
    if(D <= -1)  perror("Chyba Server 1");
    usleep(500000);

    P1 = fork();
    if(P1 < 0){
        perror("P1 sa nepodarilo vytvorit\n");
        exit(EXIT_FAILURE);
    } else if(P1 == 0){
        if((execl("proc_p1","proc_p1",s_p1W,(char*)NULL)) < 0){
            perror("Proces vytvorenie p1 zlyhal");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    usleep(500000);

    P2 = fork();
    if(P2 < 0){
        perror("P2 sa nepodarilo vytvorit\n");
        exit(EXIT_FAILURE);
    }
    if(P2 == 0){
        if((execl("proc_p2","proc_p2",s_p1W,(char*)NULL)) < 0){
            perror("proces vytvorenie p2 zlyhal...\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    usleep(500000);

    memset(s_p1,'\0', 10);
    memset(s_p2,'\0', 10);
    sprintf(s_p1,"%d", P1);
    sprintf(s_p2,"%d", P2);

    Pr = fork();
    if(Pr < 0){
        perror("Pr sa nepodarilo vytvorit\n");
        exit(EXIT_FAILURE);
    }
    if(Pr == 0){
        if((execl("proc_pr", "proc_pr",s_p1,s_p2,s_p1R,s_p2W,(char*)NULL)) < 0){
            perror("proces otvorenie pr zlyhal ");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    usleep(500000);

    T = fork();
    if(T < 0){
        perror("Proces T sa nepodarilo vytvorit\n");
        exit(EXIT_FAILURE);
    }
    if(T == 0){
        if((execl("proc_t", "proc_t",s_p2R,s_sm1,s_sem1,(char*)NULL)) < 0){
            perror("proces T zlyhal ");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    usleep(500000);

    S = fork();
    if(S < 0){
        perror("Proces S sa nepodarilo vytvorit\n");
        exit(EXIT_FAILURE);
    }
    if(S == 0){
        if((execl("proc_s", "proc_s",s_sm1,s_sem1,s_sm2,s_sem2, (char*)NULL)) < 0){
            perror("proces S zlyhal ");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    while(1) {
        if (time(NULL) - start_time >= 115) {
            printf("Casovy limit 115 sekund dosiahnuty, ukoncujem...\n");
            kill_all(0);
        }
        pause();
    }
    return 0;
}

void clean() {    
    semctl(sem1, 0, IPC_RMID, 0);
    semctl(sem2, 0, IPC_RMID, 0);
    shmctl(shm1, IPC_RMID, NULL);
    shmctl(shm2, IPC_RMID, NULL);
}

void kill_all(int in) {    
    pid_t procs[] = {Serv1, Serv2, D, P1, P2, Pr, T, S};
    
    for (int i = 0; i < 8; i++) {
        if (procs[i] > 0) kill(procs[i], SIGKILL);
    }
    
    for (int i = 0; i < 8; i++) {
        if (procs[i] > 0) waitpid(procs[i], NULL, 0);
    }

    if (sem1 >= 0) semctl(sem1, 0, IPC_RMID, 0);
    if (sem2 >= 0) semctl(sem2, 0, IPC_RMID, 0);
    
    if (shm1 >= 0) shmctl(shm1, IPC_RMID, NULL);
    if (shm2 >= 0) shmctl(shm2, IPC_RMID, NULL);
    
    exit(0);
}

void make_strings(char* s_p1R, char* s_p2R, char* s_p1W, char* s_p2W, char* s_sem1, char* s_sem2, char* s_sm1, char* s_sm2,
                  char* s_port1, char* s_port2, int pr1, int pr2, int pw1, int pw2,
                  int sem1, int sem2, int shm1, int shm2, int port1, int port2) {
    
    sprintf(s_p1R, "%d", pr1);
    sprintf(s_p2R, "%d", pr2);
    sprintf(s_p1W, "%d", pw1);
    sprintf(s_p2W, "%d", pw2);
    sprintf(s_sem1, "%d", sem1);
    sprintf(s_sem2, "%d", sem2);
    sprintf(s_sm1, "%d", shm1);
    sprintf(s_sm2, "%d", shm2);
    sprintf(s_port1, "%d", port1);
    sprintf(s_port2, "%d", port2);
}

void message(int n){}