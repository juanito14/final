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
#include "mongoose.h"

using namespace std;

#define DAEMON_NAME "shamandaemon"

string port;
string host;
string dir;


void process(){
	syslog (LOG_NOTICE, "Writing to my Syslog");
}

static int request_handler(struct mg_connection *conn) {

	string s = conn->uri;
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
		mg_send_status(conn, 200);
		mg_send_header(conn, "Content-Type", "text/html");
		mg_printf_data(conn, "HTTP/1.0 200 OK\r\nThis is a reply from server instance # %s\r\n", (char *) conn->server_param);
	}else{
		mg_send_status(conn, 404);
		mg_send_header(conn, "Content-Type", "text/html");
		mg_printf_data(conn, "HTTP/1.0 404 Not Found\r\nThis is a reply from server instance # %s\r\n", (char *) conn->server_param);
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

	//	if (optind >= argc) {
	//		fprintf(stderr, "Expected argument after options\n");
	//		exit(EXIT_FAILURE);
	//	}

	//	printf("name argument = %s\n", argv[optind]);

	//Set our Logging Mask and open the Log
	setlogmask(LOG_UPTO(LOG_NOTICE));
	openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

	syslog(LOG_INFO, "Entering Daemon");

	pid_t pid, sid;

	//Fork the Parent Process
	pid = fork();

	if (pid < 0) { exit(EXIT_FAILURE); }

	//We got a good pid, Close the Parent Process
	if (pid > 0) { exit(EXIT_SUCCESS); }

	//Change File Mask
	umask(0);

	//Create a new Signature Id for our child
	sid = setsid();
	if (sid < 0) { exit(EXIT_FAILURE); }

	//Change Directory
	//If we cant find the directory we exit with failure.
	if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }

	//Close Standard File Descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	//----------------
	//Main Process
	//----------------
	{
		//process();    //Run our Process
		struct mg_server *server1, *server2;

		server1 = mg_create_server((void *) "1");
		server2 = mg_create_server((void *) "2");

		mg_add_uri_handler(server1, "/", request_handler);
		mg_add_uri_handler(server2, "/", request_handler);

		// Make both server1 and server2 listen on the same socket
		mg_set_option(server1, "listening_port", port.c_str());
		mg_set_option(server1, "document_root", dir.c_str());
		mg_set_listening_socket(server2, mg_get_listening_socket(server1));

		// server1 goes to separate thread, server 2 runs in main thread.
		// IMPORTANT: NEVER LET DIFFERENT THREADS HANDLE THE SAME SERVER.
		mg_start_thread(serve, server1);
		mg_start_thread(serve, server2);

		//sleep(60);    //Sleep for 60 seconds
		getchar();
	}

	//Close the log
	closelog ();

	return 0;
}
