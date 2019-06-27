
#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

void create_table(int size);
void search_directory(char* dir, char* filename, char* temp_filename);
int save_current_file(char* tmp_file);
void remove_block(int index);

struct extended_arr
{
	char **blocks;
	int size;
} *table;

#endif

