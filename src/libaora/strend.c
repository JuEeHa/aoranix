#include <a.h>
#include <aora.h>

char*
strend(char *s, usize offset) {
	char *p;
	for(p=s; *p; p++);
	p-=offset;
	if(p<s)
		return s;
	return p;
}
