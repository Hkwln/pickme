
#include "fuzzy.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "logic.h"
#define MAXDISTANCE 20

// using levenstein to calc distance -> word with least distance wins
size_t levenshtein_distance(char* s, char* t)
{
    size_t n = strlen(s), m = strlen(t);
    size_t* prev = malloc((m + 1) * sizeof(*prev));
    size_t* cur = malloc((m + 1) * sizeof(*cur));

    for (size_t j = 0; j <= m; j++) prev[j] = j;  // D[0][j] = [j]
    for (size_t i = 1; i <= n; i++) {
        cur[0] = i;
        for (size_t j = 1; j <= m; j++) {
            size_t del = prev[j] + 1;
            size_t ins = cur[j - 1] + 1;
            size_t subs =
                prev[j - 1] + (s[i - 1] != t[j - 1]);  // match = 0 ; subs = 0
            cur[j] = (subs < del) ? (subs < ins ? subs : ins)
                                  : (del < ins ? del : ins);
        }
        size_t* tmp = prev;
        prev = cur;
        cur = tmp;  // real swap
    }

    // after the last swap, the results of v1 are now in v0
    size_t distance = prev[m];
    free(prev);
    free(cur);
    return distance;
}

/* XXX: remember to free() the returned word and to free the dictonary
 * */
void* find_near_match(char* input, str_list* dictonary)
{
    size_t d = 0;
    char* word = get_word(dictonary, d++);
    char* current_smallest = malloc(50);
    current_smallest[0] = '\0';
    size_t smallest = MAXDISTANCE;
    size_t tmp = 0;
    while (word != NULL) {
        tmp = levenshtein_distance(input, word);
        // normally i would use a if for this, but this time we use ternary just
        // for fun
        smallest =
            tmp < smallest ? (strcpy(current_smallest, word), tmp) : smallest;
        word = get_word(dictonary, d++);
    }
    return current_smallest;
}
