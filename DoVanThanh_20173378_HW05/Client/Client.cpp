#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Winsock2.h"
#include "Ws2tcpip.h"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

//struct for client and server communication
typedef struct {
	int function;
	char post[BUFF_SIZE];
	int log_status;
} POST_STRUCT;

char check[20];

int main(int argc, char* argv[])
{
	//declare struct server and client
	POST_STRUCT Server_Post;
	POST_STRUCT Client_Post;

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
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect server.", WSAGetLastError());

		return 0;
	}
	printf("Connected server \n");

	// Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret, messageLen;
	int selection;
	Client_Post.log_status = 0;
	Server_Post.log_status = 0;

	while (1) {

		// Print MENU for user
		printf("=============================================\n");
		printf("MENU\n");
		printf("=============================================\n");
		printf("1. Log in\n2. Post message\n3. Logout\n4. Exit\n");
		printf("Choose an option: ");
		scanf_s("%d", &selection);
		getchar();
		int i = 0;

		switch (selection) {
		case 1:
			//option 1 LOGIN
			if (Server_Post.log_status == 1) {
				printf("you are logged in \n");
				Client_Post.function = 1;
				Client_Post.log_status = 0;
				ret = send(client, (char *)&Client_Post, sizeof(Client_Post), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
			}
			else
			{
				char buff[20];
				printf("Enter your user \n");
				gets_s(buff, 20);
				fflush(stdin);
				Client_Post.function = 1;
				i++;
				if (i = 1) {
					strcpy(check, buff);
					strcpy(Client_Post.post, buff);
					ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
					if (ret == SOCKET_ERROR)
						printf("Error %d: Cannot send data.", WSAGetLastError());
				}
				else
				{
					if ((strcmp(check, buff) == 0)) {
						strcpy(Client_Post.post, buff);
						ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
					else
					{
						Client_Post.log_status = 0;
						ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
				}
			}
			break;
		case 2:
			//option 2 POST
			char mess[200];  // post max 200 char
			printf("Please enter the content you want to post \n");
			gets_s(mess, 200);
			fflush(stdin);
			if (Server_Post.log_status == 1) {
				printf("The content you posted is %s \n", mess);
				Client_Post.function = 2;
				strcpy(Client_Post.post, mess);
				Client_Post.log_status = 1;
				ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.", WSAGetLastError());
			}
			else
			{
				printf("You are not logged in \n");
				Client_Post.function = 2;
				strcpy(Client_Post.post, "-ERR");
				ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.", WSAGetLastError());
			}
			break;
		case 3:
			// option 1 LOGOUT
			if (Server_Post.log_status == 1) {
				printf("Logout success \n");
				Client_Post.function = 3;
				strcpy(Client_Post.post, "+OK");
				Client_Post.log_status = 0;
				ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.", WSAGetLastError());
			}
			else
			{
				printf("You are no login \n");
				Client_Post.function = 3;
				strcpy(Client_Post.post, "-ERR");
				ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.", WSAGetLastError());
			}
			break;
		case 4:
			// //option 4  QUIT APP CLIENT
			printf("quiting app\n");
			Client_Post.function = 4;
			ret = send(client, (char*)&Client_Post, sizeof(POST_STRUCT), 0);
			if (ret == SOCKET_ERROR)
				printf("Error %d: Cannot send data.", WSAGetLastError());
			exit(0);
			break;
		default:
			printf(" Invalid selection\n");
			break;
		}

		// Receive message to server
		ret = recv(client, (char*)&Server_Post, sizeof(POST_STRUCT), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot recv data.", WSAGetLastError());
		printf("%d Status: %s \n", Server_Post.function, Server_Post.post);

	}

	// Step 6: Close socket
	closesocket(client);

	// Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}
