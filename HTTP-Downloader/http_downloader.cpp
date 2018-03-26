//============================================================================
// Name        : http_downloader.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>


using namespace std;

int getBodySize(string responseString);
string getBody(string body);
void *createMultipleConnections(void *args);

struct FileContents {
	int threadNumber;
	int startRange;
	int endRange;
	string filePath;
	string domainName;
	int fileSize;
};

int main(int argc, char *argv[]) {

	//number of connections from command line
	int numberOfConnections=atoi(argv[2]);

	// Request Message from Command Line
	char httprequest[300];
	strcpy(httprequest, argv[1]);

	// converting char into string
	string httpRequest(httprequest);

	int domainfirstIndex=httpRequest.find_first_of("/")+2;

	string truncatedhhtRequest= httpRequest.substr(domainfirstIndex);
	int indexOfFilePath=truncatedhhtRequest.find_first_of("/");

	string domain=truncatedhhtRequest.substr(0,indexOfFilePath);

	string filePath = truncatedhhtRequest.substr(indexOfFilePath);
	int imageIndex=truncatedhhtRequest.find_last_of("/")+1;

	string imageName=truncatedhhtRequest.substr(imageIndex);
	char finalImage[1024];
	strncpy(finalImage, imageName.c_str(), sizeof(finalImage));

	cout << finalImage << "sdsds"<< endl;

	stringstream requestHTTPStringsteam;
	string requestString;
	requestHTTPStringsteam <<"HEAD "<<filePath<<" HTTP/1.1\r\nHOST: "<<domain<<"\r\n\r\n";

	requestString=requestHTTPStringsteam.str();
	//coverting to char which will be used in SEND function
	char domainName[300];

	strcpy(domainName,domain.c_str());


	//Variable for range of each file
	int totalFileSize;
	int range = 0;
	int secondRange = 0;
	int startRange[numberOfConnections], endRange[numberOfConnections];

	//variables to create socket and also establish connection
	int s;
	static const int num_threads = numberOfConnections;
	pthread_t connectionThread[num_threads];
	struct sockaddr_in serverDetails;
	struct hostent *serverHost;
	int port = 80;
	char responseData[3000];

	struct FileContents fileData[num_threads];
	char requestMessage[3000];
	strcpy(requestMessage,requestString.c_str());
	// socket creation of TCP for header getting header
	if ((s = (socket(AF_INET, SOCK_STREAM, 0))) < 0) {
		cout << "Connection Not Formed" << s;
	} else {

		// getting IP address of server
		serverHost = gethostbyname(domainName);
		cout << serverHost->h_addr_list;

		bzero((char *) &serverDetails, sizeof(serverDetails));

		//setting sockaddr_in serverDetails to establish server connection
		serverDetails.sin_family = AF_INET;
		bcopy((char *) serverHost->h_addr, (char *)&serverDetails.sin_addr.s_addr, serverHost->h_length);
		serverDetails.sin_port = htons(port);
		memset(&(serverDetails.sin_zero), '\0', 8);

		//connection to the server with details of sockaddr_in
		if (connect(s, (struct sockaddr *) &serverDetails,
				sizeof(serverDetails)) < 0) {
			cout << "Connection failed";
		} else {
			cout << "Connected to server UGA" << endl;
		}

	}
	// sending request for server
	if (send(s, requestMessage, strlen(requestMessage), 0) < 0) {
		cout << "Failed to get send request";
	} else {
		cout << "Image request sent" << endl;


		recv(s, responseData, 3000, 0);

		// process to get content length from response received from server
		string responseString(responseData);


		totalFileSize = getBodySize(responseString);

		// for getting range of each file --> divide the total file length / number of connections and storing ranges in arrays

		range = totalFileSize / numberOfConnections;
		for (int i = 0, firstRange = 0, j = 1; j <= numberOfConnections;
				i++, j++) {
			firstRange = secondRange + 1;
			secondRange = secondRange + range;

			if (j == numberOfConnections) {
				startRange[j] = firstRange - 1;
				endRange[j] = totalFileSize - 1;
			} else {
				startRange[j] = firstRange - 1;
				endRange[j] = secondRange - 1;
			}

			i = secondRange;

		}

	}
	// closing socket
	close(s);

	// creating threads based on HEADER RESPONSE from sever

	for (int i = 0; i < num_threads; ++i) {

		fileData[i].startRange = startRange[i + 1];
		fileData[i].endRange = endRange[i + 1];
		fileData[i].threadNumber = connectionThread[i];

		fileData[i].threadNumber = i + 1;
		fileData[i].domainName=domain;
		fileData[i].filePath=filePath;
		fileData[i].fileSize = totalFileSize;

		pthread_create(&connectionThread[i], NULL, createMultipleConnections,
				&fileData[i]);

	}

	// calling join so that main method waits complete THREAD execution
	for (int i = 0; i < num_threads; ++i) {
		pthread_join(connectionThread[i], NULL);
	}



	fstream fileToConcatenate, result;

	for (int i = 1; i <= num_threads; i++){

		char filename[8];
		sprintf(filename, "part_%d", i);

		fileToConcatenate.open(filename, ios::in);
		if (fileToConcatenate.is_open()){
			result.open(finalImage, ios::out|ios::app);
			result << fileToConcatenate.rdbuf();
			fileToConcatenate.close();
			result.close();
		}else{
			puts("The file you are trying to concatenate from doesn't exist!Try again!");
		}

		fileToConcatenate.close();
		result.close();
	}



	return 0;
}

