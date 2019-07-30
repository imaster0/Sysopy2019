#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define random(a, b) (rand() % (b - a + 1) + a)
/*
plik, pmin pmax i bytes, który do zadanego jako pierwszy argument pliku 
z losową częstością od pmin do pmax wyrażoną w sekundach dopisuje 
na jego koniec linię tekstu zawierającą pid procesu, 
wylosowaną liczbę sekund, aktualną datę i czas  
(nie liczą się do liczby bajtów) oraz dowolnego ciągu znaków 
o długości określonej w bytes.
*/

int main(int argc, char **argv) {

    srand(time(NULL));
    if (argc <= 4) {
        fprintf(stderr, "Not enough arguments [%d]\n", argc-1);
        exit(-1);
    }

    char *filename = argv[1];
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);
    int freq = random(pmin, pmax);

    FILE *file;


    while (1) {

        if (!(file = fopen(filename, "a"))) {
            fprintf(stderr, "Cannot open a file [%s]\n", filename);
            exit(-1);
        }

        time_t current_time = NULL;
        pid_t pid = getpid();

        struct tm *time_info;
        char time_string[21]; // space for "_RRRR-MM-DD_GG-MM-SS\0"
        
        time(&current_time);
        time_info = localtime(&current_time);
        strftime(time_string, sizeof(time_string), "%F %H:%M:%S", time_info);

        fprintf(file, "%d %d %s ", pid, freq, time_string);
        int i;
        for (i = 0; i < bytes; ++i) 
            fprintf(file, "%c", (char)random(97, 122));
        fprintf(file, "\n");
        fclose(file);

        sleep(freq);
    }

    

    return 0;
}