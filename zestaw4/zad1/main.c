#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

int stp = 0;
int waiting = 0;

void stp_handler();
void int_handler();
void loop();

int main (int argc, char **argv) 
{

    struct sigaction act;
    act.sa_handler = stp_handler;
    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, int_handler);

    time_t time_info;
    struct tm *ptr = NULL;

    while (1)
    {
        if (!waiting)
        {
            time(&time_info);
            ptr = localtime(&time_info);
            char buffer[80];
            strftime(buffer, 80, "%F %T", ptr);
            printf("%s\n", buffer);
        }
    }

    return 0;
}

void stp_handler() 
{

    if (stp == 1) 
    {
        stp = 0;
        waiting = !waiting;
        return;
    }
    stp = 1;
    printf("Oczekuje na  CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
    waiting = !waiting;
}

void int_handler() 
{

    if (stp == 1) 
    {
        printf("Odebrano sygnal SIGINT\n");
        exit(0);
    }
}
