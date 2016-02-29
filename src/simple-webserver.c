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
#define VERSION 23
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
				<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type
					or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
			(void)sprintf(logbuffer,"FORBIDDEN: %s:%s",ret1, ret2);
			break;
		case NOTFOUND:
			(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\n
				Content-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n
					<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
			(void)sprintf(logbuffer,"NOT FOUND: %s:%s",ret1, ret2);
			break;
		case LOG:
			(void)sprintf(buffer," INFO: %s:%s:%d",ret1, ret2,socket);
			break;
	}
		if((fd = open("nweb.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
			(void)write(fd,logbuffer,strlen(logbuffer));
			(void)write(fd,"\n",1);
			(void)close(fd);
		}

		if(type == ERROR || type == NOTFOUND || type == FORBIDDEN) exit(3);
	}

// Lets spawn a subprocess to quit on errors
void web()
{
	// Main web function here
	exit(1);
}

int main()
{
// Here we run


}
