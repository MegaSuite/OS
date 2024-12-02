#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define BUF_SIZE 64

int fd[2];
pid_t pid1, pid2;
int x = 1;
char buffer[BUF_SIZE];

void sig_handler_parent(int signo);
void sig_handler_child1(int signo);
void sig_handler_child2(int signo);

int main() 
{
    // pipe init
    if (pipe(fd) == -1) 
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // SIGINT handler
    signal(SIGINT, sig_handler_parent);

    // child 1
    pid1 = fork();
    if (pid1 < 0) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid1 == 0) 
    {
        // SIGINT ignorance
        signal(SIGINT, SIG_IGN); 
        signal(SIGUSR1, sig_handler_child1);
        // close read end
        close(fd[0]); 

        while (1) 
        {
            sprintf(buffer, "I send you %d times.", x++);
            write(fd[1], buffer, strlen(buffer) + 1);
            sleep(1);
        }
    }

    // child 2
    pid2 = fork();
    if (pid2 < 0) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid2 == 0) 
    {
        signal(SIGINT, SIG_IGN); 
        signal(SIGUSR2, sig_handler_child2);
        // close write end
        close(fd[1]); 

        while (1) 
        {
            read(fd[0], buffer, BUF_SIZE);
            printf("Received: %s\n", buffer);
        }
    }

    // parent
    wait(NULL);
    wait(NULL);
    close(fd[0]);
    close(fd[1]);
    printf("Parent Process is Killed!\n");
    return 0;
}

void sig_handler_parent(int signo) 
{
    if (signo == SIGINT) 
    {
        kill(pid1, SIGUSR1);
        kill(pid2, SIGUSR2);
    }
}

void sig_handler_child1(int signo) 
{
    if (signo == SIGUSR1) 
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        fprintf(stderr, "[Child 1] Terminating at %ld.%06ld\n", tv.tv_sec, tv.tv_usec);

        close(fd[1]); 
        printf("Child Process 1 is Killed by Parent!\n");
        printf("Times Sent: %d\n", x - 1);
        exit(0);
    }
}

void sig_handler_child2(int signo)
{
    if (signo == SIGUSR2) 
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        fprintf(stderr, "\n[Child 2] Terminating at %ld.%06ld\n", tv.tv_sec, tv.tv_usec);

        close(fd[0]);
        printf("Child Process 2 is Killed by Parent!\n");
        exit(0);
    }
}