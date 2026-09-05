
#include "logic.h"

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void sl_init(str_list* l)
{
    l->item = NULL;
    l->exec = NULL;
    l->cap = l->len = 0;
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
    return (index >= 0 && (l->len > index)) ? l->item[index] : NULL;
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
    char data[1024], name[512] = "", exec[512] = "";
    name[0] = '\0';
    exec[0] = '\0';
    while (fgets(data, 500, f) != NULL) {
        data[strcspn(data, "\n")] = '\0';

        if (strncmp(data, "Name=", 5) == 0) {
            // Name=
            strcat(name, data + 5);
        } else if (strncmp(data, "Exec=", 5) == 0) {
            // Exec=
            snprintf(exec, sizeof exec, "%s", data + 5);
            char* codes = strstr(exec, " %");
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
    char path_dir[] = "/usr/share/applications";
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
void launch(char* exec_cmd)
{
    pid_t pid = fork();
    if (pid == 0) {
        // child: .desktop Exec → /bin/sh -c "<stored command>"
        // user script → execl(path, path, NULL)
        execl("/bin/sh", "sh", "-c", exec_cmd, (char*)NULL);
        _exit(127);  // only reached if exec fails
    }
}
