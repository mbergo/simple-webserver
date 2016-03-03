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

// Log requests and quits
void logger(int type, char *ret1, char *ret2, int socket)
{
	int fd;
	char buffer[BUFSIZE*2]; // Say no to overflow

	switch (type) {
		case ERROR:
			(void)sprintf(buffer, "ERROR: %s:%s Errno=%d exiting pid=%d", ret1, ret2, errno);
			break;
		case FORBIDDEN:
			(void)write(socket, "HTTP/1.1 Forbidden\nContent-Length: 190\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n",171);
			(void)sprintf(buffer,"FORBIDDEN: %s:%s",ret1, ret2);
			break;
		case NOTFOUND:
			(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n",140);
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

// Lets spawn a subprocess to quit on errors
void web(int fd, int hit) {
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

	// Methods
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

	buflen = strlen(buffer);
	fstr = (char *)0; // why not? :)
	for(x = 0; extensions[x].ext != 0; x++) {
		len = strlen(extensions[x].ext);
		if( !strncmp(&buffer[buflen], extensions[x].ext, len) ) {
			fstr = extensions[x].filetype;
			break;
		}
	}

	if(fstr == 0) logger(FORBIDDEN,"file extension type not supported",buffer,fd);

	if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) {  // open the file for reading
		logger(NOTFOUND, "failed to open file",&buffer[5],fd); //
	}

	logger(LOG,"SEND",&buffer[5],hit);
	len = (long)lseek(file_fd, (off_t)0, SEEK_END); // Size of the file
	      (void)lseek(file_fd, (off_t)0, SEEK_SET); // Move back the seek to read the file
        (void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: SimpleWebserver/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", VERSION, len, fstr); // Header
	logger(LOG,"Header",buffer,hit);
	(void)write(fd,buffer,strlen(buffer));

	while (	(ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
		(void)write(fd,buffer,ret);
	}

	sleep(1);	// Back in the day, we need to "flush" the socket buffer
	close(fd);

	exit(1);
}

int main() {
	int i, port, pid, listenfd, socketfd, hit;
	socklen_t length;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;

	if( argc < 3  || argc > 3 || !strcmp(argv[1], "-?") ) {
		(void)printf("Usage: ./simple-webserver <port> [dir]\t\tversion %d\n\n", VERSION);
		for(i=0;extensions[i].ext != 0;i++) {
			(void)printf(" %s",extensions[i].ext);
		}
		exit(0);
	}

	if(chdir(argv[2]) == -1){
		(void)printf("ERROR: Can't Change to directory %s\n",argv[2]);
		exit(4);
	}

	if(fork() != 0) {
		return 0; // parent returns OK to shell
	}
	(void)signal(SIGCLD, SIG_IGN); // ignore child death
	(void)signal(SIGHUP, SIG_IGN); // ignore terminal hangups

	for(i=0;i<32;i++) {
		(void)close(i);
	}	// close open files

	(void)setpgrp();		// break away from process group
	logger(LOG,"server is starting",argv[1],getpid());

	// setup the network socket
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0) {
		logger(ERROR, "system call","socket",0);
	}

	port = atoi(argv[1]);
	if(port < 0 || port > 65536) {
		logger(ERROR,"Invalid port number (try 1-> 65536)",argv[1],0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		logger(ERROR, "system call", "bind", 0);
	}

	if( listen(listenfd,64) < 0) {
		logger(ERROR,"system call", "listen", 0);
	}

	for(hit=1; ;hit++) {
		length = sizeof(cli_addr);
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
			logger(ERROR, "system call", "accept", 0);
		if((pid = fork()) < 0) {
			logger(ERROR, "system call", "fork", 0);
		} else {
			if(pid == 0) { 	// child
				(void)close(listenfd);
				web(socketfd,hit); // never returns
			} else { 	// parent
				(void)close(socketfd);
			}
		}
	}
}
