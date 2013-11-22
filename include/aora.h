ssize writeall(int fd, char *s, usize len);
ssize writestr(int fd, char *s);
ssize writefmt(int fd, char *fmt, ...);
char *strend(char *s, usize offset);
int exists(char *path);

#include <string.h>

#include <stdlib.h>

#include <dirent.h>

#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <ctype.h>
