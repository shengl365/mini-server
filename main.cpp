#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <map>

#define BUFSIZE 1024
using namespace std;

void serv_client(int fd, struct sockaddr_in* sin, char* argport, char* argpath) {

	string root = argpath;
	string port = argport;
	pid_t pid;
	int fd1[2], fd2[2];

	int len, err;
	char buffer[BUFSIZE]={0}, head_content[BUFSIZE];
	int file_fd, buflen;
	map<string, string> http_request;
	map<string, string> extensions={
    	{"gif", "image/gif" },
    	{"jpg", "image/jpeg"},
    	{"jpeg","image/jpeg"},
    	{"png", "image/png" },
    	{"zip", "image/zip" },
    	{"gz",  "image/gz"  },
   	{"tar", "image/tar" },
    	{"htm", "text/html" },
    	{"html","text/html" },
    	{"exe","text/plain" },
	{"txt","text/plain" }
	};

	printf("connected from %s:%d\n",inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));

	long ret = read(fd,buffer,BUFSIZE);
	if (ret==0||ret==-1) {exit(3);}


	const char *head = buffer;
	const char *tail = buffer;
	const char *temp = head;

	while (*tail != ' ') ++tail;
	http_request["Type"] = string(head, tail);

	while (*tail == ' ') ++tail;
	head = tail;
	while (*tail != ' ') ++tail;
	http_request["Path"] = string(head, tail);
	size_t pos = http_request["Path"].find('?');		//location of '?'

	string origin = http_request["Path"].substr(0,pos);	//"origin"= /dir/xxx.html |?
	string query = http_request["Path"].substr(pos+1);

	string exe_path;
	if(root.find("/")==root.length()) exe_path=root.append(origin, 1, origin.length()-1);
	else exe_path=root.append(origin, 0, origin.length()); //"exe_path" = root/dir/xxx.html

	while (*tail == ' ') ++tail;
	head = tail;
	while (*tail != '\r') ++tail;
	http_request["Version"] = string(head, tail);

	for(int i = 0; i<2; i++) ++tail;
	head = tail;
	tail++;

	int j = 0;
	while (*head != '\r' && j<200) {
        	while (*tail != ':' && j<200) {++tail; j++;}
        	const char* colon = tail;
		while (*tail != '\r') ++tail;
        	http_request[string(head, colon)] = string(colon+2, tail);
        	head = tail+2;
		j++;
	}
	j = 0;
	head++;
	tail++;
	tail++;
	if(http_request["Type"]=="POST"){
		while (j<100) {++tail;j++;}
        	http_request["POST_request"] = string(head, tail);
		printf("%s\n", string(head, tail).c_str());
	}



//--------dir or not----------------------------

	struct stat buf;	//check st.st_mode==dir?
	DIR * dir;
	struct dirent * ptr;

	stat(exe_path.c_str(), &buf);

	bool isdir = S_ISDIR(buf.st_mode);
	size_t dir_last = http_request["Path"].rfind('/');	//location of last '/'

	if(isdir) {

		if(pos==(dir_last+1) || (dir_last+1) == ' '){ // case:xxx/dir/?n=...

			if((dir =opendir(exe_path.c_str()))==NULL){
			sprintf(head_content,"%s 404 NOT FOUND\r\nServer: test\r\nContent-Type: text\r\nContent-Length: %ld\r\n\
Connection: %s\r\nKeep-alive: timeout=20\r\nAccept-Ranges: bytes\r\n\r\n", \
			http_request["Version"].c_str(), head-temp, http_request["Connection "].c_str());

			write(fd,head_content,strlen(head_content));

			char html[BUFSIZE]="<!doctype html>\r\n<html lang=\"en\">\r\n<head>\r\n<meta charset=\"utf-8\">\r\n\
<title>404 Not Found</title>\r\n</head>\r\n<body>\r\n\r\n";

			write(fd,html, sizeof(html));
			}	
    			while((ptr = readdir(dir)) != NULL){
				if(strstr(ptr->d_name,"index")){

				exe_path = exe_path + "index.html";

				goto enddir;
				}
				
        			write(fd, ptr->d_name,strlen(ptr->d_name));
				write(fd, "\r\n", 2);
    			}
    			closedir(dir);
		return;
		}

		

		else{ //case:xxx/dir?n=...

			string exe_path_slash = exe_path + "/";//301 Moved Permanently

			exe_path_slash = "http:/" + http_request["Host"] + exe_path_slash;

			sprintf(head_content,"\r\n%s 301 Moved Permanently\r\nServer: test\r\nContent-Type: text/html\r\nContent-Length: %ld\r\nConnection: %s\r\nKeep-alive: timeout=20\r\nAccept-Ranges: bytes\r\nLocation: %s\r\n\r\n", \
			http_request["Version"].c_str(), head-temp, http_request["Connection"].c_str(),exe_path_slash.c_str());

			write(fd,head_content,strlen(head_content));

			char html[]="<!doctype html>\r\n<html lang=\"en\">\r\n<head>\r\n<meta charset=\"utf-8\">\r\n<title>301 Moved Permanently</title>\r\n</head>\r\n<body>\r\n\r\n";

			write(fd, html, strlen(html));

			return;
		}
	}
