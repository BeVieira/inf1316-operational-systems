#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define PATH_MAX 4096

long long soma_dir(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[PATH_MAX];
    long long total = 0;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (stat(fullpath, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) { /*subdiretório*/
            total += soma_dir(fullpath);
        } else if (S_ISREG(st.st_mode)) { /*arquivo regular*/
            total += st.st_size;
        } else { /*outro tipo de arquivo*/
            continue;
        }
    }

    closedir(dir);
    return total;
}

int main(void) {
    char path[PATH_MAX];

    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd");
        return 1;
    }

    printf("Diretório inicial: %s\n", path);

    long long total = soma_dir(path);

    printf("Tamanho total dos arquivos (bytes): %lld\n", total);

    return 0;
}
