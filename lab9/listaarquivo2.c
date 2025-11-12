#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int alphasort();
char pathname[MAXPATHLEN];

int file_select(struct direct *entry) {
  if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    return 0;
  else
    return 1;
}

int main() {
  int count, i;
  struct direct **files;

  if (getwd(pathname) == NULL) {
    printf("Error getting path\n");
    exit(1);
  }

  printf("Current Working Directory = %s\n", pathname);
  count = scandir(pathname, &files, file_select, alphasort);

  if (count <= 0) {
    printf("No files in this directory\n");
    exit(0);
  }

  printf("Number of files = %d\n", count);
  printf("--------------------------------------------------------------------------\n");
  printf("%-25s %-10s %-10s %-10s %-10s\n", "File", "Inode", "Size", "Links", "Age(days)");
  printf("--------------------------------------------------------------------------\n");

  for (i = 0; i < count; i++) {
      char fullpath[MAXPATHLEN];
      struct stat info;
      time_t now = time(NULL);

      sprintf(fullpath, "%s/%s", pathname, files[i]->d_name);

      if (stat(fullpath, &info) == 0) {
        double age_days = difftime(now, info.st_mtime) / (60 * 60 * 24);
        printf("%-25s %-10lu %-10ld %-10lu %-10.1f\n",
                files[i]->d_name,
                (unsigned long) info.st_ino,
                (long) info.st_size,
                (unsigned long) info.st_nlink,
                age_days);
      } else {
        perror("stat");
      }

    free(files[i]);
  }

  free(files);
  return 0;
}