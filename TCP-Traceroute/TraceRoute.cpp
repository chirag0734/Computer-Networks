//============================================================================
// Name        : TraceRoute.cpp
// Author      : Pranay Reddy, Chirag Jain
// Version     :
// Copyright   : Your copyright notice
// Description : TraceRoute in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

void routerResponse(int ICMPSock, int ttl, struct timeval *t1);

#define PACKET_SIZE 1000

int main(int argc, char *argv[]) {

	char dataGram[PACKET_SIZE];
	struct sockaddr_in serverDetails, ICMPServerDetails;

	int sd, ICMPSock;
	char buffer[PACKET_SIZE];
	int numBytes;
	unsigned addrLength;

	struct ip *IPH;
	struct tcphdr *TCPH;
	int tcp_pkt_len = sizeof(struct ip) + sizeof(struct tcphdr);
	int ip_pkt_len = sizeof(struct ip) + sizeof(struct tcphdr);

	struct timeval timeout, t1;
	struct timezone tz;

	memset(dataGram, 0, PACKET_SIZE);

	if ((sd = (socket(AF_INET, SOCK_RAW, IPPROTO_TCP))) < 0) {
		cout << "RAW SOCKET Connection Not Formed" << sd;
	} else {
		//cout << "RAW SOCKET Connection Formed";

		if ((ICMPSock = (socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))) < 0)
			cout << "ICMP Connection Not Formed" << ICMPSock;

	}

	serverDetails.sin_family = AF_INET;
	serverDetails.sin_port = atoi(argv[3]);
	serverDetails.sin_addr.s_addr = inet_addr(argv[2]);
	//serverDetails.sin_addr.in_addr=inet_addr("74.125.157.104") 74.125.196.100;74.125.21.101;209.85.254.107;96.34.73.193

	/* IP HEADER */
	IPH = (struct ip*) dataGram;

	IPH->ip_hl = 5; // 5*4 = 20
	IPH->ip_v = 4; // IPv4
	IPH->ip_tos = 0;
	IPH->ip_len = ip_pkt_len; // we want to send an empty TCP packet so only headers size
	IPH->ip_id = htons(rand());
	IPH->ip_off = htons(IP_DF);
	IPH->ip_p = IPPROTO_TCP;
	IPH->ip_sum = 0;
	IPH->ip_src.s_addr = inet_addr(argv[1]);
	IPH->ip_dst.s_addr = inet_addr(argv[2]);//74.125.21.101 74.125.196.100;209.85.254.107

	/* TCP HEADER */
	TCPH = (struct tcphdr*) (dataGram + sizeof(struct ip));

	TCPH->source = htons(1024 + rand() % (65535 - 1024));
	TCPH->dest = atoi(argv[3]);
	//TCPH->seq = htonl(rand());
	TCPH->ack_seq = htonl(rand());
	TCPH->syn = 1;
	TCPH->ack = 0;
	TCPH->psh = 0;
	TCPH->doff = 5;
	TCPH->window = htons(1460);
	TCPH->check = 0;
	TCPH->urg_ptr = 0;

	{
		int oneValue = 1;
		const int *val = &oneValue;
		if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(oneValue)) < 0)
			printf("Warning: Cannot set HDRINCL!\n");
		/*else
			printf("Set Socket Executed\n");*/
	}

	{
		int two = 1;
		const int *val = &two;
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		int ret = setsockopt(ICMPSock, SOL_SOCKET, SO_RCVTIMEO, &timeout,
				sizeof(struct timeval));
		if (ret == -1) {
			perror("setsockopt in main receive:");

		}
	}

	for (int ttl = 1; ttl < 31; ttl++) {

		IPH->ip_ttl = ttl;
		cout << ttl << "  ";

		for (int i = 1; i < 4; i++){
			(void) gettimeofday(&t1, &tz);

			if (sendto(sd, dataGram, IPH->ip_len, 0,
					(struct sockaddr *) &serverDetails, sizeof(serverDetails))
					< 0) {
				perror("Cannot sent DataGram\n");
			}else{

			routerResponse(ICMPSock, ttl, &t1);

			}
		}
		cout << "\n";
	}

	return 0;
}

void routerResponse(int ICMPSock, int ttl, struct timeval *t1) {

	unsigned addrLength;
	struct sockaddr_in ICMPServerDetails;
	ICMPServerDetails.sin_family = AF_INET;
	addrLength = sizeof(ICMPServerDetails);
	int numBytes;
	char buffer[PACKET_SIZE];
	struct ip *iphdrResponse = NULL;
	struct icmp *icmpResponse = NULL;
	unsigned short iphdrlen;
	char *str1, *str2;
	struct timeval t2;
	struct timezone tz;
	double RTT;

	(void) gettimeofday(&t2, &tz);
	if ((numBytes = recvfrom(ICMPSock, buffer, sizeof(buffer), 0,
			(struct sockaddr *) &ICMPServerDetails, &addrLength)) < 0) {

		cout << "*\t";

	} else {
		//printf("Received BYTES from Routers %d\n",numBytes);

		iphdrResponse = (struct ip*) buffer;

		iphdrlen = iphdrResponse->ip_hl * 4;

		icmpResponse = (struct icmp*) (buffer + iphdrlen);

		if ((iphdrResponse->ip_p == IPPROTO_ICMP)
				&& (icmpResponse->icmp_type == ICMP_TIME_EXCEEDED)) {
			str1 = (char *) malloc(16 * sizeof(char));

			inet_ntop(AF_INET, &(iphdrResponse->ip_src.s_addr), str1, 16);

			printf("%s\t", str1);

			RTT = (double)(t2.tv_sec - t1->tv_sec) * 1000000.0
					+ (double)(t2.tv_usec - t1->tv_usec);
			printf("%.3f ms\t", RTT);

		}else if((iphdrResponse->ip_p )
				&& (icmpResponse->icmp_code == ICMP_UNREACH_PORT)){
			str2 = (char *) malloc(16 * sizeof(char));

			inet_ntop(AF_INET, &(iphdrResponse->ip_src.s_addr), str2, 16);

			printf("%s\t", str2);

			RTT = (double)(t2.tv_sec - t1->tv_sec) * 1000.0
					+ (double)(t2.tv_usec - t1->tv_usec);
			printf("%.3f ms\t", RTT);
			exit(1);
		}

	}

}
