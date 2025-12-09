#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 150

int fd, P2;
volatile sig_atomic_t exit_flag = 0;
static int curr_pos = 0;

void sigusr2_handler(int sig) {exit_flag = 1;}

void handler(int signum) {
    char input[MAX_LINE_LENGTH + 1], buffer;
    int length = 0;
    
    lseek(fd, curr_pos, SEEK_SET);
    
    while(1) {
        if(read(fd, &buffer, 1) <= 0) {
            lseek(fd, 0, SEEK_SET);
            curr_pos = 0;
            read(fd, &buffer, 1);
        }
        
        if(buffer != '\n') {
            input[length] = buffer;
            length++;
        } else {
            input[length] = '\0';
            curr_pos = lseek(fd, 0, SEEK_CUR);
            break;
        }
        if(length >= MAX_LINE_LENGTH) break;
    }
    
    if(length > 0) {
        write(P2, input, strlen(input));
        write(P2, "\n", 1);
    }
}

int main(int argc, char* argv[]) {
    signal(SIGUSR1, handler);
    signal(SIGUSR2, sigusr2_handler);
    
    fd = open("p2.txt", O_RDONLY);
    P2 = atoi(argv[1]);
    
    printf("P2 is open\n");
    kill(getppid(), SIGUSR1);
    
    while(!exit_flag) {pause();}
    
    close(fd);
    printf("P2 is closed\n");
    return 0;
}