enddir:
//--------------------------------------------------


	pos = origin.find('.');	
	string type = origin.substr(pos+1); // root/dir/xxx| "type"=html

	err = access(exe_path.c_str(),F_OK);

//not exist
	if(((file_fd=open(exe_path.c_str(),O_RDONLY))==-1) && (err==-1)) {

	sprintf(head_content,"%s 403 Forbidden\r\nServer: test\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\
Connection: %s\r\nKeep-alive: timeout=20\r\nAccept-Ranges: bytes\r\n\r\n", \
	http_request["Version"].c_str(), extensions[type].c_str(), head-temp, http_request["Connection "].c_str());

	write(fd,head_content,strlen(head_content));

	char html[BUFSIZE]="<!doctype html>\r\n<html lang=\"en\">\r\n<head>\r\n<meta charset=\"utf-8\">\r\n\
<title>403 Forbidden</title>\r\n</head>\r\n<body>\r\n\r\n";

	write(fd, html, sizeof(html));
	}
//inaccessible
	else if((file_fd == -1) && (err==0)) {
	sprintf(head_content,"%s 404 NOT FOUND\r\nServer: test\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\
Connection: %s\r\nKeep-alive: timeout=20\r\nAccept-Ranges: bytes\r\n\r\n", \
	http_request["Version"].c_str(), extensions[type].c_str(), head-temp, http_request["Connection "].c_str());

	write(fd,head_content,strlen(head_content));

	char html[BUFSIZE]="<!doctype html>\r\n<html lang=\"en\">\r\n<head>\r\n<meta charset=\"utf-8\">\r\n\
<title>404 Not Found</title>\r\n</head>\r\n<body>\r\n\r\n";

	write(fd,html, sizeof(html));
	}
