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
#include <vector>
#include <fstream>
#define MAXREQ 50
#define MAXBUF 1024
using namespace std;
string fileName = "requests.txt";
string hostname = "localhost";
string portnum = "59747";
void PANIC(const char *msg) {
	perror(msg);
}
vector<string> split(string str) {

	string buf; // Have a buffer string
	stringstream ss(str); // Insert the string into a stream

	vector<string> tokens; // Create vector to hold our words

	while (ss >> buf)
		tokens.push_back(buf);
	return tokens;
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
	int port_num;
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
			vector<string> vec = split(request);
			//if port number sent remove it from request
			if (vec.size() == 4) {
				request = request.substr(0, request.length() - vec[3].length());
				cout << request << endl;

			}
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
			if (vec.size() >= 3)
				server = gethostbyname(vec[2].c_str());

			if (server == NULL) {
				fprintf(stderr, "ERROR, no such host\n");
				exit(0);
			}
			bzero((char *) &dest, sizeof(dest));
			dest.sin_family = AF_INET;
			bcopy((char *) server->h_addr,
			(char *)&dest.sin_addr.s_addr,
			server->h_length);
			if (vec.size() == 4) {
				dest.sin_port = htons(atoi(vec[3].c_str()));
			} else {
				/*default HTTP Server port */
				dest.sin_port = htons(atoi(portnum.c_str()));
			}

			/*---Connect to server---*/
			if (connect(sockfd, (struct sockaddr*) &dest, sizeof(dest)) != 0)
				PANIC("Connect");
			/*------Send Request to Server--------*/
			sprintf(buffer, r);
			send(sockfd, buffer, strlen(buffer), 0);

			/*---While there's data, read and print it---*/
			bzero(buffer, sizeof(buffer));
			bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
			if (bytes_read > 0)
				printf("%s", buffer);
			if (buffer == "HTTP/1.0 404 Not Found\r\n") {
				PANIC("HTTP/1.0 404 Not Found");
			} else {
				string buf = buffer;
				//create file with data received
				size_t file_start = buf.find("{", 0);
				size_t file_end = buf.length() - 1;
				int file_length = file_end - file_start - 1;
				buf = buf.substr(file_start + 1, file_length);
				string fileName = vec[1].substr(1, vec[1].length());
				/*-------------@TODO-------------*/
				/*------check if file is image handle it--------*/
				/*-----write it to file-----*/
				std::ofstream outfile((fileName).c_str());
				outfile.write(buf.c_str(), buf.length());
				outfile.close();
			}


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
