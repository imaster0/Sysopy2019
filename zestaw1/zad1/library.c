#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


int save_current_file(char* tmp_file) {

	if (tmp_file == NULL) {
		fprintf(stderr, "Tmp_file cannot be NULL");
		exit(EXIT_FAILURE);
	}

	FILE *file = fopen(tmp_file, "r");
	fseek(file, 0, SEEK_END);
	
	size_t len = ftell(file);
	rewind(file);
	
	char *content = calloc(len, sizeof(char));	
	fread(content, sizeof(char), len, file);
	
	int index = -1, i;
	for (i = 0; i < table->size; i++) {
		if (table->blocks[i] == NULL) {
			table->blocks[i] = content; 
			index = i;
			break;	
		}
	}

	return index;
}


void create_table(int size) {

	if (size <= 0) {
		fprintf(stderr, "Size must be greater than 0");
		exit(EXIT_FAILURE);
	}
	
	if (table != NULL) {
		free(table);
		table = NULL;
	}

	table = calloc(1, sizeof(struct extended_arr));
	table->blocks  = calloc(size, sizeof(char*));
	table->size = size;	
}


void search_directory(char* dir, char* file, char* tmp_file) {

	if (dir == NULL || file == NULL || tmp_file == NULL) {
		fprintf(stderr, "Dir/filename/tmp_filename cannot be NULL");
		exit(EXIT_FAILURE);
	}

	size_t mem = snprintf(NULL, 0, "find \"%s\" -name \"%s\" > \"%s\" 2>/dev/null", dir, file, tmp_file);	
	
	char *buffer = malloc(mem + 1);
	sprintf(buffer, "find \"%s\" -name \"%s\" > \"%s\" 2>/dev/null", dir, file, tmp_file);
	
	system(buffer);
}


void remove_block(int index) {

	if (table->size <= index) {
		fprintf(stderr, "Too small array");
		exit(EXIT_FAILURE);
	}	

	free(table->blocks[index]);
	table->blocks[index] = NULL;
}