int getBodySize(string responseString) {

	int contententLenghtPosition = responseString.find("Content-Length:");
	string contentLengthSubstring = responseString.substr(
			contententLenghtPosition + 16);
	int EOD = contentLengthSubstring.find("\n");
	string bodyLength = responseString.substr(contententLenghtPosition + 16,
			EOD);

	int bodySize = atoi(bodyLength.c_str());

	return bodySize;

}

string getBody(string bodyContent) {
	int headerStartLocation = bodyContent.find("\r\n\r\n");
	string completebody = bodyContent.substr(headerStartLocation + 4);

	return completebody;
}

void *createMultipleConnections(void *args) {
	struct FileContents *fileData = (struct FileContents *) args;

	int start = fileData->startRange;
	int end = fileData->endRange;
	int contentLength = end - start + 1;

	int threadNumber = fileData->threadNumber;
	string domain=fileData->domainName;
	string filepath=fileData->filePath;

	stringstream filestream;
	filestream << "part_" << threadNumber;
	string fileName = filestream.str();
	int s;
	struct sockaddr_in serverDetails;
	struct hostent *serverHost;
	int port = 80;
	char responseData[3000];
	char responseData2[3000];
	char requestMessage[3000];
	stringstream requestStringStream;
	string requestString;

	requestStringStream <<"GET "<<filepath<<" HTTP/1.1\r\nHOST: "<<domain<<"\r\nRange: bytes="<< fileData->startRange
			<< "-" << fileData->endRange << "\r\n\r\n";;

	char domainName[300];
	strcpy(domainName,domain.c_str());
	requestString = requestStringStream.str();
	//converting to char which will be used in SEND function
	strcpy(requestMessage, requestString.c_str());

	// socket creation of TCP for header getting header
	if ((s = (socket(AF_INET, SOCK_STREAM, 0))) < 0) {
		cout << "Connection Not Formed" << s;
	} else {

		// getting IP address of server
		serverHost = gethostbyname(domainName);
		cout << serverHost->h_addr_list;

		bzero((char *) &serverDetails, sizeof(serverDetails));

		//setting sockaddr_in serverDetails to establish server connection
		serverDetails.sin_family = AF_INET;
		bcopy((char *) serverHost->h_addr, (char *)&serverDetails.sin_addr.s_addr, serverHost->h_length);
		serverDetails.sin_port = htons(port);

		//connection to the server with details of sockaddr_in
		if (connect(s, (struct sockaddr *) &serverDetails,
				sizeof(serverDetails)) < 0) {
			cout << "Connection failed";
		} else {
			cout << "Connected to server UGA" << endl;
		}

	}

	// sending request for server
	if (send(s, requestMessage, strlen(requestMessage), 0) < 0) {
		cout << "Failed to get send request";
	} else {
		cout << "Image request sent" << endl;
	}

	ofstream out;
	out.open(fileName.c_str(), ios::app);
	int size_recv = 0;
	int count=0;
	int kValue=0;
	int z;
	int header = 0;
	while((size_recv = recv(s, responseData, sizeof(responseData), 0)) > 0 && contentLength > 0){

		contentLength = contentLength - size_recv;

		for(int k=0;k<size_recv;k++){
			if((responseData[k]=='\r')&&(responseData[k+1]=='\n')&&(responseData[k+2]=='\r')&&(responseData[k+3]=='\n')){
				count++;
				kValue=k+4;

			}
		}
		if(count==1){
			for(z=0;kValue<size_recv;z++)
			{
				responseData2[z]=responseData[kValue];
				kValue++;
			}
			out.write(responseData2, z);
		}else {
			out.write(responseData, size_recv);
		}

		count=0;

	}

	//closing file and socket
	out.close();
	close(s);

	return NULL;

}
