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
using namespace std;

/*------------------------------------------------*/
#define MAXBUF 1024 // max buffer size
string fileName = "requests.txt"; // file containing requests
static string portNumber = "8005"; // default port number
string server_ip = "127.0.0.1"; //default host name
/*------------------------------------------------*/
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

/*
*send request : open a socket , connect to the server then send the request 
*receive response until read bytes = -1. 
*@attr port_num : port number to connect with it.
*@attr host_name : host name of the socket.
*@attr request : request to be sent to the server.
*@return : string containing the response of the server.
*/
string send_request(string port_num, string host_name, char* request) {
	// create socket
	/*----attributes----*/
	int sockfd, bytes_read;
	struct sockaddr_in dest; // sockaddr_in is a structure containing an internet address
	char buffer[MAXBUF];
	struct hostent *server;
	string response;

	// create a socket
	// socket(int domain, int type, int protocol)
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		PANIC("Socket");

	/*---Initialize server address/port struct---*/
	if (host_name != "") {
		server = gethostbyname(host_name.c_str());
		// if server is null so there is no host with this name
		if (server == NULL) {
			fprintf(stderr, "ERROR, no such host\n");
			exit(0);
		}
		// clear address structure
		bzero((char *) &dest, sizeof(dest));
		// set address domain of the socket
		dest.sin_family = AF_INET;
		// automatically be filled with current host's IP address
		bcopy((char *) server->h_addr,
		(char *)&dest.sin_addr.s_addr,
		server->h_length);
	} else
	{
		// clear address structure
				bzero((char *) &dest, sizeof(dest));
				// set address domain of the socket
				dest.sin_family = AF_INET;
				// automatically be filled with current host's IP address
				dest.sin_addr.s_addr = inet_addr(server_ip.c_str());
			}
		// convert short integer value for port must be converted into network byte order
	dest.sin_port = htons(atoi(port_num.c_str()));
	/*---Connect to server---*/
	if (connect(sockfd, (struct sockaddr*) &dest, sizeof(dest)) != 0)
		PANIC("Connect");
	/*------Send Request to Server--------*/
	sprintf(buffer, request);
	send(sockfd, buffer, strlen(buffer), 0);

	/*----------While there's data, read and append it to the response ---*/
	do {
		bzero(buffer, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if (bytes_read > 0) {
			/*-----if read bytes less than buffer size so copy the real data to the response-----*/
			if (bytes_read < BUFSIZ) {
				for (int i = 0; i < bytes_read; i++) {
					response += buffer[i];
				}
			}
		}
	} while (bytes_read > 0);
	/*---Clean up---*/
	close(sockfd);
	return response;
}

int main(int argc, char *argv[]) {

	/*-----------initialize variables---------------*/
	string fileRead;
	string response;
	string file;
	string portNum;
	string line;
	string host_name;
	int TempNumOne = fileName.size();
	char Filename[100];
	for (int a = 0; a <= TempNumOne; a++) {
		Filename[a] = fileName[a];
	}
	ifstream myfile(Filename);

	/*------------------------------------------*/
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	/*--------set port and server ip------------*/
	server_ip = argv[1];
	portNumber = argv[2];

	/*-----------------------------------------------------------*/
	/*---------Read Requests File-----------------*/
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
			/*--------set request attributes--------------*/
			if (vec.size() == 4) {
				request = request.substr(0, request.length() - vec[3].length());
				portNum = vec[3];
				host_name = vec[2];
			} else if (vec.size() == 3) {
				portNum = portNumber;
				host_name = vec[2];
			} else {
				portNum = portNumber;
				host_name = "";
			}
			/*----------get file requested---------------------*/
			string fileName = vec[1].substr(1, vec[1].length());
			/*----------convert string to char*------------------*/
			char* r = (char *) alloca(request.size() + 1);
			memcpy(r, request.c_str(), request.size() + 1);

			/*---------------get server response------------------------*/
			response = send_request(portNum, host_name, r);
			cout << "Request : " << r << endl;
			cout << "Response : " << response << endl;

			if (response == "HTTP/1.0 404 Not Found\r\n") {
				PANIC("HTTP/1.0 404 Not Found");
			} else {
				/*-------------take file content--------------------*/
				size_t file_start = response.find("{", 0);
				size_t file_end = response.length() - 1;
				int file_length = file_end - file_start - 1;
				file = response.substr(file_start + 1, file_length);
				/*-----create file with data received-----*/
				std::ofstream outfile((fileName).c_str(), ios::binary);
				outfile.write(file.c_str(), file.length());
				outfile.close();
			}

		}
		myfile.close();
	} else {
		cout << "Unable to open file";
	}
	return 0;
}
