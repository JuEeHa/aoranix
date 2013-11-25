#include <a.h>
#include <aora.h>
#include <terminal.h>
#include <termios.h>

/* TODO: implement per-fd __tm_mode */
int __tm_mode=tm_normal;
struct termios __tm_normalmode, __tm_rawmode;

int
terminit(int fd) {
	if(tcgetattr(fd, &__tm_normalmode))
		return -1;
	
	__tm_rawmode=__tm_normalmode;
	__tm_rawmode.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|INLCR|IGNCR|ICRNL|IXON);
	__tm_rawmode.c_oflag &= ~OPOST;
	__tm_rawmode.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	
	return 0;
}
