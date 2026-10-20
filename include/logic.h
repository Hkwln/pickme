
#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    char** item;
    char** exec;
    size_t len, cap;
} str_list;

str_list* logic_init();
void sl_free(str_list* l);

// scan .Desktop files for programs to launch ->puts them into a dynamic array
void file_listing(str_list* list);
/*puts a new word in the dictonary*/
void append_word(char* word, char* exec, str_list* l);

char* get_word(str_list* l, size_t index);

void launch(char* exec_cmd);

/* on_key() is the keyboard hook. Its prototype already lives in
 * include/input_handling.h - include that header where you define it:
 *     void on_key(enum key_action action, const char* utf8);
 * (A bare "void on_key()" is not a prototype: it takes no arguments and
 * breaks the build if the semicolon is missing.) */

#endif  // LOGIC_H
