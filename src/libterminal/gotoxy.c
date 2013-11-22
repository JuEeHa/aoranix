/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>

int
gotoxy(int fd, uint x, uint y) {
	if(writefmt(fd, "\e[%u;%uH", y+1, x+1) == -1)
		return -1;
	return 0;
}
