#include "stdio.h"
#include "stdafx.h"
#include "stdlib.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[]) {
	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported \n");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));  // use atoi to convert string to int and Command-Line Arguments
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot bind this address.", WSAGetLastError());
		return 0;
	}
	printf("server started");
	
	// Step4: Communicate wwith client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;

	while (1) {
		// Receive message to client
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot receive data.", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Receive from client [%s:%d] %s \n", clientIP, clientPort, buff);
			
			// use getaddrinfo to convert ipv4 or domain name
			addrinfo *result;
			int rc;
			sockaddr_in *address;
			addrinfo hints;
			memset(&hints, 0, sizeof(hints)); // set hints is zero
			hints.ai_family = AF_INET;
			rc = getaddrinfo(buff, NULL, &hints, &result);
			char ipStr[INET_ADDRSTRLEN];
			if (rc == 0) {
				address = (struct sockaddr_in *) result->ai_addr;
				inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
				printf("IPV4 address %s\n", ipStr);

				// send result invalid to client
				ret = sendto(server, ipStr, strlen(ipStr), 0, (SOCKADDR *)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data", WSAGetLastError());
			}
			else
			{
				printf("getaddrinfo() error: %d", WSAGetLastError());

				// send notification "error" to client
				ret = sendto(server, "Not Find to Your Address", 50, 0, (SOCKADDR *)&clientAddr, sizeof(clientAddr));
			}
			freeaddrinfo(result);
			
		}
	}

	// Step 5
	closesocket(server);

	// Step 6 Terminate winsock
	WSACleanup();

	return 0;

}