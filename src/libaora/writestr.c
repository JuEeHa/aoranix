#include <a.h>
#include <aora.h>

ssize
writestr(int fd, char *s) {
	return writeall(fd, s, strlen(s));
}
