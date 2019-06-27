#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dlfcn.h>

enum level
{
	SMALL = 0,
	MEDIUM = 1,
	LARGE = 2
};

//tests
void search_file_test(enum level level, short clock);
void save_blocks_test(enum level level, short clock);
void delete_blocks_test(enum level level, short clock);
void add_and_delete_test(short clock);


void init_time(struct rusage *rusage_start, struct timeval *realtime_start)
{
	getrusage(RUSAGE_SELF, rusage_start);
	struct timezone timezone = {0, 0};
	gettimeofday(realtime_start, &timezone);
}


void print_time(struct rusage *rusage_start, struct timeval *realtime_start, FILE *file)
{

	struct rusage *rusage_end = malloc(sizeof(struct rusage));
	struct timeval *realtime_end = malloc((sizeof(struct timeval)));

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


struct rusage* rusage_start;
struct timeval* realtime_start;
FILE* file;

short is_uint(char* text) {
	
	int i;
	for (i = 0; i < strlen(text); i++) {
		if (text[i] < '0' || text[i] > '9') {
			fprintf(stderr, "[%s] argument should be int", text);
			exit(-1);
			return 0;
		}
	}
	return 1;
}

// DLL
void *handle;
void (*create_table)(int);
void (*search_directory)(char *, char *, char *);
int (*save_current_file)(char *);
void (*remove_block)(int);


int main(int argc, char* argv[]) { 

	int last_index = 1;
    char *error;

    handle = dlopen("./libmy_library_dynamic.so", RTLD_LAZY);

    if (!handle)
    {
        fputs(dlerror(), stderr);
        exit(EXIT_FAILURE);
    }

    create_table = dlsym(handle, "create_table");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(EXIT_FAILURE);
    }

    search_directory = dlsym(handle, "search_directory");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(EXIT_FAILURE);
    }

    save_current_file = dlsym(handle, "save_current_file");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(EXIT_FAILURE);
    }

    remove_block = dlsym(handle, "remove_block");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(EXIT_FAILURE);
    }
    
    rusage_start = malloc(sizeof(struct rusage));
	realtime_start = malloc(sizeof(struct timeval));
	file = fopen("../raport3a.txt", "a");
	if (file == NULL) {
		fprintf(stderr, "Error with file\n");
		exit(EXIT_FAILURE);
	}

	while (argc > last_index) {     
		if (strcmp(argv[last_index], "search_directory") == 0) {  
			assert(argc > last_index + 3);  
			(*search_directory)(argv[last_index + 1], argv[last_index + 2], argv[last_index + 3]); 
			int saved_index = save_current_file(argv[last_index + 3]); 
			if (saved_index == -1) {
				printf("Table is full\n");
				exit(EXIT_FAILURE);
			}
			last_index += 3;
		} else if (strcmp(argv[last_index], "create_table") == 0 && 
					is_uint(argv[last_index + 1])) { 
			(*create_table)(atoi(argv[last_index + 1])); 
			last_index++;
		} else if (strcmp(argv[last_index], "remove_block") == 0 && 
					is_uint(argv[last_index + 1])) {
			(*remove_block)(atoi(argv[last_index + 1]));
			last_index++;
		} else if (strcmp(argv[last_index], "search_file_test") == 0 &&
					is_uint(argv[last_index + 1])) {
			search_file_test(atoi(argv[last_index + 1]), 1);
			last_index++;
		} else if (strcmp(argv[last_index], "save_blocks_test") == 0 &&
					is_uint(argv[last_index + 1])) {
			save_blocks_test(atoi(argv[last_index + 1]), 1);
			last_index++;
		} else if (strcmp(argv[last_index], "delete_blocks_test") == 0 &&
					is_uint(argv[last_index + 1])) { 
			delete_blocks_test(atoi(argv[last_index + 1]), 1);
			last_index++;
		} else if (strcmp(argv[last_index], "add_and_delete_test") == 0) {
			add_and_delete_test(1);
		} else { 
			fprintf(stderr, "Invalid command [%s]\n", argv[last_index]);       
			exit(-1);
		}

		last_index++;
	}
    
    dlclose(handle);

    return 0; 
}

// - przeprowadzenie przeszukania katalogów o różnych poziomach zagłębień 
// i różnych liczbach zawartych plików
// (omownie - dla zawierającego mało, średnią liczbę i dużo plików i podkatalogów)
void search_file_test(enum level level, short clock) {
	
	if (clock) init_time(rusage_start, realtime_start);
	switch (level) {
		case SMALL:
			search_directory("../", "*.c", "tmp_small.txt");
			break;
		case MEDIUM:
			search_directory("../../../../", "c", "tmp_medium.txt");
			break;
		case LARGE:
			search_directory("/", "l*", "tmp_large.txt");
			break;
	}
	if (clock) print_time(rusage_start, realtime_start, file);
}

// - zapisanie w pamięci bloków o różnych rozmiarach
// (odpowiadających rozmiarom różnych przeprowadzonych przeszukiwań)
void save_blocks_test(enum level level, short clock) {

	create_table(1);
	search_file_test(level, 0);

	int index;
	if (clock) init_time(rusage_start, realtime_start);
	switch (level) {
		case SMALL:
			index = (*save_current_file)("tmp_small.txt");
			break;
		case MEDIUM:
			index = (*save_current_file)("tmp_medium.txt");
			break;
			index = (*save_current_file)("tmp_large.txt");
		case LARGE:
			break;
	}
	if (clock) print_time(rusage_start, realtime_start, file);
}

// - usunięcie zaalokowanych bloków o różnych rozmiarach
// (odpowiadających rozmiarom różnych przeprowadzonych przeszukiwań)
void delete_blocks_test(enum level level, short clock) {
	
	save_blocks_test(level, 0);
	if (clock) init_time(rusage_start, realtime_start);
	(*remove_block)(0);
	if (clock) print_time(rusage_start, realtime_start, file);
}

// - na przemian kilkakrotne dodanie i usunięcie zadanej liczby bloków
void add_and_delete_test(short clock) {

	int i;
	if (clock) init_time(rusage_start, realtime_start);
	for (i = 0; i < 30; i++) {
		save_blocks_test(MEDIUM, 0);
		(*remove_block)(0);
	}
	if (clock) print_time(rusage_start, realtime_start, file);
}