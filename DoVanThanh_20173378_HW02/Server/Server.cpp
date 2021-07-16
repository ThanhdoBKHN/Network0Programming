#include "stdafx.h"
#include "stdio.h"
#include "winsock2.h"
#include "WS2tcpip.h"
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	// Step 1: Initiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported \n");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	// Step 3: Bind address socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));  // use atoi to convert char to int and use Command-Line Arguments
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error: %d Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	// Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error: %d cannot place server socket in state Listen", WSAGetLastError());
		return 0;
	}
	
	printf("Server started \n");

	// Step 5: Communiocate wwith client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN], buff1[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort, n1=0, n2=0;
	SOCKET connSock;
	// accept request
	connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen);
	if (connSock == SOCKET_ERROR) {
		printf("Error: %d Cannot permit incomming connection.", WSAGetLastError());
		return 0;
	}
	else {
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
		clientPort = ntohs(clientAddr.sin_port);
		printf("Acept connect from %s:%d \n", clientIP, clientPort);
	}

	while (1) {
		// recive message from client 
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot recive data to client.", WSAGetLastError());
			break;
		}
		else if (ret == 0) {
			printf("Client disconnects. \n");
			break;
		}
		else
		{
			buff[ret] = 0;
			buff1[ret] = 0; 
			printf("Recive from client [%s:%d] %s\n", clientIP, clientPort, buff);

			// process the received data to client
			//count alphabet and numbers in buff
			int n = (strlen(buff) - 1);
			for (int i = 0; i <= n; i++) {
				if ((('a' <= buff[i])&('z' >= buff[i])) || (('A' <= buff[i])&('Z' >= buff[i]))) {
					n1++;
				}
				else if (('0' <= buff[i])&('9' >= buff[i]))
				{
					n2++;
				}			
			}

			//separates alphabet and numbers with the characters '&' appended between
			if ((n1 + n2 - 1) == n) {
				int j = 0;

				// separate alphabet
				for (int i = 0; i <= n; i++) {
					if ((('a' <= buff[i])&('z' >= buff[i])) || (('A' <= buff[i])&('Z' >= buff[i]))) {
						buff1[j] = buff[i]; 
						j++;
					}
				}

				// add character"&"
				buff1[j] = '&'; j++; 
				
				//separate number
				for (int i = 0; i <= n; i++) {
					if (('0' <= buff[i])&('9' >= buff[i])) {
						buff1[j] = buff[i];
						j++;
					}
				}
				n1 = 0; n2 = 0; // reset value of variable count
				
				// send to client string buff1 added & between alphabet and numbers
				ret = send(connSock, buff1, strlen(buff) + 1, 0);
			}
			else
			{
				// send '*' to client when string with special characters
				n1 = 0; n2 = 0; // reset value of variable count
				ret = send(connSock, "*",1 , 0);
				
			}
				
			
			if (ret == SOCKET_ERROR) {
				printf("Error %d: Cannot send data to client \n", WSAGetLastError());
				break;
			}
		}
	} // end communiacation

	// Step 5 : Close socket
	closesocket(listenSock);
	closesocket(connSock);

	// Step 6 : Terminate Winsock
	WSACleanup();

	return 0;
}