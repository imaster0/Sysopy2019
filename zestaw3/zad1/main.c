#define _GNU_SOURCE 1
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/wait.h>

const int MAX_PATH_SIZE = 1000;


void find_nftw(const char *path);
void find_stat(char *path);


int main(int argc, char **argv) {

    assert(argc > 2);
    char *type = argv[1];
    char *path = argv[2];

    if (strcmp(type, "nftw") == 0) find_nftw(path);
    else if (strcmp(type, "stat") == 0) find_stat(path);
    else {
        fprintf(stderr, "Invalid command [%s]\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}


void print_dir(const char *current_path) {
    // 1.
    printf("Current path: %s\n", current_path);
    printf("PID: %d\n", getpid());
    char buff[MAX_PATH_SIZE];
    sprintf(buff, "ls -l %s", current_path);
    system(buff);
}


int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {

    if (typeflag != FTW_D) return 0;
    //FILE INFO
    pid_t pid = fork();
    int status;
    if (pid) {
        wait(&status);
        return 1;
    }
    print_dir(fpath);

    return 0;
}


void find_nftw(const char *path) {

    nftw(path, fn, 1, FTW_PHYS);
}


void find_stat(char *path) {

    DIR *directory = opendir(path);
    struct dirent *file_info;
    struct stat *file_stat = malloc(sizeof(struct stat));
    char full_path[MAX_PATH_SIZE];
    char current_path[MAX_PATH_SIZE];

    print_dir(path);

    while ((file_info = readdir(directory)) != NULL) {
        
        sprintf(current_path, "%s/%s", path, file_info->d_name);    
        realpath(current_path, full_path);
        lstat(current_path, file_stat);

        if (S_ISDIR(file_stat->st_mode) &&
            strcmp(file_info->d_name, ".") != 0 &&
            strcmp(file_info->d_name, "..") != 0) {

            pid_t child = fork();

            if (child == 0) {
                find_stat(full_path);
                return;
            }
        }
    }

    free(file_stat);
    file_stat = NULL;
    closedir(directory);
}