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
		if (myfile.is_open()) {
			fileRead += ("HTTP/1.0 200 OK \r\n");
			while (myfile.good()) {
				//line
				getline(myfile, line);

				fileRead += (line);

			}

			myfile.close();
		}

		else {
			//cout << "Unable to open file";
			fileRead += ("HTTP/1.0 404 Not Found\r\n");
		}
	} else {
		//image file .. read it binary
		ifstream::pos_type size;
		char * memblock;
		ifstream file(Filename, ios::in | ios::binary | ios::ate);
		if (file.is_open()) {
			// long begin,end;
			fileRead += ("HTTP/1.0 200 OK \r\n");
			fileRead += ("Content-Type : " + type + "\r\n");
			size = file.tellg();
			//begin  = size;
			memblock = new char[size];
			file.seekg(0, ios::beg);
			// end = myfile.tellg();
			file.read(memblock, size);
			file.close();

			// cout << "the complete file content is in memory";
			//  cout<< memblock <<endl;
			//  cout <<size;
			fileRead += ("Content-Length: ");
			std::string number;
			std::stringstream strstream;
			strstream << size;
			strstream >> number;
			fileRead += number;
			fileRead += "\r\n \r\n";
			fileRead += memblock;
			delete[] memblock;
		} else {
			//cout << "Unable to open file";
			fileRead += ("HTTP/1.0 404 Not Found\r\n");
		}

	}
	return fileRead;
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	while (true) {
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		pid_t PID = fork();
		if (PID == 0) {
			bzero(buffer, 1024);
			n = read(newsockfd, buffer, 1023);
			if (n < 0)
				error("ERROR reading from socket");
			printf("Here is the message: %s\n", buffer);
			string buf = buffer;
			vector<string> dataOfTheMessage = split(buf); //[GET , fname , HTTP1.0 ..]
			if (dataOfTheMessage.size() == 3 && dataOfTheMessage[0] == "GET"
					&& dataOfTheMessage[2] == "HTTP/1.0") {
				string fname = dataOfTheMessage[1];
				fname = fname.substr(1, fname.size() - 1);
				string dataReturned = readTheFile(fname);
				n = write(newsockfd, dataReturned.c_str(), 1024);
			} else {
				n = write(newsockfd, "Error not well formed", 50);
			}
			if (n < 0)
				error("ERROR writing to socket");

			exit(0);
		} else {
			close(newsockfd);
		}

	}
	return 0;
}

