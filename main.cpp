#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <iostream>
#include <dirent.h>
#include <map>
#include <cstring>
#include "http.h"
#include "request.h"
using namespace std;

#define BUFSIZE 1024


void request_handler(int fd);
void response_return(int fd, Request &req);

int main(int argc, char *argv[]) {

	Socket sock;
	int sock_accept;

	if(argc < 2) {
		printf("No port!");
		exit(1); 
	}

	unsigned short arg_port = (unsigned short)strtoul(argv[1], NULL, 0);

	sock.setsocket();
	sock.bindsocket(arg_port);
	sock.listensocket();


	while(1) {
		sock_accept = sock.acceptsocket();
		if(sock_accept > 0)
        	{
            		request_handler(sock_accept);
            		sock.closesocket();
            		exit(EXIT_FAILURE);
        	}
		
		sock.closesocket();
	}

}


void request_handler(int fd){

	Request req;
	char p[BUFSIZE] = {};

	recv(fd,p,sizeof(p),0);
	req.parse_http_head(p);
	response_return(fd, req);
}

void response_return(int fd, Request &req) {


	int len = req.res.status.length();
	char *foo = (char*)malloc(sizeof(char) * len); 
	strcpy(foo, req.res.status.c_str());
	send(fd,foo,len+1,0);

	len = req.res.data.size();
	cout << len << endl;
	char *data = (char*)malloc(sizeof(char) * len); 
	strcpy(data, req.res.data.c_str());
	send(fd,data,len+1,0);
}

