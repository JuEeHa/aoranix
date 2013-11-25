AUTOLIB(terminal);

#define tm_normal 0
#define tm_raw    1

struct termsize {
	uint lines;
	uint columns;
};

int terminit(int fd);
int termgetsize(int fd, struct termsize *ts);
int termgetmode(int fd);
int termsetmode(int fd, int mode);
int gotoxy(int fd, uint x, uint y);
int clearsrc(int fd);
