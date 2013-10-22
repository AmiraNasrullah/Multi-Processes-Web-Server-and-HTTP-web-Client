/*
 * Main.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: omnia
 */
/* A simple server in the internet domain using TCP
 The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;
void error(const char *msg) {
	perror(msg);
	exit(1);
}
vector<string> split(string str) {

	string buf; // Have a buffer string
	stringstream ss(str); // Insert the string into a stream

	vector<string> tokens; // Create vector to hold our words

	while (ss >> buf)
		tokens.push_back(buf);
	return tokens;
}
string readTheFile(string fileName) {
	string type = "";
	//check the type if image
	if (fileName.find(".jpg") != std::string::npos) {
		type = "image/jpg\r\n";
	}
	if (fileName.find(".png") != std::string::npos) {
		type = "image/png\r\n";
	}
	if (fileName.find(".gif") != std::string::npos) {
		type = "image/gif\r\n";
	}
	int TempNumOne = fileName.size();
	char Filename[100];
	for (int a = 0; a <= TempNumOne; a++) {
		Filename[a] = fileName[a];
	}
	string line;

	ifstream myfile(Filename);
	string fileRead;
	if (type == "") {
		//read text and html files
		if (myfile.is_open()) {
			fileRead += ("HTTP/1.0 200 OK \r\n");
			fileRead += "{";
			while (myfile.good()) {
				//line
				getline(myfile, line);
				fileRead += (line);
				fileRead += "\n";
			}
			fileRead += "}";
			myfile.close();
		}

		else {
			//cout << "Unable to open file";
			fileRead += ("HTTP/1.0 404 Not Found\r\n");
		}
	} else {
		if (myfile.is_open()) {
			long size = 0;
			fileRead += ("HTTP/1.0 200 OK \r\n");
			fileRead += ("Content-Type : " + type + "\r\n");

			//image file .. read it binary
			std::ifstream fin(Filename, std::ios::in | std::ios::binary);
			std::ostringstream oss;
			oss << fin.rdbuf();
			std::string data(oss.str());
			size = fin.tellg();
			fileRead += ("Content-Length: ");
			//casting size to string
			std::string number;
			std::stringstream strstream;
			strstream << size;
			strstream >> number;
			fileRead += number;
			fileRead += "\n";
			fileRead += "{";
			fileRead += oss.str();

			fileRead += "}";
		} else {
			fileRead += ("HTTP/1.0 404 Not Found\r\n");
		}

	}
	return fileRead;
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd; //file descriptors one for the socket system call and another for accept system call
	int portno; //stores the port number on which the server accepts connections.
	socklen_t clilen; // stores the size of the address of the client
	char buffer[1024000]; //The server reads characters from the socket connection into this buffer.
	struct sockaddr_in serv_addr, cli_addr; //a structure containing an internet address
	int n; //. it contains the number of characters read or written.
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	//creates a new socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr)); // sets all values in a buffer to zero
	portno = atoi(argv[1]); //cast the portno from the arguments
	serv_addr.sin_family = AF_INET; // a code for the address family
	serv_addr.sin_addr.s_addr = INADDR_ANY; //This field contains the IP address of the host
	serv_addr.sin_port = htons(portno); //the port number
	/**
	 * The bind() system call binds a socket to an address,
	 * in this case the address of the current host and port number on which the server will run.
	 *  It takes three arguments, the socket file descriptor, the address to which is bound, and the size of the address to which it is bound.
	 */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	while (true) {
		/**The listen system call allows the process to listen on the socket for connections. The first argument is the socket file descriptor,
		 * and the second is the size of the backlog queue, i.e., the number of connections that can be waiting while the process is handling a particular connection
		 *
		 */
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		/**
		 * The accept() system call causes the process to block until a client connects to the server.
		 * Thus, it wakes up the process when a connection from a client has been successfully established. It returns a new file descriptor
		 */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		pid_t PID = fork();
		if (PID == 0) {
			//create the process
			bzero(buffer, 1024000);
			/**
			 *  read() will block until there is something for it to read in the socket,
			 *   i.e. after the client has executed a write().
			 */
			n = read(newsockfd, buffer, 1023999);
			if (n < 0)
				error("ERROR reading from socket");
			printf("Here is the message: %s\n", buffer);
			string buf = buffer;
			vector<string> dataOfTheMessage = split(buf); //[GET , fname , HTTP1.0 ..]
			if (dataOfTheMessage[0] == "GET") {
				string fname = dataOfTheMessage[1];
				fname = fname.substr(1, fname.size() - 1);
				string dataReturned = readTheFile(fname);
//				cout << dataReturned <<endl;
				n = write(newsockfd, dataReturned.c_str(),
						dataReturned.length());
			} else {
				n = write(newsockfd, "Error not well formed", 50);
			}
			if (n < 0)
				error("ERROR writing to socket");

			exit(0);
			//exit the process
		} else {
			//the parent closes the socket
			close(newsockfd);

		}

	}
	return 0;
}

