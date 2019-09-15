#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

void send_kill(pid_t pid, int count); //KILL
void send_sigqueue(pid_t pid, int count); //SIGQUEUE
void send_sigrt(pid_t pid, int count); //SIGRT

void fun_usr1();
void fun_usr2();

int signals_received, signals_sent;
int returned = 1;
/*
./sender <PID> <ilość sygnałów> <TYPE>
 1) PID procesu catcher, 
 2) ilość sygnałów do wysłania,
 3) TYPE: KILL|SIGQUEUE|SIGRT
*/
int main(int argc, char **argv)
{
    if (argc <= 3) {
        fprintf(stderr, "Not enough arguments\n");
        exit(-1);
    }

    sigset_t signalset;
    sigfillset(&signalset);
    sigdelset(&signalset, SIGUSR1);
    sigdelset(&signalset, SIGUSR2);
    sigdelset(&signalset, SIGRTMIN);
    sigdelset(&signalset, SIGRTMAX);

    sigprocmask(SIG_BLOCK, &signalset, NULL);

    struct sigaction act, act2;
    act.sa_handler = fun_usr1;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR2);
    sigaddset(&act.sa_mask, SIGRTMAX);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGRTMIN, &act, NULL);
    act2.sa_sigaction = fun_usr2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act2, NULL);
    sigaction(SIGRTMAX, &act2, NULL);

    pid_t pid = atoi(argv[1]);
    signals_sent = atoi(argv[2]);
    char *type;
    type = malloc(sizeof(argv[3]));
    strcpy(type, argv[3]);

    if (strcmp(type, "KILL") == 0) send_kill(pid, signals_sent);
    else if (strcmp(type, "SIGQUEUE") == 0) send_sigqueue(pid, signals_sent);
    else if (strcmp(type, "SIGRT") == 0) send_sigrt(pid, signals_sent);
    else fprintf(stderr, "Unknown type [%s]\n", type);

    while(1);

    return 0;
}

void send_kill(pid_t pid, int count)
{
    int i;
    returned = 1;
    for (i = 0; i < count;) {
        if (!returned) continue;
        kill(pid, SIGUSR1);
        returned = 0;
        i++;
    }
    kill(pid, SIGUSR2);
}

void send_sigqueue(pid_t pid, int count)
{
    int i;
    union sigval val;
    val.sival_int = count;
    returned = 1;
    for (i = 0; i < count;) {
        if (!returned) continue;
        sigqueue(pid, SIGUSR1, val);
        returned = 0;
        i++;
    }
    sigqueue(pid, SIGUSR2, val);
}

void send_sigrt(pid_t pid, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        kill(pid, SIGRTMIN);
    }
    kill(pid, SIGRTMAX);
}

void fun_usr1()
{
    ++signals_received;
    returned = 1;
}

void fun_usr2()
{
    fprintf(stderr, "Signals received: %d, Signals sent: %d\n", signals_received, signals_sent);
    exit(0);
}