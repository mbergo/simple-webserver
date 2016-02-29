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
		case ERROR: (void)sprintf(buffer, "ERROR: %s:%s Errno=%d exiting pid=%d", ret1, ret2, errno.getpid());
	}
	// Logger function
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
