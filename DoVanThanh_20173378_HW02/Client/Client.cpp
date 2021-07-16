#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Winsock2.h"
#include "Ws2tcpip.h"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	// Step 1 : Inittiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not suppoerted \n");
		return 0;
	}

	// Step 2 Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	// option Set time-out interval receiving 
	int tv = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	// Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));   // use atoi to convert char to int and use Command-Line Arguments
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr); // use Command-Line Arguments

	// Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))){
		printf("Error %d: Cannot connect server.", WSAGetLastError());

		return 0;
	}
	printf("Connected server \n");

	// Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret, messageLen;
	while (1) {
		// Send message
		printf("Send to server:");
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		if (messageLen == 0) break;

		ret = send(client, buff, messageLen, 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.", WSAGetLastError());

		// Receive message to server
		ret = recv(client, buff, BUFF_SIZE, 0);
		
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out");
			else printf("Error %d: Cannot receive data.", WSAGetLastError());
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			if (ret == 1) {
				// where the string contains special characters
				printf("Error!!");
			}
			else
			{
				//where the string contains only alphabet and numbers
				for (int i = 0; i <= ret; i++) {
					if (buff[i] == '&') {
						printf("\n");
					}
					else
					{
						printf("%c", buff[i]);
					}
				}
			}
			printf("\n");
		}
	}

	// Step 6: Close socket
	closesocket(client);

	// Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}
