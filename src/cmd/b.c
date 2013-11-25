#include <a.h>
#include <aora.h>

#include "config/b.h"
/* TODO: add building only if file in $in(-target) or $depfile has been changed */
/* TODO: building cross-b (prefixing commands) */
/* TODO: cross-building with b (prefixing output files) */
#define debg(...) writefmt(1, __VA_ARGS__);

int cflag, iflag, vflag;
char *obj;

void*
xmalloc(usize size) {
	void *ret;
	
	if((ret=malloc(size)) == 0) {
		writestr(2, "b: error: malloc()\n");
		exit(1);
	}
	
	return ret;
}

void*
xrealloc(void *buf, usize size) {
	void *ret;
	
	if((ret=realloc(buf, size)) == 0) {
		writestr(2, "b: error: realloc()\n");
		exit(1);
	}
	
	return ret;
}

void
usage(int fd) {
	writestr(fd, "usage: b [-v] [-h] [-c | -i] [OUT]\n"
	             "\n"
	             "-v    Verbose\n"
	             "-h    Display help and quit\n"
	             "-c    Do clean instead of build\n"
	             "-i    Do install instead of build\n"
	             "\n"
	             "OUT   Override content of $out\n");
}

void
varset(char *name, char *value, int overwrite) {
	if(setenv(name, value, overwrite)) {
		writestr(2, "b: error: setenv()\n");
		exit(2);
	}
	if(vflag && !strcmp(getenv(name), value))
		writefmt(1, "$%s=%s\n", name, value);
}

void
bfile(void) {
	char *buf, *p, *name, *val, *bufend;
	int f, i, r;
	uint line;
	
	if(!exists("bfile"))
		return;
	
	if((f=open("bfile", O_RDONLY)) == -1) {
		writestr(2, "b: can't open bfile\n");
		exit(1);
	}
	
	buf=xmalloc(1024);
	for(i=0; (r=read(f, buf+i, 1024)) && r!=-1;) {
		i+=r;
		buf=xrealloc(buf, (i+1023)/1024*1024);
	}
	close(f);
	bufend=buf+i;
	
	i++;
	buf=xrealloc(buf, (i+1023)/1024*1024);
	buf[i-1]=0;
	
	line=1;
	for(p=buf; p<bufend;) {
		while(isblank(*p) && p<bufend) p++; /* skip any whitespace in the start of name */
		if(p>=bufend)
			break;
		if(*p=='\n') {
			p++;
			line++;
			continue;
		}
		name=p;
		
		if((p=strchr(p, '=')) == 0) {
			writefmt(2, "b: bfile: %u: expected '='\n", line);
			exit(1);
		}
		*p++=0;
		
		while(isblank(*p) && p<bufend) p++; /* as above, but with val */
		if(p>=bufend)
			val="";
		else {
			val=p;
		
			for(;;) {
				if((p=strchr(p, '\n')) == 0) {
					p=bufend;
					break;
				}
				line++;
				if(*(p-1)!='\\')
					break;
				else
					p++;
			}
			*p++=0;
		}
		
		varset(name, val, 1);
	}
	
	free(buf);
}

void
setvars(void) {
	char *tmpbuf;
	uint tmpbuflen;
	DIR *d;
	struct dirent *e;

	varset("prefix", PREFIX, 0);
	varset("execprefix", EXECPREFIX, 0);
	varset("host", HOST, 0);
	varset("target", TARGET, 0);
	varset("objext", OBJEXT, 1);
	
	/* bindir */
	tmpbuf=xmalloc(strlen(getenv("execprefix"))+4+1); /* $execprefix/bin */
	strcpy(tmpbuf, getenv("execprefix"));
	strcat(tmpbuf, "/bin");
	varset("bindir", tmpbuf, 0);
	free(tmpbuf);
	
	/* libdir */
	tmpbuf=xmalloc(strlen(getenv("execprefix"))+4+1); /* $execprefix/lib */
	strcpy(tmpbuf, getenv("prefix"));
	strcat(tmpbuf, "/lib");
	varset("libdir", tmpbuf, 0);
	free(tmpbuf);
	
	/* mandir */
	tmpbuf=xmalloc(strlen(getenv("prefix"))+4+1); /* $prefix/man */
	strcpy(tmpbuf, getenv("prefix"));
	strcat(tmpbuf, "/man");
	varset("mandir", tmpbuf, 0);
	free(tmpbuf);
	
	varset("mode", "one", 1);
	varset("out", "a.out", 1);

	/* in */
	tmpbuf=0;
	tmpbuflen=0;
	d=opendir(".");
	while(e=readdir(d))
		if(!strcmp(strend(e->d_name, 2), ".c")) {
			tmpbuflen+=strlen(e->d_name)+1;
			if(!tmpbuf) {
				tmpbuf=xmalloc(tmpbuflen);
				strcpy(tmpbuf, e->d_name);
			} else {
				tmpbuf=xrealloc(tmpbuf, tmpbuflen);
				strcat(tmpbuf, " ");
				strcat(tmpbuf, e->d_name);
			}
		}
	closedir(d);
	if(tmpbuf)
		varset("in", tmpbuf, 1);
	else
		varset("in", "", 1);
	free(tmpbuf);

	varset("configure", "", 1);
	varset("build", "c $infile", 1);
	varset("link", "l -o $outfile $obj", 1);
	varset("clean", "rm -f $out $obj", 1);
	varset("install", "install $out $bindir", 1);
	varset("depend", "", 1);
	varset("depfile", "", 1);
}

