
#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include <stdlib.h>
typedef struct {
    char** item;
    char** exec;
    size_t len, cap;
} str_list;

void sl_init(str_list* l);
void sl_free(str_list* l);

// scan .Desktop files for programs to launch ->puts them into a dynamic array
void file_listing(str_list* list);
/*puts a new word in the dictonary*/
void append_word(char* word, char* exec, str_list* l);

char* get_word(str_list* l, size_t index);

void launch(char* exec_cmd);
#endif  // LOGIC_H
