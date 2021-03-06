#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <vector>
#include <fstream>
//#include <ctime>
//#include <iomanip>
//#include <iostream>
#include <streambuf>
#include "mongoose.h"

using namespace std;

string port;
string host;
string dir;
struct mg_server *server1, *server2;

static int request_handler(struct mg_connection *conn) {

	string s = conn->uri;
	string headers;
	for(int i = 0; i != conn->num_headers; ++i){
		headers += conn->http_headers[i].name;
		headers += "=";
		headers += conn->http_headers[i].value;
		headers += "\r\n";
	}
	//string query = conn->query_string;
	//ofstream of("/home/osboxes/log.txt", std::ofstream::out | std::ofstream::app);
	ofstream of("/home/box/log.txt", std::ofstream::out | std::ofstream::app);
	//if(!query.empty())
	//	of << query << "\r\n";
	//if(!s.empty())
	of << headers << "\r\n";
	of << s << "\r\n"<< "\r\n";
	of.close();

	string fname;
	if(s == "/"){
		fname = dir + "/index.html";
	}else{
		fname = dir + s;
		string::iterator it = fname.end();
		--it;
		if((*it) == '/'){
			fname += "index.html";
		}
	}

	if (std::ifstream(fname.c_str())){
		std::ifstream t(fname.c_str());
		std::string data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		//mg_send_status(conn, 200);
		//conn->status_code = 200;
		//mg_send_header(conn, "Content-Type", "text/html");
		//mg_printf_data(conn, "%s", data.c_str());

//		string uri = "http://" + host + ":" + port;
//		uri += conn->uri;
//		std::time_t _time = std::time(NULL);
//		string date, dayofWeek, day, month, year, time, zone;
//		stringstream ss;
//		ss << std::put_time(std::gmtime(&_time), "%a, %d %b %Y %H:%M:%S %Z");
//		ss >> dayofWeek;
//		ss >> day;
//		ss >> month;
//		ss >> year;
//		ss >> time;
//		ss >> zone;
//		date = dayofWeek + " " + day + " " + month + " " + year + " " + time + " " + zone;
//		mg_printf(conn, "HTTP/1.0 200 OK\r\n"
//				  "Date: %s\r\n"
//				  "Pragma: no-cache\r\n"
//				  "Location: %s\r\n"
//				  "Server: CERN/3.0\r\n"
//				  "Allow: GET, HEAD\r\n"
//				  "Expires: %s\r\n"
//				  "Last-Modified: %s\r\n"
//					"Content-Length: %d\r\n"
//					"Content-Type: text/html\r\n\r\n%s",
//					date.c_str(), uri.c_str(), date.c_str(), date.c_str(), (int) data.size(), data.c_str());
		mg_printf(conn, "HTTP/1.0 200 OK\r\n"
					"Content-Length: %d\r\n"
					"Content-Type: text/html\r\n\r\n%s",
					(int) data.size(), data.c_str());


	}else{
//		mg_send_status(conn, 404);
//		conn->status_code = 404;
//		mg_send_header(conn, "Content-Type", "text/html");
//		mg_printf_data(conn, "This is a reply from server instance # %s\r\n", (char *) conn->server_param);
		//mg_printf(conn, "HTTP/1.0 404 Not Found\r\n");
		mg_printf(conn, "HTTP/1.0 404 Not Found\r\n"
						"Content-Length: %d\r\n"
						"Content-Type: text/html\r\n\r\n%s",
						9, "Not Found");
	}

	return 0;
}

static void *serve(void *server) {
	for (;;) mg_poll_server((struct mg_server *) server, 1000);
	return NULL;
}

int main(int argc, char *argv[])
{
	int opt = 0;


	while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
		switch (opt) {
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'd':
				dir = optarg;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	printf("host=%s; port=%s; dir=%s; optind=%d\n",
		   host.c_str(), port.c_str(), dir.c_str(), optind);


	//ofstream of("/home/osboxes/params.txt", std::ofstream::out);
	ofstream of("/home/box/params.txt", std::ofstream::out);
	of << host << "\r\n";
	of << port << "\r\n";
	of << dir << "\r\n";
	of.close();

	//	if (optind >= argc) {
	//		fprintf(stderr, "Expected argument after options\n");
	//		exit(EXIT_FAILURE);
	//	}

	//	printf("name argument = %s\n", argv[optind]);

	// создаем потомка
		int pid = fork();

		if (pid == -1) // если не удалось запустить потомка
		{
			// выведем на экран ошибку и её описание
			printf("Error: Start Daemon failed (%s)\n", strerror(errno));

			return -1;
		}
		else if (!pid) // если это потомок
		{
			// данный код уже выполняется в процессе потомка
			// разрешаем выставлять все биты прав на создаваемые файлы,
			// иначе у нас могут быть проблемы с правами доступа
			umask(0);

			// создаём новый сеанс, чтобы не зависеть от родителя
			setsid();

			server1 = mg_create_server((void *) "1");
			server2 = mg_create_server((void *) "2");

			mg_add_uri_handler(server1, "/", request_handler);
			mg_add_uri_handler(server2, "/", request_handler);

			// Make both server1 and server2 listen on the same socket
			string ip = host + ":" + port;
			mg_set_option(server1, "listening_port", ip.c_str());
			mg_set_option(server1, "document_root", dir.c_str());
			mg_set_listening_socket(server2, mg_get_listening_socket(server1));
			// server1 goes to separate thread, server 2 runs in main thread.
			// IMPORTANT: NEVER LET DIFFERENT THREADS HANDLE THE SAME SERVER.
			mg_start_thread(serve, server1);
			mg_start_thread(serve, server2);
			getchar();

			return 0;
		}
		else // если это родитель
		{
			// завершим процес, т.к. основную свою задачу (запуск демона) мы выполнили
			return 0;
		}


	return 0;
}
