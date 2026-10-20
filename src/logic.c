
#include "../include/logic.h"

#include <dirent.h>
#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/fuzzy.h"
#include "../include/input_handling.h"
#include "../include/ui.h"

static str_list* dict;
static size_t hits[32];
static char* shown[32];
static size_t shown_n;

str_list* logic_init()
{
    // create dictionary
    dict = malloc(sizeof(str_list));
    if (dict == NULL) {
        fprintf(stderr, "error while creating dictionary");
        return NULL;
    }
    dict->item = NULL;
    dict->exec = NULL;
    dict->cap = dict->len = 0;
    file_listing(dict);
    return dict;
}
void sl_free(str_list* l)
{
    for (size_t i = 0; i < l->len; i++) free(l->item[i]);
    free(l->item);
    for (size_t i = 0; i < l->len; i++) free(l->exec[i]);
    free(l->exec);
    *l = (str_list){0};
}
void append_word(char* word, char* exec, str_list* l)
{
    if (l->cap <= l->len) {
        size_t ncap = (l->cap) ? l->cap * 2 : 8;
        l->item = realloc(l->item, (ncap + 1) * sizeof(char*));
        l->exec = realloc(l->exec, (ncap + 1) * sizeof(char*));
        // ERROR handling
        l->cap = ncap;
    }

    l->exec[l->len] = strdup(exec);
    l->item[l->len] = strdup(word);
    l->len++;
    l->item[l->len] = NULL;
    l->exec[l->len] = NULL;
}

char* get_word(str_list* l, size_t index)
{
    return (l->len > index) ? l->item[index] : NULL;
}
static int is_desktop_file(char* word)
{
    static const char suf[] = ".desktop";
    size_t n = strlen(word);
    return n >= sizeof(suf) - 1 &&
           strcmp(word + n - (sizeof(suf) - 1), suf) == 0;
}

void append_name_exec(str_list* l, char* entry)
{
    FILE* f = fopen(entry, "r");
    bool skip = false;
    if (f == NULL) {
        printf("failed to read file\n");
        exit(EXIT_FAILURE);
    }
    char data[500], name[512] = "", exec[512] = "";
    name[0] = '\0';
    exec[0] = '\0';
    bool in_main = 0;
    while (fgets(data, 500, f) != NULL) {
        data[strcspn(data, "\n")] = '\0';
        if (data[0] == '[') {
            if (in_main) break;
            if (strcmp(data, "[Desktop Entry]") == 0) in_main = 1;
            continue;
        }
        if (!in_main) continue;
        if (name[0] == '\0' && strncmp(data, "Name=", 5) == 0) {
            // Name=
            snprintf(name, sizeof(name), "%s", data + 5);
        } else if (strncmp(data, "Exec=", 5) == 0) {
            // Exec=
            snprintf(exec, sizeof exec, "%s", data + 5);
            char* codes = strstr(exec, " %"); /* trailing %U %F %f %u ... */
            if (codes) *codes = '\0';
        } else if (strncmp(data, "NoDisplay=true", 14) == 0 ||
                   strncmp(data, "Hidden=true", 11) == 0 ||
                   strncmp(data, "Terminal=true", 13) == 0 ||
                   (strncmp(data, "Type=", 5) == 0 &&
                    strcmp(data + 5, "Application") != 0)) {
            skip = true;
            break;
        }
    }
    fclose(f);
    if (name[0] && exec[0] && !skip) {
        append_word(name, exec, l);
    }
}

// TODO: use stat instead of readdir()
void file_listing(str_list* l)
{
    const char* home_dir = getenv("HOME");
    char path_dir[] = "/usr/share/applications";
    char path_dir2[100];  // TODO: add these two to the scan
    char path_dir3[100];
    snprintf(path_dir2, sizeof(path_dir2), "%s/.config/pickme/", home_dir);
    snprintf(path_dir3, sizeof(path_dir3), "%s/.local/share/applications/",
             home_dir);

    char path[PATH_MAX];
    DIR* dir = opendir(path_dir);
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.' && is_desktop_file(entry->d_name)) {
            snprintf(path, sizeof(path), "%s/%s", path_dir, entry->d_name);
            append_name_exec(l, path);
        }
    }
    closedir(dir);
}

void on_key(enum key_action action, const char* utf8)
{
    (void)utf8;
    if (dict == NULL) {
        fprintf(stderr, "error initialize the dictionary first");
        return;
    }
    switch (action) {
        case ACTION_ESCAPE:
            ui_quit();
            return;
        case ACTION_CHAR:
        case ACTION_BACKSPACE: {
// INFO: this is just temporary until I design the search function
#if 1
            char* m = find_near_match(input_query(), dict);
            free(shown[0]);
            shown[0] = m;
            shown_n =
                (m != NULL && m[0] != '\0') ? 1 : 0; /* "" means "no match" */
            hits[0] = 0;
            for (size_t i = 0; i < dict->len; i++)
                if (strcmp(dict->item[i], m) == 0) {
                    hits[0] = i;
                    break;
                }
#endif
            ui_set_list(shown, (int)shown_n);
            break;
        }
        case ACTION_ENTER: {
            int sel = ui_selected();
            if (shown_n > 0 && hits[sel] < dict->len)
                launch(dict->exec[hits[sel]]);
            ui_quit();
            break;
        }
        default:
            return;
    }
}

void launch(char* exec_cmd)
{
    /* Nobody is ever going to wait() for these children, so let the kernel
     * reap them. Without this, every launched app that already exited stays
     * around as a zombie for as long as pickme keeps running.*/
    static int reaping_ready;
    if (!reaping_ready) {
        signal(SIGCHLD, SIG_IGN);
        reaping_ready = 1;
    }

    if (exec_cmd == NULL || exec_cmd[0] == '\0') {
        fprintf(stderr, "pickme: nothing to launch (empty Exec line)\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("pickme: fork");
        return;
    }
    if (pid > 0) return; /* parent: the child takes over from here */

    /* ---- child ---- */

    /* Own session, so it is no longer in pickme's process group: closing the
     * terminal that started pickme can no longer SIGHUP the app, and a
     * Ctrl+C in that terminal no longer hits it. */
    setsid();

    /* A .desktop Exec line is shell-ish (arguments, quoting, env vars), so
     * let sh parse it rather than splitting it ourselves. */
    execl("/bin/sh", "sh", "-c", exec_cmd, (char*)NULL);

    /* Only reached when /bin/sh itself could not be executed. _exit and not
     * exit: we must not flush the parent's stdio buffers a second time. */
    perror("pickme: cannot run /bin/sh");
    _exit(127);
}
