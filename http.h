#ifndef SOCKET_H
#define SOCKET_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class Socket {
public:
	struct sockaddr_in stSockAddr, cliaddr;
	int SocketFD;
	void setsocket();
	void bindsocket(unsigned short);
	void listensocket();
	int acceptsocket();
	void closesocket();
};

#endif
