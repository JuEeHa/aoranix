/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>
#include <sys/ioctl.h>

int
termgetsize(int fd, struct termsize *ts) {
	struct winsize w;
	if(ioctl(fd, TIOCGWINSZ, &w) != -1 && w.ws_col>0 && w.ws_row>0) { /* try ioctl */
		ts->columns=w.ws_col;
		ts->lines=w.ws_row;
	} else { /* fallback: 80x24 */
		ts->columns=80;
		ts->lines=24;
	}
	
	return 0;
}
