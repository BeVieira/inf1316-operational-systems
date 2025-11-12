#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define PATH_MAX 4096

void print_tree(const char *path, const char *prefix) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    int total = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        total++;
    }

    rewinddir(dir);

    int idx = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        idx++;
        int is_last = (idx == total);

        printf("%s%s%s\n",
               prefix,
               is_last ? "└── " : "├── ",
               entry->d_name);

        char child_path[PATH_MAX];
        struct stat st;

        sprintf(child_path, "%s/%s", path, entry->d_name);

        if (stat(child_path, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            char new_prefix[PATH_MAX];
            sprintf(new_prefix, "%s%s",
                    prefix,
                    is_last ? "    " : "│   ");

            print_tree(child_path, new_prefix);
        }
    }

    closedir(dir);
}

int main(void) {
    char path[PATH_MAX];

    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd");
        return 1;
    }

    printf("%s\n", path);
    print_tree(path, "");

    return 0;
}
