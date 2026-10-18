
#include "../include/fuzzy.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../include/logic.h"
#define MAXDISTANCE 20
#define _GNU_SOURCE

// using levenstein to calc distance -> word with least distance wins
// FIX: ich muss auf jeden fall noch hier irgendwas verändern, vermutlich
// einfach immer die größe 1:1 vergleichen statt mit dem ganzen text oder den
// nächst matchenden inhalt suchen, wenn ich browser eingebe sollte Zen browser
// herauskommen und nicht Piper!!!!
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
char* find_near_match(char* input, str_list* dictionary)
{
    char* best_word = NULL;
    char* current_smallest = malloc(50);
    current_smallest[0] = '\0';
    long best = (long)MAXDISTANCE;
    size_t d = 0;
    char* word;
    while ((word = get_word(dictionary, d++)) != NULL) {
        long score = levenshtein_distance(input, word);
        // INFO: add prefix  and a substring bonux :)
        if (strncasecmp(word, input, strlen(input)) == 0) {
            score -= 200;
        }
        if (strcasestr(word, input) != NULL) {
            score -= 100;  // substring match = huge bonus
        }
        if (score < best) {
            best = score;
            free(best_word);
            best_word = strdup(word);
        }
    }
    return best_word ? best_word : strdup("");
}