void
runcmd(char *name, char *cmd) {
	int retval;
	
	if(vflag)
		writefmt(1, "%s\n", cmd);
	if((retval=system(cmd)) == -1) {
		writestr(2, "b: eror: system()\n");
		exit(1);
	} else if(retval) {
		writefmt(2, "b: %s exited with %i\n", name, retval);
		exit(1);
	}
}

void
recursedeps(void) {
	char *p, *start, *depdirs, *depdirsend, *cmd;
	uint cmdlen;
	struct stat st;
	
	depdirs=xmalloc(strlen(getenv("depend"))+1);
	strcpy(depdirs, getenv("depend"));
	depdirsend=strend(depdirs, 0);
	
	for(start=depdirs, p=start; p<depdirsend;) {
		p=strchr(p, ' ');
		if(!p) p=depdirsend;
		
		*p=0;
		
		if(!exists(start)) {
			writefmt(2, "b: directory '%s' not found\n", start);
			exit(1);
		}
		if(lstat(start, &st)) {
			writestr(2, "b: error: lstat()\n");
			exit(1);
		}
		if(!S_ISDIR(st.st_mode)) {
			writefmt(2, "b: '%s' not directory\n", start);
			exit(1);
		}
		
		cmdlen=3+strlen(start)+3; /* "cd " start ";b" */
		if(cflag||iflag||vflag) cmdlen+=2;
		if(cflag||iflag) cmdlen++;
		if(vflag) cmdlen++;
		
		cmd=xmalloc(cmdlen+1);
		strcpy(cmd, "cd ");
		strcat(cmd, start);
		strcat(cmd, ";b");
		if(cflag||iflag||vflag) strcat(cmd, " -");
		if(cflag) strcat(cmd, "c");
		if(iflag) strcat(cmd, "i");
		if(vflag) strcat(cmd, "v");
		
		runcmd(start, cmd);
		
		free(cmd);
		
		start=++p;
	}
	
	free(depdirs);
}

void
doclean(void) {
	DIR *d;
	struct dirent *e;
	char *cleanobjs, *objext;
	uint cleanobjslen, objextlen;
	
	objext=xmalloc(1+strlen(getenv("objext"))+1);
	strcpy(objext, ".");
	strcat(objext, getenv("objext"));
	objextlen=strlen(objext);
	cleanobjs=0;
	cleanobjslen=0;
	
	d=opendir(".");
	while(e=readdir(d))
		if(!strcmp(strend(e->d_name, objextlen), objext)) {
			cleanobjslen+=strlen(e->d_name)+1;
			if(!cleanobjs) {
				cleanobjs=xmalloc(cleanobjslen);
				strcpy(cleanobjs, e->d_name);
			} else {
				cleanobjs=xrealloc(cleanobjs, cleanobjslen);
				strcat(cleanobjs, " ");
				strcat(cleanobjs, e->d_name);
			}
		}
	closedir(d);
	if(cleanobjs)
		varset("obj", cleanobjs, 1);
	else
		varset("obj", "", 1);
	free(objext);
	free(cleanobjs);
		
	runcmd("$clean", getenv("clean"));
	
	if(unsetenv("obj")) {
		writestr(2, "b: error: unsetenv()\n");
		exit(1);
	}
}

void
doinstall(void) {
	runcmd("$install", getenv("install"));
}

void
doconfigure(void) {
	runcmd("$configure", getenv("configure"));
}

void
objfile(char *sourcefile) {
	char *buf, *p, *objext;
	
	objext=getenv("objext");
	
	buf=xmalloc(strlen(sourcefile)+1);
	strcpy(buf, sourcefile);
	p=strrchr(buf, '.');
	
	if(p)
		*p=0;
	buf=xrealloc(buf, strlen(buf)+1+strlen(objext)+1);
	strcat(buf, ".");
	strcat(buf, objext);
	
	varset("objfile", buf, 1);
	
	if(!obj) {
		obj=xmalloc(strlen(buf)+1);
		strcpy(obj, buf);
	} else {
		obj=xrealloc(obj, strlen(obj)+1+strlen(buf)+1);
		strcat(obj, " ");
		strcat(obj, buf);
	}
	
	free(buf);
}

