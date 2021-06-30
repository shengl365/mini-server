#ifndef REQUEST_H_
#define REQUEST_H_

#include<string>
#include<map>
#include<vector>
using namespace std;

class Request;

struct Http_request {
	string method;
	string path;
	string raw;
	string body;
};

struct Http_responese {
	string status;
	string data;
};

struct Url {
	string route;
	string query;
};

typedef void (Request::*REQHANDLE)(Http_request q);

class Request {
	public:
		Request();
		Http_responese res;
		void parse_http_head(char*);
		void parse_http_body(string);
		Url parse_http_url(string url);

	private:
		REQHANDLE pfReqHandleRule[3];
		Http_request text;
		void get_handler(Http_request q);
		void post_handler(Http_request q);
		int cgi_handler(string);
		void error_handler(int error_type);
};


#endif
