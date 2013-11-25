/* aora terminal control API implementation for *nix and ANSI compatible terminals */
#include <a.h>
#include <aora.h>
#include <terminal.h>
#include <termios.h>

/* TODO: implement per-fd __tm_mode */
extern int __tm_mode;
extern struct termios __tm_normalmode, __tm_rawmode;

int
termsetmode(int fd, int mode) {
	if(mode==tm_normal) {
		__tm_mode=mode;
		return tcsetattr(fd, TCSADRAIN, &__tm_normalmode);
	} else if(mode==tm_raw) {
		__tm_mode=mode;
		return tcsetattr(fd, TCSADRAIN, &__tm_rawmode);
	}
	return -1;
}
