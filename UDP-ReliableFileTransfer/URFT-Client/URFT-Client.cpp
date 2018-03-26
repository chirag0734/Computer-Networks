#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define CLIENTPORT 10000    // the port users will be connecting to
#define BUFFERLENGTH 512
#define BUFFER_SIZE 480
struct udp_packet {
	u_int32_t seqNumber;
	u_int32_t buf_len;
	char fileName[24];
	char buf[BUFFER_SIZE];
};

int main(int argc, char *argv[]) {

	//Initializing Variable to connect to PROXY
	int sockfd;
	struct sockaddr_in proxyServerDetails; // connector's address information
	struct hostent *hostDetails;
	int numbytes = 0;
	unsigned addr_len;
	char buffer[BUFFERLENGTH];
	int proxyPort;
	FILE *fp = NULL;
	int file_block_length = 0;
	long numBytes = 0;
	struct udp_packet packet;

	//checking arguments from command line for ($ ./UDP-client 127.0.0.1 10000 test_file1)
	if (argc != 4) {
		fprintf(stderr, "usage: talker hostname message\n");
		exit(1);
	} else {
		//connecting to Proxy server on this port
		proxyPort = atoi(argv[2]);
	}

	//Connecting to IP of proxy
	if ((hostDetails = gethostbyname(argv[1])) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	proxyServerDetails.sin_family = AF_INET;     // host byte order
	proxyServerDetails.sin_port = htons(proxyPort); // short, network byte order
	proxyServerDetails.sin_addr = *((struct in_addr *) hostDetails->h_addr);
	memset(&(proxyServerDetails.sin_zero), '\0', 8); // zero the rest of the struct

	int buffsize = 65536; // this number is 65536
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*) &buffsize,
			sizeof(buffsize));

	if ((fp = fopen(argv[3], "r")) == NULL) {
		perror("error opening file");
		exit(1);
	} else {
		fseek(fp, 0, SEEK_END);

		// get the file size
		int Size = ftell(fp);
		printf("File size = %d\n", Size);

		// return the file pointer to begin of file if you want to read it
		rewind(fp);
		printf("File transfer started\n");

		int numOfPackets = Size / BUFFER_SIZE + 1;
		printf("Number Of Packets = %d\n", numOfPackets);
		file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp);

		//strncpy(packet.buf, buffer, file_block_length);
		snprintf(packet.buf, file_block_length, "%s", buffer);
		packet.seqNumber = 0;
		packet.buf_len = file_block_length;

		snprintf(packet.fileName, sizeof(packet.fileName), "%s", argv[3]);
		//strncpy(packet.fileName, argv[3], (size_t)sizeof(argv[3]));

		memset(buffer, 0, BUFFERLENGTH);

		if ((numbytes = sendto(sockfd, &packet, sizeof(packet), 0,
				(struct sockaddr *) &proxyServerDetails,
				sizeof(struct sockaddr))) == -1) {
			perror("sendto");
			exit(1);
		}
	}
	//Implementing TimeOut Here

	fd_set select_fds; /* fd's used by select */
	struct timeval timeout; /* Time value for time out */

	timeout.tv_sec = 0; /* Timeout set for 5 sec + 0 micro sec*/
	timeout.tv_usec = 1000;

	while (numbytes > 0) {

		FD_ZERO(&select_fds); /* Clear out fd's */
		FD_SET(sockfd, &select_fds); /* Set the interesting fd's */
		//printf("inWHILE LOOP\n");
		int timeOutStatus = select(sockfd + 1, &select_fds, NULL, NULL,
				&timeout);

		if (timeOutStatus <= 0) {
			//printf("%d*****", timeOutStatus);
			numbytes = sendto(sockfd, &packet, sizeof(packet), 0,
					(struct sockaddr *) &proxyServerDetails,
					sizeof(struct sockaddr));
			//printf("Data Has been resent--\n");

			timeout.tv_sec = 0;		 //Timeout set for 5 sec + 0 micro sec
			timeout.tv_usec = 1000;

		}

		else {

			addr_len = sizeof(proxyServerDetails);

			if ((numBytes = recvfrom(sockfd, buffer, BUFFERLENGTH, 0,
					(struct sockaddr *) &proxyServerDetails, &addr_len)) > 0) {

				//printf("\n %ld bytes received from server", numBytes);

				if (numbytes == numBytes) {
					file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE,
							fp);

					//printf("file_block_length 2: %d", file_block_length);

					if (file_block_length == 0) {
						numbytes = 0;
						printf("End of file");
					} else {
						//printf("file_block_length 3: %d", file_block_length);

						//strncpy(packet.buf, buffer, file_block_length);
						snprintf(packet.buf, file_block_length, "%s", buffer);
						memset(buffer, 0, BUFFERLENGTH);
						packet.seqNumber++;
						packet.buf_len = file_block_length;
					}
				}

			}

		}

	}

	close(sockfd);

	return 0;
}
