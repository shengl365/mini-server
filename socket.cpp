#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "http.h"


void Socket::setsocket(void){
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
 
    if(-1 == SocketFD)
    {
        printf("can not create socket");
        exit(EXIT_FAILURE);
    }	
}
	
void Socket::bindsocket(unsigned short port){
    bzero(&stSockAddr, sizeof(stSockAddr));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;


    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        printf("error bind failed");
        this->closesocket();
        exit(EXIT_FAILURE);
    }

}

void Socket::listensocket() {

	listen(SocketFD, 10);

	if(-1 == listen(SocketFD, 10)) {
        	printf("error listen failed");
        	this->closesocket();
        	exit(EXIT_FAILURE);
    	}

}


int Socket::acceptsocket() {
	int addrlen = sizeof(cliaddr);
	int ConnectFD = accept(SocketFD, (struct sockaddr*) &cliaddr, (socklen_t*) &addrlen);
	return ConnectFD;
}


void Socket::closesocket() {
	close(SocketFD);
}