//normal
	else{
	sprintf(head_content,"%s 200 OK\r\nServer: test\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\
Connection: %s\r\nKeep-alive: timeout=20\r\nAccept-Ranges: bytes\r\n\r\n", \
	http_request["Version"].c_str(), extensions[type].c_str(), head-temp,http_request["Connection"].c_str());

	write(fd,head_content,strlen(head_content));

//CGI----------------------------------------------------------------------------------

	if(strstr(http_request["Path"].c_str(),"cgi")) {


		char new_buffer[20];
		read(file_fd, new_buffer, strlen(new_buffer));
		string head_line = new_buffer;

		string shell_path;
		if(strstr(exe_path.c_str(), "php")) shell_path="/usr/bin/php";
		else shell_path="/bin/sh";


		//build pipe
		if (signal(SIGPIPE, SIG_IGN)== SIG_ERR) printf("%s","signal error");
		if (pipe(fd1) < 0 || pipe(fd2) < 0) printf("%s", "pipe error");
		//env1
		string request_m = "REQUEST_METHOD=" + http_request["Type"];
		char *cstr1 = new char[request_m.length() + 1];
		strcpy(cstr1, request_m.c_str());	
		//env2
		string request_u = "REQUEST_URL=" + http_request["Path"];
		char *cstr2 = new char[request_u.length() + 1];
		strcpy(cstr2, request_u.c_str());
		//env3 post
		string cont_len = "CONTENT_LENGTH=" + http_request["Content-Length"];
		char *cstr3 = new char[cont_len.length() + 1];
		strcpy(cstr3, cont_len.c_str());
		//env4 post
		string query_typ = "CONTENT_TYPE=" + http_request["Content-Type"];
		char *cstr4 = new char[query_typ.length() + 1];
		strcpy(cstr4, query_typ.c_str());
		//env5
		string script = "SCRIPT_NAME=" + origin;
		char *cstr5 = new char[script.length() + 1];
		strcpy(cstr5, script.c_str());
		//env6
		string query_str = "QUERY_STRING=" + query;
		char *cstr6 = new char[query_str.length() + 1];
		strcpy(cstr6, query_str.c_str());
		//env7
		string gateway = "GATEWAY_INTERFACE=CGI/1.1";
		char *cstr7 = new char[gateway.length() + 1];
		strcpy(cstr7, gateway.c_str());
		//env8
		string remote_addr = "REMOTE_ADDR=" + http_request["Host"]; //bug
		char *cstr8 = new char[remote_addr.length() + 1];
		strcpy(cstr8, remote_addr.c_str());
		//env9
		string remote = "REMOTE_PORT=" + port;
		char *cstr9 = new char[remote.length() + 1];
		strcpy(cstr9, remote.c_str());
		//env10
		string envpath = "PATH=/bin:/usr/bin:/usr/local/bin";
		char *cstr10 = new char[envpath.length() + 1];
		strcpy(cstr10, envpath.c_str());
		//env11 post
		string req_line = http_request["POST_request"];
		char *cstr11 = new char[req_line.length() + 1];
		strcpy(cstr11, req_line.c_str());

		char* envp_get[]={cstr1,cstr2,cstr5,cstr6,cstr7,cstr8,cstr9,cstr10,NULL};
		char* envp_post[]={cstr1,cstr2,cstr3,cstr4,cstr5,cstr6,cstr7,cstr8,cstr9,cstr10,cstr11,NULL};


		char *cstr = new char[exe_path.length() + 1];
		strcpy(cstr, exe_path.c_str());

		string sh;
		if (strstr(head_line.c_str(), "php")) sh="php";
		else sh="sh";
		char *shell = new char[sh.length() + 1];
		strcpy(shell, sh.c_str());
		char* arg[]={shell, cstr, NULL};
		//arg={"sh/php", exe_path, NULL}



		if((pid= fork()) < 0) {
			perror("fork");
			return;
		} 
		
		else if(pid == 0) {

			if (fd2[1] != STDOUT_FILENO) {
				dup2(fd2[1], STDOUT_FILENO);
				//close(fd2[1]);
			}

			if(http_request["Type"]=="POST"){
				if(execve(shell_path.c_str(), arg,envp_post)) perror("execve");} 
			else{
				if(execve(shell_path.c_str(), arg,envp_get)) perror("execve");} 
			//execve("/usr/bin/php", "/home/shnegl/cgi/printenv_php.txt", *env[]) shell_path.c_str()


		}
		else {
			//close(fd1[0]);
			close(fd2[1]);
			
			ssize_t j=1;

			while(j!=0){

			char env_output[32]={0};
			j = read(fd2[0], env_output, sizeof(env_output));
			//cout << env_output<< endl;
			write(fd, env_output, sizeof(env_output));
			}

		
		}

			if(http_request["Type"]=="POST")
				write(fd,http_request["POST_request"].c_str(),strlen(http_request["POST_request"].c_str()));

			sleep(1);
		return;
	}
//CGI---------------------------------------------------------------------


		while ((ret=read(file_fd, buffer, BUFSIZE))>0) {
			buffer[BUFSIZE]={0};
			write(fd,buffer,ret);
		}

	}

	printf("disconnected from %s:%d\n",inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
	return;
}


int main(int argc, char *argv[]) {
	pid_t pid;
	int fd, pfd; 
	int val;

	struct sockaddr_in sin, psin;

	if(argc< 2) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return(-1);
	}
	signal(SIGCHLD, SIG_IGN);

	//1.socket
	if((fd= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		return(-1);
	}
	val= 1;
	if(setsockopt(fd,SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		perror("setsockopt");
		return(-1);
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family= AF_INET;

	//port input = argv[1]
	sin.sin_port= htons(atoi(argv[1]));


	//2.bind()
	if(bind(fd, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
		perror("bind");
		return(-1);
	}

	//3.listen
	if(listen(fd, SOMAXCONN) < 0) {
		perror("listen");
		return(-1);
	}
	char* arg_path = argv[2];
	char* arg_port = argv[1];

	while(1) {
		val= sizeof(psin);
		bzero(&psin, sizeof(psin));
		if((pfd=accept(fd, (struct sockaddr*) &psin, (socklen_t*) &val))<0) { //4.accept
			perror("accept");
			return(-1);
		}

		if((pid= fork()) < 0) {
			perror("fork");
			return(-1);
		} 
		else if(pid== 0) {/* child */
			close(fd);
			serv_client(pfd, &psin, arg_port, arg_path);
			exit(0);
		}/* parent */

		close(pfd);
	}
}
