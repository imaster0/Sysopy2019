#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

/*
1) plik lista
*/
const int MAX_PATH_SIZE = 1000;
int paused = 0, copies = -1;

struct node {
    int value;
    char *filename;
    struct node *next;
} *list;

void monitor(char *path, int freq);
void show_list(struct node * list); // LIST
void stop_all(struct node * list); // STOP ALL
void start_all(struct node * list); // START ALL
void raport_all(struct node * list); // END
void raport();
int is_in(struct node * list, int pid);
void pause_proc();
void start_proc();

struct node * create_node() {
    struct node * new_node = malloc(sizeof(struct node));
    new_node->next = NULL;
    new_node->value = 0;
    return new_node;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "Missing arguments");
        exit(-1);
    }

    FILE *file;

    if (!(file = fopen(argv[1], "r")))
    {
        fprintf(stderr, "Cannot open file");
        exit(-1);
    }

    char path[MAX_PATH_SIZE];
    int freq;
    int processes = 0;
    list = create_node();
    struct node *pointer = list;

    while (fscanf(file, "%s %d", path, &freq) != EOF)
    {
        pid_t child = fork();
        
        if (child == 0) monitor(path, freq);
        else {
            processes++;
            pointer->next = create_node();
            pointer = pointer->next;
            pointer->value = child;
            pointer->filename = malloc(sizeof(path));
            strcpy(pointer->filename, path);
        }
    }
    fclose(file);
    char command[9];

    show_list(list);

    while (1) {
        scanf("%s", command);

        if (strcmp(command, "LIST") == 0) show_list(list);
        else if (strcmp(command, "STOP") == 0) {
            scanf("%s", command);
            if (strcmp(command, "ALL") == 0) stop_all(list);
            else if (is_in(list, atoi(command))) kill(atoi(command), SIGUSR1);
            else fprintf(stderr, "Invalid option / pid\n");
        }
        else if (strcmp(command, "START") == 0) {
            scanf("%s", command);
            if (strcmp(command, "ALL") == 0) start_all(list);
            else if (is_in(list, atoi(command))) kill(atoi(command), SIGUSR2);
            else fprintf(stderr, "Invalid option / pid\n");
        }
        else if (strcmp(command, "END") == 0) {
            raport_all(list);
            exit(0);
        }
        else if (strcmp(command, "HELP") == 0) {
            printf("Available commands: LIST, STOP <PID>, STOP <ALL>, START <PID>, START <ALL>, END\n");
        }
        else {
            fprintf(stderr, "Unknown command, try HELP\n");
        }
    }

    return 0;
}

void copy_file(char *path, time_t current_time, char *remember)
{

    struct tm *time_info;
    char time_string[21]; // space for "_RRRR-MM-DD_GG-MM-SS\0"
    char path_cp[100];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "_%F_%H-%M-%S", time_info);
    sprintf(path_cp, "archiwum/%s%s", path, time_string);


    FILE *remembered;
    if (!(remembered = fopen(path_cp, "w+")))
    {
        fprintf(stderr, "Cannot create file [%s]\n", path_cp);
        exit(0);
    }

    fwrite(remember, sizeof(char), strlen(remember), remembered);
    fclose(remembered);
}

void monitor(char *path, int freq)
{

    signal(SIGUSR1, pause_proc);
    signal(SIGUSR2, start_proc);
    signal(SIGINT, raport);

    if (freq <= 0)
    {
        fprintf(stderr, "fequence should be greater than 0");
        exit(0);
    }

    struct stat stats;
    int i;
    time_t last_time;
    copies = 0;

    if (stat(path, &stats) == -1)
    {
        fprintf(stderr, "stat error");
        exit(0);
    }
    last_time = stats.st_mtime;

    FILE *file = NULL;
    char *remember = NULL;

    if (!(file = fopen(path, "r")))
    {
        fprintf(stderr, "Cannot open a file [%s]\n", path);
        exit(0);
    }

    fseek(file, 0, SEEK_END);
    int curr_pos = ftell(file);
    rewind(file);

    remember = malloc(curr_pos);
    fread(remember, sizeof(char), curr_pos, file);

    i = 0;
    

    while(1)
    {
        if (paused) continue;
        i = (i + 1) % freq;
        sleep(1);
        if (i != 0)
            continue;

        if (stat(path, &stats) == -1)
        {
            fprintf(stderr, "stat error");
            exit(0);
        }


        if (difftime(last_time, stats.st_mtime) != 0)
        {
            copy_file(path, stats.st_mtime, remember);
            
            fseek(file, 0, SEEK_END);
            int curr_pos = ftell(file);
            rewind(file);

            free(remember);
            remember = malloc(curr_pos);
            fread(remember, sizeof(char), curr_pos, file);

            last_time = stats.st_mtime;
            copies++;
            fprintf(stderr, "Copied!\n");
        }

        last_time = stats.st_mtime;
    }

    if (file != NULL)
        fclose(file);
    free(remember);
    exit(copies);
}


void show_list(struct node * list) {
    
    struct node *pointer = list->next;
    while (pointer != NULL)
    {
        printf("PID: %d %s\n", pointer->value, pointer->filename);
        pointer = pointer->next;
    }
}

void stop_all(struct node * list) {

    struct node *pointer = list->next;
    while (pointer != NULL) 
    {
        kill(pointer->value, SIGUSR1);
        pointer = pointer->next;
    }
}

void start_all(struct node * list) {

    struct node * pointer = list->next;
    while (pointer != NULL)
    {
        kill(pointer->value, SIGUSR2);
        pointer = pointer->next;
    }
}

void raport() {

    if (copies != -1) printf("PID: %d, copies: %d\n", getpid(), copies);
    exit(0);
}

void raport_all(struct node * list) {

    struct node * pointer = list->next;
    while (pointer != NULL)
    {
        kill(pointer->value, SIGINT);
        pointer = pointer->next;
    }
}

void pause_proc() {
    fprintf(stderr, "Process %d stopped\n", getpid());
    paused = 1;
}

void start_proc() {
    fprintf(stderr, "Process %d started\n", getpid());
    paused = 0;
}

int is_in(struct node * list, int pid) {

    struct node * pointer = list->next;
    while (pointer != NULL)
    {
        if (pointer->value == pid) return 1;
        pointer = pointer->next;
    }
    return 0;
}