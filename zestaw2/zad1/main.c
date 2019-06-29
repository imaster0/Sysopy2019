#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#define random(a, b) (rand() % (b - a + 1) + a)

void generate(char *filename, int rows, int size);
void sort_tmp_lib(char *filename, int rows, int size);
void sort_tmp_sys(char *filename, int rows, int size);
void copy_file_lib(char *filename1, char *filename2, int rows, int size);
void copy_file_sys(char *filename1, char *filename2, int rows, int size);


void init_time(struct rusage *rusage_start, struct timeval *realtime_start) {
    getrusage(RUSAGE_SELF, rusage_start);
    struct timezone timezone = {0, 0};
    gettimeofday(realtime_start, &timezone);
}


void print_time(struct rusage *rusage_start, struct timeval *realtime_start, FILE *file) {

    struct rusage *rusage_end = malloc(sizeof(struct rusage));
    struct timeval *realtime_end = malloc(sizeof(struct timeval));

    getrusage(RUSAGE_SELF, rusage_end);
    struct timezone timezone = {0, 0};
    gettimeofday(realtime_end, &timezone);

    timersub(&rusage_end->ru_stime, &rusage_start->ru_stime, &rusage_end->ru_stime);
    timersub(&rusage_end->ru_utime, &rusage_start->ru_utime, &rusage_end->ru_utime);
    timersub(realtime_end, realtime_start, realtime_end);

    printf("\tReal time:   %ld.%06ld \n", realtime_end->tv_sec, realtime_end->tv_usec);
    printf("\tUser time:   %ld.%06ld \n", rusage_end->ru_utime.tv_sec, rusage_end->ru_utime.tv_usec);
    printf("\tSystem time: %ld.%06ld \n", rusage_end->ru_stime.tv_sec, rusage_end->ru_stime.tv_usec);

    fprintf(file, "\tReal time:   %ld.%06ld \n", realtime_end->tv_sec, realtime_end->tv_usec);
    fprintf(file, "\tUser time:   %ld.%06ld \n", rusage_end->ru_utime.tv_sec, rusage_end->ru_utime.tv_usec);
    fprintf(file, "\tSystem time: %ld.%06ld \n", rusage_end->ru_stime.tv_sec, rusage_end->ru_stime.tv_usec);

    free(rusage_end);
    free(realtime_end);
}


int main(int argc, char **argv) {
    
    srand(time(NULL));
    assert(argc > 1);

    FILE *logs = fopen("logs", "a");
    struct rusage *rusage_start = malloc(sizeof(struct rusage));
    struct timeval *timeval_start = malloc(sizeof(struct timeval));

    if (strcmp(argv[1], "generate") == 0) {
        
        assert(argc > 4);
        char *filename = argv[2];
        int rows = atoi(argv[3]);
        int size = atoi(argv[4]);
        generate(filename, rows, size);
    } else if (strcmp(argv[1], "sort") == 0) {

        assert(argc > 5);
        char *filename = argv[2];
        int rows = atoi(argv[3]);
        int size = atoi(argv[4]);
        char *type = argv[5];
        
        init_time(rusage_start, timeval_start);
        if (strcmp(type, "sys") == 0) {
            fwrite("sort_tmp_sys\n", sizeof(char), 13, logs);
            sort_tmp_sys(filename, rows, size);
        } else {
            fwrite("sort_tmp_lib\n", sizeof(char), 13, logs);
            sort_tmp_lib(filename, rows, size);
        } 
        print_time(rusage_start, timeval_start, logs);
    } else if (strcmp(argv[1], "copy") == 0) {

        assert(argc > 6);
        char *filename1 = argv[2];
        char *filename2 = argv[3];
        int rows = atoi(argv[4]);
        int size = atoi(argv[5]);
        char *type = argv[6];

        init_time(rusage_start, timeval_start);
        if (strcmp(type, "sys") == 0) {
            fwrite("copy_file_sys\n", sizeof(char), 14, logs);
            copy_file_sys(filename1, filename2, rows, size);
        } else {
            fwrite("copy_file_lib\n", sizeof(char), 14, logs);
            copy_file_lib(filename1, filename2, rows, size);
        }
        print_time(rusage_start, timeval_start, logs);

    } else {
        fprintf(stderr, "Invalid command [%s]\n", argv[1]);
    }

    free(rusage_start);
    free(timeval_start);
    fclose(logs);
    return 0;
}


short is_good(char *filename, int rows, int size) {
    
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 0;
    }

    char c;
    int count_rows = 0;
    int count_size = 0;

    for (c = getc(file); c != EOF; c = getc(file)) {
        if (c == '\n')
            count_rows = count_rows + 1;
        count_size++;
    }
    count_size /= count_rows;

    fclose(file);

    if (rows != count_rows || size != count_size)
        return 0;
    return 1;
}

