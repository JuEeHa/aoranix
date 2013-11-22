/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>

int
clearscr(int fd) {
	if(writestr(fd, "\e[2J") == -1)
		return -1;
	return gotoxy(fd, 0, 0);
}
