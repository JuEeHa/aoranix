#include <a.h>
#include <aora.h>

ssize
writeall(int fd, char *s, usize len) {
	usize i, w;
	
	for(i=0, w=0; i<len; i+=w)
		if((w=write(fd, s+i, len-i)) == -1)
			return -1;
		else if(!w)
			break;
	
	return i;
}
