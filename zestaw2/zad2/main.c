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

const int MAX_PATH_SIZE = 1000;
char *OPT, *DATE;

void find_nftw(char *path);
void find_stat(char *path, char *opt, char *date);
void date_to_str(time_t str, char buff[20]);


int main(int argc, char **argv) {

    assert(argc > 1);

    if (strcmp(argv[1], "nftw") == 0) {

        assert(argc > 4);
        char *path = argv[2];
        OPT = argv[3];
        DATE = argv[4];
        find_nftw(path);
    } else if (strcmp(argv[1], "stat") == 0) {
        
        assert(argc > 4);
        char *path = argv[2];
        char *opt = argv[3];
        char *date = argv[4];
        find_stat(path, opt, date);
    } else {

        fprintf(stderr, "Invalid command [%s]\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}


void date_to_str(time_t str, char buff[20]) {

    strftime(buff, 20, "%F", localtime(&str));
}


short comp(int result, char *opt) {

    if (strcmp(opt, "<") == 0) {
        return (result < 0 ? 1 : 0);
    } else if (strcmp(opt, ">") == 0) {
        return (result > 0 ? 1 : 0);
    } else if (strcmp(opt, "=") == 0) {
        return (result == 0 ? 1 : 0);
    } else {
        printf("Invalid option [%s]", opt);
        exit(EXIT_FAILURE);
    }

    return 0;
}


void print_file_info(const char *full_path, const struct stat *file_stat) {

    // 1.
    printf("Full path: %s\n", full_path);
    // 2.
    if (S_ISREG(file_stat->st_mode)) {
        printf("File type: regular file\n");
    }
    else if (S_ISDIR(file_stat->st_mode)) {
        printf("File type: directory\n");
    }
    else if (S_ISBLK(file_stat->st_mode)) {
        printf("File type: block dev\n");
    }
    else if (S_ISCHR(file_stat->st_mode)) {
        printf("File type: character dev\n");
    }
    else if (S_ISFIFO(file_stat->st_mode)) {
        printf("File type: fifo\n");
    }
    else if (S_ISLNK(file_stat->st_mode)) {
        printf("File type: slink\n");
    }
    else if (S_ISSOCK(file_stat->st_mode)) {
        printf("File type: sock\n");
    }
    // 3.
    printf("File size [B]: %ld\n", file_stat->st_size);
    // 4.
    char buff[20];
    date_to_str(file_stat->st_atime, buff);
    printf("Time of last access: %s\n", buff);
    // 5.
    date_to_str(file_stat->st_mtime, buff);
    printf("Time of last modification: %s\n", buff);
}


int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {

    char full_path[MAX_PATH_SIZE];
    realpath(fpath, full_path);

    char buff[20];
    date_to_str(sb->st_mtime, buff);
    if (comp(strcmp(buff, DATE), OPT)) {

        //FILE INFO
        print_file_info(full_path, sb);
    }

    return 0;
}


void find_nftw(char *path) {
    nftw(path, fn, 10, FTW_PHYS);
}


void find_stat(char *path, char *opt, char *date) {

    DIR *directory = opendir(path);
    struct dirent *file_info;
    struct stat *file_stat = malloc(sizeof(struct stat));
    char full_path[MAX_PATH_SIZE];
    char current_path[MAX_PATH_SIZE];

    while ((file_info = readdir(directory)) != NULL) {
        
        sprintf(current_path, "%s/%s", path, file_info->d_name);        
        realpath(current_path, full_path);
        lstat(current_path, file_stat);

        char buff[20];
        date_to_str(file_stat->st_mtime, buff);
        if (comp(strcmp(buff, date), opt) &&
            strcmp(file_info->d_name, ".") != 0 &&
            strcmp(file_info->d_name, "..") != 0) {

            //FILE INFO
            print_file_info(full_path, file_stat);
        }
        if (S_ISDIR(file_stat->st_mode) &&
            strcmp(file_info->d_name, ".") != 0 &&
            strcmp(file_info->d_name, "..") != 0) {
                find_stat(full_path, opt, date);
        }
    }

    free(file_stat);
    file_stat = NULL;
    closedir(directory);
}