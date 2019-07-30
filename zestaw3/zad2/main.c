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
2) czas monitorowania
3) tryb
*/
const int MAX_PATH_SIZE = 1000;
void monitor(char *path, int freq, int monitoring_time, int type);

int main(int argc, char **argv) {

    if (argc <= 3) {

        fprintf(stderr, "Missing arguments\n");
        exit(EXIT_FAILURE);
    }

    FILE *file;

    if (!(file = fopen(argv[1], "r"))) {

        fprintf(stderr, "Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    int monitoring_time = atoi(argv[2]);
    int type = atoi(argv[3]);

    if (type != 1 && type != 2) {
        fprintf(stderr, "Unknown type [%d]", type);
        exit(EXIT_FAILURE);
    }

    system("mkdir archiwum 2>/dev/null"); // :)

    char path[MAX_PATH_SIZE];
    int values = 0;
    int processes = 0;
    int return_value = 0;
    int freq, i;
    
    while (fscanf(file, "%s %d", path, &freq) != EOF) {
        
        pid_t child = fork();
        if (child == 0) {
            monitor(path, freq, monitoring_time, type);
        } else {
            processes++;
        }
    }

    for (i = 0; i < processes; ++i) {
        pid_t pid = wait(&return_value);
        if (WIFEXITED(return_value)) {
            values += WEXITSTATUS(return_value);
            printf("Proces PID: %d utworzyl %d kopii pliku\n", pid, WEXITSTATUS(return_value));
        }
        else {
            fprintf(stderr, "Proces PID: %d nie zostal zakonczony prawidlowo\n", pid);
        }
    }

    printf("Total copies %d\n", values);

    fclose(file);

    return 0;
}


void copy_file(char *path, time_t current_time, int type, char *remember) {

    struct tm *time_info;
    char time_string[21]; // space for "_RRRR-MM-DD_GG-MM-SS\0"
    char path_cp[100];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "_%F_%H-%M-%S", time_info);
    sprintf(path_cp, "archiwum/%s%s", path, time_string);
    
    if (type == 1) {
       
        pid_t child_pid;
        child_pid = fork();

        if (child_pid == 0) {

            char *const av[] = {"cp", path, path_cp, NULL};
            execvp("cp", av);
        }
    } else if (type == 2) {
        
        FILE *remembered;
        if (!(remembered = fopen(path_cp, "w+"))) {
            fprintf(stderr, "Cannot create file [%s]\n", path_cp);
            exit(0);
        }

        fwrite(remember, sizeof(char), strlen(remember), remembered);
        fclose(remembered);
    } 
}


void monitor(char *path, int freq, int monitoring_time, int type) {

    if (freq <= 0) {
        fprintf(stderr, "frequence should be greater than 0\n");
        exit(EXIT_FAILURE);
    }

    struct stat stats;
    int i, copies = 0;
    time_t last_time;
    
    if (stat(path, &stats) == -1) {
        fprintf(stderr, "stat error\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = NULL;
    char *remember = NULL;
    if (type == 2) {
        if (!(file = fopen(path, "r"))) {
            fprintf(stderr, "Cannot open a file [%s]\n", path);
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        int curr_pos = ftell(file);
        rewind(file);

        remember = malloc(curr_pos);
        fread(remember, sizeof(char), curr_pos, file);
    }

    if (type == 1) {

        copy_file(path, stats.st_mtime, type, remember);
        copies++;
    }

    for (i = 0; i < monitoring_time; i++) {

        if (i % freq != 0) continue;

        if (stat(path, &stats) == -1) {
            fprintf(stderr, "stat error");
            exit(EXIT_FAILURE);
        }
        
        if (i == 0) {
        
            last_time = stats.st_mtime;
        } else {

            if (difftime(last_time, stats.st_mtime) != 0) {
                
                copy_file(path, stats.st_mtime, type, remember);
                if (type == 2) {
                    fseek(file, 0, SEEK_END);
                    int curr_pos = ftell(file);
                    rewind(file);

                    free(remember);
                    remember = malloc(curr_pos);
                    fread(remember, sizeof(char), curr_pos, file);
                }

                last_time = stats.st_mtime;
                copies++;
                fprintf(stderr, "Copied!\n");
            }

            last_time = stats.st_mtime;
        }
        sleep(1);
    }

    if (file != NULL) fclose(file);
    free(remember);
    exit(copies);
}

