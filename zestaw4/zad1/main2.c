#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int stp = 0;
int waiting = 0;
int make_fork = 0;
pid_t pid = 0;

void stp_handler();
void int_handler();
void wait_for();

int main (int argc, char **argv) 
{

    struct sigaction act;
    act.sa_handler = stp_handler;
    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, int_handler);
    signal(SIGCHLD, SIG_IGN);

    pid = fork();
    while (1)
    {
        if (!waiting && pid == 0) {
            execl("./shell.sh", "./shell.sh", NULL);
        }
    }

    return 0;
}

void wait_for()
{
    waiting = !waiting;
    
    if (waiting && pid != 0) {
        kill(pid, SIGKILL);
    }
    else if (!waiting && pid != 0) {
        pid = fork();
    }
}

void stp_handler() 
{
    stp = stp == 1 ? 0 : 1;
    wait_for();

    if (stp == 1) {
        printf("Oczekuje na  CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
    }
}

void int_handler() 
{
    if (stp == 0) {
        printf("Odebrano sygnal SIGINT\n");
    }
    exit(EXIT_SUCCESS);
}