// generate - tworzenie pliku z rekordami wypełnionego wygenerowaną losową 
// zawartością(można wykorzystać wirtualny generator / dev / random 
// lub w wersji uproszczonej funkcję rand) 
void generate (char *filename, int rows, int size) {

    FILE *file = fopen(filename, "w");

    int i;
    for (i = 0; i < rows; i++) {
        
        int j;
        for(j = 0; j < size; j++) {
        
            fprintf(file, "%c", random(65, 90));
        }
        
        fprintf(file, "\n");
    }

    fclose(file);
}

// sort (2) - sortuje rekordy w pliku używając sortowania przez proste wybieranie.
// Kluczem do sortowania niech będzie wartość pierwszego bajtu 
// rekordu(interpretowanego jako liczba bez znaku - unsigned char) 
// Podczas sortowania w pamięci powinny być przechowywane jednocześnie 
// najwyżej dwa rekordy(sprowadza się do zamieniania miejscami 
// i porównywania dwóch rekordów).
void sort_tmp_lib(char *filename, int rows, int size) {

    size++; // bo \n to tez znak
    if (!is_good(filename, rows, size)) {
        fprintf(stderr, "Invalid number of rows / size for [%s]\n", filename);
        exit(EXIT_FAILURE);
    }

    char *buff = calloc(size, sizeof(char));
    char *buff2 = calloc(size, sizeof(char));

    FILE *file = fopen(filename, "r+");
    
    int i;
    for (i = 0; i < rows; i++) {
        
        fseek(file, i*size, SEEK_SET);
        fread(buff, sizeof(char), size, file);
        
        int j;
        for (j = i + 1; j < rows; j++) {
            
            fseek(file, j*size, SEEK_SET);
            fread(buff2, sizeof(char), size, file);

            if (strcmp(buff, buff2) > 0) {

                fseek(file, i * size, SEEK_SET);
                fwrite(buff2, sizeof(char), size, file);
                fseek(file, j * size, SEEK_SET);
                fwrite(buff, sizeof(char), size, file);
                strcpy(buff, buff2);
            }
        }
    }
    
    free(buff);
    free(buff2);
    fclose(file);
}


void sort_tmp_sys(char *filename, int rows, int size) {

    size++; // bo \n to tez znak
    if (!is_good(filename, rows, size)) {
        fprintf(stderr, "Invalid number of rows / size for [%s]\n", filename);
        exit(EXIT_FAILURE);
    }
    char *buff = calloc(size, sizeof(char));
    char *buff2 = calloc(size, sizeof(char));

    int file = open(filename, O_RDWR);
    
    int i;
    for (i = 0; i < rows; i++) {

        lseek(file, i * size, SEEK_SET);
        read(file, buff, sizeof(char) * size);

        int j;
        for (j = i + 1; j < rows; j++) {

            lseek(file, j * size, SEEK_SET);
            read(file, buff2, sizeof(char) * size);

            if (strcmp(buff, buff2) > 0) {

                lseek(file, i * size, SEEK_SET);
                write(file, buff2, sizeof(char) * size);
                lseek(file, j * size, SEEK_SET);
                write(file, buff, sizeof(char) * size);
                strcpy(buff, buff2);
            }
        }
    }

    free(buff);
    free(buff2);
    close(file);
}

// copy (2) - kopiuje plik1 do pliku2. Kopiowanie powinno odbywać się za pomocą 
// bufora o zadanej wielkości rekordu.
void copy_file_lib(char *filename1, char *filename2, int rows, int size) {

    if (strcmp(filename1, filename2) == 0) {
        fprintf(stderr, "Illegal operation - coping to itself\n");
        exit(EXIT_FAILURE);
    }

    size++; // \n
    if (!is_good(filename1, rows, size)) {
        fprintf(stderr, "Invalid number of rows / size for [%s]\n", filename1);
        exit(EXIT_FAILURE);
    }
    char *buff = calloc(rows * size, sizeof(char));

    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "w+");

    fread(buff, sizeof(char), rows * size, file1);
    fwrite(buff, sizeof(char), rows * size, file2);

    free(buff);
    fclose(file1);
    fclose(file2);
}


void copy_file_sys(char *filename1, char *filename2, int rows, int size) {

    if (strcmp(filename1, filename2) == 0) {
        fprintf(stderr, "Illegal operation - coping to itself\n");
        exit(EXIT_FAILURE);
    }

    size++; // \n
    if (!is_good(filename1, rows, size)) {
        fprintf(stderr, "Invalid number of rows / size for [%s]\n", filename1);
        exit(EXIT_FAILURE);
    }
    char *buff = calloc(rows * size, sizeof(char));

    int file1 = open(filename1, O_RDONLY);
    int file2 = open(filename2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);

    read(file1, buff, sizeof(char) * rows * size);
    write(file2, buff, sizeof(char) * rows * size);

    free(buff);
    close(file1);
    close(file2);
}

