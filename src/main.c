#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/fuzzy.h"
#include "../include/logic.h"
#include "../include/ui.h"

static void print_help(void)
{
    printf(
        "pickme - minimal launcher\n"
        "  (no arguments)  open the window (needs a Wayland session)\n"
        "  --logic         text mode: fuzzy-match one stdin line\n"
        "  --help          this text\n"
        "  --version       print version\n");
}

/* Old text-only test path: pipe one line in, get the closest match out. */
static int run_text_mode(void)
{
    str_list* dict = malloc(sizeof(str_list));
    if (dict == NULL) return 1;
    sl_init(dict);
    file_listing(dict);

    char* input = NULL;
    size_t len = 0;
    if (getline(&input, &len, stdin) < 0) {
        perror("getline");
        return 1;
    }
    input[strcspn(input, "\n")] = '\0';
    char* match = find_near_match(input, dict);
    printf("possible match: %s\n", match);
    free(match);
    free(input);
    sl_free(dict);
    free(dict);
    return 0;
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        }
        if (strcmp(argv[i], "--version") == 0) {
            printf("pickme 0.0\n");
            return 0;
        }
    }

    /* Interactive terminal + no arguments -> GUI mode.
     * Piped stdin (or --logic) -> text mode. */
    int use_ui = (argc == 1) && isatty(STDIN_FILENO);
    if (!use_ui) return run_text_mode();

    if (connect_display() != 0) return 1;
    int rc = run_ui();
    cleanup_ui();
    return rc;
}
