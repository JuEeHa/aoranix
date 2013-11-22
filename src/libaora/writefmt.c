#include <a.h>
#include <aora.h>
#include <stdarg.h>

ssize
writefmt(int fd, char *fmt, ...) {
	va_list args;
	char *p, *str, tmp[64], sigflag;
	int count, intarg; 
	uint uintarg;
	
	va_start(args, fmt);
	
	count=0;
	for(p=fmt; *p; p++)
		if(*p=='%') {
			switch(*++p) {
				case 'c':
					tmp[0]=va_arg(args, int);
					tmp[1]=0;
					str=tmp;
					break;
				case 'i':
					intarg=va_arg(args, int);
					sigflag=(intarg & (1<<(sizeof(int)*8-1)))>>(sizeof(int)*8-1);
					if(sigflag)
						intarg=~intarg+1;
					if(intarg==0)
						str="0";
					else {
						memset(tmp, 0, 64);
						
						while(intarg) {
							memmove(tmp+1, tmp, 62);
							*tmp='0'+intarg%10;
							intarg/=10;
						}
						
						if(sigflag) {
							memmove(tmp+1, tmp, 62);
							*tmp='-';
						}
						
						str=tmp;
					}
					break;
				case 's':
					str=va_arg(args, char*);
					break;
				case 'u':
					uintarg=va_arg(args, uint);
					if(uintarg==0)
						str="0";
					else {
						memset(tmp, 0, 64);
						
						while(uintarg) {
							memmove(tmp+1, tmp, 62);
							*tmp='0'+uintarg%10;
							uintarg/=10;
						}
						
						str=tmp;
					}
					break;
				default:
					str="";
			}
			writestr(fd, str);
			count+=strlen(str);
		} else {
			write(fd, p, 1);
			count++;
		}

	va_end(args);
	
	return count;
}
