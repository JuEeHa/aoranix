/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>

int
termgetsize(int fd, struct termsize *ts) {
	/* TODO: make this actually figure out the terminal size */
	ts->lines=24;
	ts->columns=80;
	return 0;
}
