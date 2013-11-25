/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>

/* TODO: implement per-fd __tm_mode */
extern int __tm_mode;

int
termgetmode(int fd) {
	return __tm_mode;
}
