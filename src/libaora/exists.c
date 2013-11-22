#include <a.h>
#include <aora.h>

int
exists(char *path) {
	return !access(path, F_OK);
}
