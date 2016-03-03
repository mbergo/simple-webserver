#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define VERSION 28
#define BUFSIZE 8096
#define ERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404

struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif" },
	{"jpg", "image/jpg" },
	{"jpeg","image/jpeg"},
	{"png", "image/png" },
	{"ico", "image/ico" },
	{"zip", "image/zip" },
	{"gz",  "image/gz"  },
	{"tar", "image/tar" },
	{"htm", "text/html" },
	{"html","text/html" },
	{0,0} };

/* Log requests and quits */
void logger(int type, char *ret1, char *ret2, int socket)
{
	int fd;
	char buffer[BUFSIZE*2]; // Say no to overflow

	switch (type) {
		case ERROR:
			(void)sprintf(buffer, "ERROR: %s:%s Errno=%d exiting pid=%d", ret1, ret2, errno.getpid());
			break;
		case FORBIDDEN:
			(void)write(socket, "HTTP/1.1 Forbidden\nContent-Length: 190\nConnection: close\nContent-Type: text/html\n\n
				<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n",171);
			(void)sprintf(buffer,"FORBIDDEN: %s:%s",ret1, ret2);
			break;
		case NOTFOUND:
			(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\n
				Content-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n",140);
			(void)sprintf(buffer,"NOT FOUND: %s:%s",ret1, ret2);
			break;
		case LOG:
			(void)sprintf(buffer," INFO: %s:%s:%d",ret1, ret2, socket);
			break;
	}

	  // Errors verification ?
		if((fd = open("web.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
			(void)write(fd,buffer,strlen(buffer));
			(void)write(fd,"\n",1);
			(void)close(fd);
		}

		if(type == ERROR || type == NOTFOUND || type == FORBIDDEN) exit(3);
}

/* Lets spawn a subprocess to quit on errors */
void web(int fd, int hit)
{
	int x, file_fd, buflen;
	long y, ret, len;
	char *fstr;
	static char buffer[BUFSIZE+1]; // Zero filled memory

	ret = read(fd,buffer,BUFSIZE); // Read the request

	if ( ret == 0 || ret == -1) {
		logger(FORBIDDEN, "Failed to read browser request", "", fd);
	}
	if ( ret > 0  && ret < BUFSIZE) {
		buffer[ret] = 0;
	} else {
		buffer[0] = 0;
	}

	for (x = 0; x < ret; x++) {
		if (buffer[x] == '\r' || buffer[x] == '\n') {
			buffer[x] = '#';
		}
	}
	logger(LOG, "request", buffer, hit);

	/* Methods */
	if ( strncmp( buffer, "GET", 4) && strncmp( buffer, "get", 4) ) { // Smart and dumb han?!
		logger(FORBIDDEN, "Only GET permited", buffer, fd);
	}

	for (x = 4; x < BUFSIZE; x++) {
		if ( buffer[x] == ' ' ) {
			buffer[x] = 0;
			break;
		}
	}

	for (y = 0; y < x-1; y++) {
		if (buffer[y] == '.' && buffer[y+1] == '.' ) {
			logger(FORBIDDEN, "Access denied", buffer, fd);
		}
	}

	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) { // convert filename to index file
			(void)strcpy(buffer,"GET /index.html");
	}
	exit(1);
}

int main()
{
// Here we run
}
