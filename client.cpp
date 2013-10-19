#include <stdarg.h>
#include <resolv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <string>
#define MAXREQ 50
#define MAXBUF 1024
using namespace std;
string fileName = "requests.txt";
string hostname = "localhost";
string portnum = "8080";
void PANIC(const char *msg) {
	perror(msg);
}
int main(int Count, char *Strings[]) {
	int TempNumOne = fileName.size();
	char Filename[100];
	for (int a = 0; a <= TempNumOne; a++) {
		Filename[a] = fileName[a];
	}
	string line;
	ifstream myfile(Filename);
	string fileRead;
	//open file
	if (myfile.is_open()) {
		//while there is more requests
		while (myfile.good()) {
			//get line
			getline(myfile, line);
			//last line in the file
			if (strcmp(line.c_str(), "\\r\\n") == 0) {
				break;
			}
			//update request format
			//remove "\n\r"
			std::size_t found = line.find_first_of("\\");
			string request = line.substr(0, found);
			char* r = (char *) alloca(request.size() + 1);
			memcpy(r, request.c_str(), request.size() + 1);
			//--------so r is the request--------
			// create socket
			/*----attributes----*/
			int sockfd, bytes_read;
			struct sockaddr_in dest;
			char buffer[MAXBUF];
			struct hostent *server;
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				PANIC("Socket");
			/*---Initialize server address/port struct---*/
			server = gethostbyname(hostname.c_str());
			if (server == NULL) {
				fprintf(stderr, "ERROR, no such host\n");
				exit(0);
			}
			bzero((char *) &dest, sizeof(dest));
			dest.sin_family = AF_INET;
			bcopy((char *) server->h_addr,
			(char *)&dest.sin_addr.s_addr,
			server->h_length);
			dest.sin_port = htons(54717); /*default HTTP Server port */

			/*---Connect to server---*/
			if (connect(sockfd, (struct sockaddr*) &dest, sizeof(dest)) != 0)
				PANIC("Connect");
			/*------Send Request to Server--------*/
			sprintf(buffer, r);
			send(sockfd, buffer, strlen(buffer), 0);

			/*---While there's data, read and print it---*/
			do {
				bzero(buffer, sizeof(buffer));
				bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
				if (bytes_read > 0)
					printf("%s", buffer);
			} while (bytes_read > 0);

			/*---Clean up---*/
			close(sockfd);
		}
		myfile.close();
	}

	else {
		cout << "Unable to open file";
	}
	return 0;
}
