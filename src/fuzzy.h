
#ifndef FUZZY_SEACH_H
#define FUZZY_SEACH_H

#include <stdio.h>

#include "logic.h"

size_t levenshtein_distance(char* s, char* t);
void* find_near_match(char* input, str_list* dictonary);

#endif  // FUZZY_SEACH_H
