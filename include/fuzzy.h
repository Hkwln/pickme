
#ifndef FUZZY_SEACH_H
#define FUZZY_SEACH_H

#include <stdio.h>

#include "logic.h"

size_t levenshtein_distance(const char* s, const char* t);
char* find_near_match(const char* input, str_list* dictonary);

#endif  // FUZZY_SEACH_H
