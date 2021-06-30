#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "request.h"

Request::Request() {

	for(int i = 0 ; i < 3 ; i++)
		pfReqHandleRule[i] = 0;

	pfReqHandleRule[0] = &Request::get_handler;
	pfReqHandleRule[1] = &Request::post_handler;
}

void Request::parse_http_head(char* req) {

	char* head;
	vector<string> tmp = {};
	unordered_map<string, int> func_map = {
		{"GET", 0},
		{"POST", 1}
	};

	strtok(req, "\n");
	head = strtok(req, " ");

	while(head != NULL) {
		tmp.push_back(string(head)); 
		head = strtok(NULL, " ");
	}
	text.method = string(tmp[0]);
	text.path = string(tmp[1]);
	text.raw = string(req);

	(this->*pfReqHandleRule[func_map[text.method]])(text);

}

//GET
Url Request::parse_http_url(string url) {
	
	Url req_url;
	size_t pos = url.find("?");


	if(pos != string::npos){
		string path, query;
		path = url.substr(0, pos);
		query = url.substr(pos+1);

		req_url.route = path;
		req_url.query = query;
	} else {
		req_url.route = url;
		req_url.query = "";
	}
	
	return req_url;
}	

void Request::get_handler(Http_request text) {

	Url url = parse_http_url(text.path);

	setenv("QUERY", url.query.c_str(), 1);

    	if((access(url.route.c_str(), F_OK)) == -1) {
        	this->error_handler(404);
		return;
    	}

	if((url.route) == "/") {
		this->error_handler(403);
		return;
    	}

	int len = url.route.length();
	if(len> 3)
		if(this->cgi_handler(url.route) < 1) {
			error_handler(502);
			return;
		}

	res.status = "200 OK";


}

//POST
void Request::parse_http_body(string req) {

	string str;
	istringstream istr; 
	istr.str(string(req));

	while (getline(istr, str)) {
        	if (str == string(1,'\r')){
			while (getline(istr, str)) {
				text.body.append(str);
				text.body.append("\n");
        		}
		}
	}
}

void Request::post_handler(Http_request text){

	parse_http_body(text.raw);

	setenv("BODY", text.body.c_str(), 1);

    	if((access(text.path.c_str(), F_OK)) == -1) {
        	this->error_handler(404);
		return;
    	}

	if((text.path) == "/") {
		error_handler(403);
		return;
    	}

	int len = text.path.length();
	if(len> 3)
		if(this->cgi_handler(text.path) < 1){
			error_handler(502);
			return;
		}

	res.status = "200 OK";



}

int Request::cgi_handler(string route){

    	char buffer[1024] = { 0 };
    	int len; 
    	int pfd[2];
    	int status;
    	pid_t pid;

	if (pipe(pfd)<0) {
		cout << "err";
	        return -1;
	}

    	pid = fork();

   	if (pid<0) {
        	return 0;
    	} else 
	if (pid==0) {

        	dup2(pfd[1], STDOUT_FILENO);
        	close(pfd[0]);
        	
		if(execlp(route.c_str(), route.c_str(), NULL) < 0) {
			error_handler(404);
		}

        	exit(0);
    	} else {
        	close(pfd[1]);
        	waitpid((pid_t)pid, &status, 0);
        	while((len=read(pfd[0], buffer, 1023))>0) {
           		res.data += string(buffer);
        	}	
		res.data += "\n";
	}
    	return 1;

}

void Request::error_handler(int err_type){

	switch(err_type) {

		case 403:
			res.status = "403 Forbidden";
			break;
		case 404:
			res.status = "404 Not Found";
			break;
		case 502:
			res.status = "502 Bad Gateway";
			break;

	}
		
	
}
