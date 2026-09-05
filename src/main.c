#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>

#include "fuzzy.h"
#include "logic.h"

int main(int argv, const char** argc)
{
    /* Argument parsing: XXX: NOT FINISHED*/
    for (size_t i = 1; i < argv; i++) {
        if (strcmp(argc[1], "--help") == 0) {
            printf(
                "Here you can find help:\n --version\tgives you the current "
                "version\n --verbose\tactivates verbose mode\n");
        } else if (strcmp(argc[i], "--version") == 0) {
            printf("currently no version\n");
        } else if (strcmp(argc[i], "--verbose")) {
            printf("verbose output activated\n");
            // #define VERBOSE
        } else {
            printf("your options don't match anything\n");
        }
    }

    printf("Without Ui modus:\n");
    str_list* dict = malloc(sizeof(str_list));
    sl_init(dict);
    file_listing(dict);

    char* input = NULL;
    size_t len = 0;
    ssize_t read = 0;
    read = getline(&input, &len, stdin);
    char* matches = find_near_match(input, dict);
    printf("possible match: %s\n", matches);
    free(matches);
    sl_free(dict);
    return 0;
}
