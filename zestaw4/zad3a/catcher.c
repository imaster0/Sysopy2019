#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

void fun_usr1();
void fun_usr2(int sig, siginfo_t *info, void *ucontext);
void send_kill(pid_t pid, int count);     //KILL
void send_sigqueue(pid_t pid, int count); //SIGQUEUE
void send_sigrt(pid_t pid, int count);    //SIGRT

const int MAX_TYPE_SIZE = 100;
int signals_count = 0;
char *type;

// ./catcher <TYPE>
// TYPE: KILL|SIGQUEUE|SIGRT
int main(int argc, char **argv)
{

    if (argc <= 1) {
        fprintf(stderr, "Not enough arguments\n");
        exit(-1);
    }

    printf("%d\n", getpid());
    type = malloc(sizeof(argv[1]));
    strcpy(type, argv[1]);

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

    while(1);

    return 0;
}

void fun_usr1()
{
    ++signals_count;
}

void fun_usr2(int sig, siginfo_t *info, void *ucontext)
{
    fprintf(stderr, "Signals received: %d\n", signals_count);

    if (strcmp(type, "KILL") == 0) send_kill(info->si_pid, signals_count);
    else if (strcmp(type, "SIGQUEUE") == 0)  send_sigqueue(info->si_pid, signals_count);
    else send_sigrt(info->si_pid, signals_count);

    exit(0);
}

void send_kill(pid_t pid, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        kill(pid, SIGUSR1);
    }
    kill(pid, SIGUSR2);
}

void send_sigqueue(pid_t pid, int count)
{
    int i;
    union sigval val;
    val.sival_int = count;
    for (i = 0; i < count; ++i)
    {
        sigqueue(pid, SIGUSR1, val);
    }
    sigqueue(pid, SIGUSR2, val);
}

void send_sigrt(pid_t pid, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        kill(pid, SIGRTMIN);
    }
    kill(pid, SIGRTMAX);
}