void
dobuild(char *outfile) {
	char *p, *start, *infiles, *infilesend, *tmpbuf;

	varset("outfile", outfile, 1);
	
	if(!strcmp(getenv("mode"), "one")) {
		infiles=xmalloc(strlen(getenv("in"))+1);
		strcpy(infiles, getenv("in"));
	} else if(!strcmp(getenv("mode"), "many")) {
		tmpbuf=xmalloc(strlen(outfile)+3+1);
		strcpy(tmpbuf, "in-");
		strcat(tmpbuf, outfile);
		if(getenv(tmpbuf) == 0) {
			infiles=xmalloc(strlen(outfile)+2+1);
			strcpy(infiles, outfile);
			strcat(infiles, ".c");
		} else {
			infiles=xmalloc(strlen(getenv(tmpbuf))+1);
			strcpy(infiles, getenv(tmpbuf));
		}
		free(tmpbuf);
	}
	infilesend=strend(infiles, 0);
	
	for(start=infiles, p=start; p<infilesend;) {
		p=strchr(start, ' ');
		if(!p) p=infilesend;
		
		*p=0;
		
		objfile(start);
		varset("infile", start, 1);
		runcmd("$build", getenv("build"));
		
		start=++p;
	}
	
	free(infiles);
	
	if(unsetenv("outfile") || unsetenv("objfile")) {
		writestr(2, "b: error: unsetenv()\n");
		exit(1);
	}
}

void
dolink(char *outfile) {
	varset("outfile", outfile, 1);
	if(!obj) {
		obj=xmalloc(1); /* this will be freed in main(), after the call to dolink */
		return; /* no reason to even try to link if there are no object files */
	}
	varset("obj", obj, 1);
	
	runcmd("$link", getenv("link"));
	
	if(unsetenv("outfile") || unsetenv("obj")) {
		writestr(2, "b: error: unsetenv()\n");
		exit(1);
	}
}

int
main(int argc, char **argv) {
	char **argp, *p, *tmpbuf, *outfile, *outfiles, *outfilesend;
	uint tmpbuflen;
	
	cflag=0;
	iflag=0;
	vflag=0;
	for(argp=argv+1; *argp&&**argp=='-'; argp++)
		for(p=*argp+1; *p; p++)
			switch(*p) {
				case 'c':
					if(iflag) {
						usage(2);
						return 1;
					}
					cflag=1;
					break;
				case 'h':
					usage(1);
					return 0;
				case 'i':
					if(cflag) {
						usage(2);
						return 1;
					}
					iflag=1;
					break;
				case 'v':
					vflag=1;
					break;
				default:
					usage(2);
					return 1;
			}
	
	setvars();
	bfile();
	recursedeps();
	
	if(*argp)
		if(!strcmp(getenv("mode"), "one")) {
			varset("out", *argp, 1); /* rewrite $out if specified on command line */
			if(*++argp) {/* if more than one arg */
				writestr(2, "b: can't specify more than one target if $mode=one\n");
				exit(1);
			}
		} else if(!strcmp(getenv("mode"), "many")) {
			tmpbuflen=strlen(*argp)+1;
			tmpbuf=xmalloc(tmpbuflen);
			strcpy(tmpbuf, *argp);
			while(*++argp) {
				tmpbuflen+=strlen(*argp)+1;
				tmpbuf=xrealloc(tmpbuf, tmpbuflen);
				strcat(tmpbuf, " ");
				strcat(tmpbuf, *argp);
			}
			
			varset("out", tmpbuf, 1); /* see above */
			free(tmpbuf);
		}
	
	if(cflag)
		doclean();
	else if(iflag)
		doinstall();
	else {
		doconfigure();
		if(!strcmp(getenv("mode"), "one")) {
			outfile=getenv("out");
			obj=0;
			dobuild(outfile);
			dolink(outfile);
			free(obj);
		} else if (!strcmp(getenv("mode"), "many")) {
			outfiles=xmalloc(strlen(getenv("out"))+1);
			strcpy(outfiles, getenv("out"));
			outfilesend=strend(outfiles, 0);
			
			for(outfile=outfiles, p=outfile; p<outfilesend;) {
				p=strchr(outfile, ' ');
				if(!p) p=outfilesend;
				
				*p=0;
				
				obj=0;
				dobuild(outfile);
				dolink(outfile);
				free(obj);
				
				outfile=++p;
			}
			
			free(outfiles);
		} else {
			writefmt(2, "b: invalid $mode '%s'\n", getenv("mode"));
			return 1;
		}
	}
	
	return 0;
}
