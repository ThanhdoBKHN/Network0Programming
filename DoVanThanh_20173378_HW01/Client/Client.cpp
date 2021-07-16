#include "stdio.h"
#include "stdafx.h"
#include "stdlib.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported \n");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}
	printf("Client started! \n");

	// (optional) Set time-out for receiving
	int tv = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	// Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr), messageLen;
	while (1) {

		// Send message
		printf("Send to server:");
		gets_s(buff, BUFF_SIZE);  // received from the keyboard
		messageLen = strlen(buff);
		if (messageLen == 0) break;
		ret = sendto(client, buff, messageLen, 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: cannot send mesage ", WSAGetLastError());
		}

		// Receive message to server
		ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time out ");
			else printf("Error %d: Cannot recive message.", WSAGetLastError());
		}
		else if (strlen(buff)) {
			buff[ret] = 0;
			printf("Receive from server: %s \n", buff);
		}
	}

	// Step5 : Close socket
	closesocket(client);

	// Step 6: Terminate winsock
	WSACleanup();

	return 0;
}