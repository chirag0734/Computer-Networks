//============================================================================
// Name        : urft-server.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <exception>
#include <stdio.h>
#include <fstream>

#define BUFFERLENGTH 1000
#define BUFFER_SIZE 480

struct udp_packet {
	u_int32_t seqNumber;
	u_int32_t buf_len;
	char fName[24];
	char buf[BUFFER_SIZE];
};

using namespace std;

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in serverDetails;    // my address information
	struct sockaddr_in proxyDetails; // connector's address information
	int numbytes;
	unsigned addr_len;
	char *buf;
	buf = (char *) calloc(BUFFERLENGTH, sizeof(char));

	char fileName[24];

	struct hostent *hostDetails;
	int serverPort;
	struct udp_packet packetReceive;
	u_int32_t seqNO = 1;

	// Checking command line arguments and running server on the PORT

	if (argc != 2) {
		fprintf(stderr, "InSufficient Command Line Argument to Root Server");
		exit(1);
	} else {

		serverPort = atoi(argv[1]);

	}

	hostDetails = gethostbyname("127.0.0.1");

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		cout << "socket created";

	} else {
		perror("sock created....");
	}

	memset(&serverDetails, 0, sizeof(serverDetails));
	serverDetails.sin_family = PF_INET;         // host byte order
	serverDetails.sin_port = htons(serverPort);     // short, network byte order
	serverDetails.sin_addr = *((struct in_addr *) hostDetails->h_addr); // automatically fill with my IP

	int buffsize = 65536; // this number is 65536
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buffsize,
			sizeof(buffsize));

	const int bindValue = bind(sockfd, (struct sockaddr *) &serverDetails,
			sizeof(serverDetails));

	if (bindValue < 0) {
		cout << "Bind Failed with Proxy";
	}

	ofstream fp;

	//Regular check for the requests from Proxy
	addr_len = sizeof(struct sockaddr);
	for (;;) {

		memset(buf, 0, BUFFERLENGTH);
		memset(fileName, 0, 24);

		//For receive File Data
		if ((numbytes = recvfrom(sockfd, &packetReceive, sizeof(packetReceive),
				0, (struct sockaddr *) &proxyDetails, &addr_len)) == -1) {

			cout << "ERROR receive created" << endl;
			exit(1);
		} else {
			strncpy(fileName, packetReceive.fName, 24);

			fp.open(fileName, ios::out | ios::app);

			if (packetReceive.seqNumber != seqNO) {

				cout << "Receiving Packet: " << packetReceive.seqNumber << endl;

				fp << strncpy(buf, packetReceive.buf, packetReceive.buf_len);

				sendto(sockfd, buf, numbytes, 0,
						(struct sockaddr *) &proxyDetails, addr_len);
				seqNO = packetReceive.seqNumber;
			} else {
				sendto(sockfd, buf, numbytes, 0,
						(struct sockaddr *) &proxyDetails, addr_len);
			}
		}
		fp.close();
	}
	close(sockfd);
	return 0;
}
