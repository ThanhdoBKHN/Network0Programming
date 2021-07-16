// WSAAsyncSelectServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Server.h"
#include "winsock2.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include "ws2tcpip.h"
#include "time.h"

#define WM_SOCKET WM_USER + 1
#define SERVER_PORT 6000
#define SERVER_ADDR "127.0.0.1"
#define MAX_CLIENT 1024
#define BUFF_SIZE 2048
#define FILE_ACC "account.txt"
#define FILE_LOG "log_20173378.txt"
#pragma comment(lib, "Ws2_32.lib")

//struct for client and server communication
typedef struct {
	int function;
	char post[BUFF_SIZE];
	int log_status;
} POST_STRUCT;

//declare struct server and client
POST_STRUCT Server_Post;
POST_STRUCT Client_Post;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	windowProc(HWND, UINT, WPARAM, LPARAM);

SOCKET client[MAX_CLIENT];
SOCKET listenSock;

// function check exist of user in file account.txt
// return true when user open and opposite
bool checkUser(char* user) {
	// read file 
	FILE *fptr;
	int check = 0;
	char line[250];
	fopen_s(&fptr, FILE_ACC, "r");
	strcat_s(user, 20, " 0");
	while (fgets(line, 250, fptr))
	{
		char *ptr = strstr(line, user);
		if (ptr != NULL) {
			check = 1;
			break;
		}
	}
	fclose(fptr);
	if (check == 1) {
		return true; // user open
	}
	else
	{
		return false;  // user close
	}
}

// function to save state to file "log_20173378"
void saveStatus(int selected_function, char *user, char *status) {
	time_t currentTime;
	time(&currentTime);

	struct tm *myTime = localtime(&currentTime);
	FILE *fptr;
	fopen_s(&fptr, FILE_LOG, "a+");
	fprintf(fptr, "%s:%d $[%i/%i/%i %i:%i:%i]$ %d: %s: %s \n", SERVER_ADDR, SERVER_PORT, myTime->tm_mon + 1, myTime->tm_mday, myTime->tm_year + 1900, myTime->tm_hour, myTime->tm_min, myTime->tm_sec, selected_function, user, status);
	fclose(fptr);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND serverWindow;

	//Registering the Window Class
	MyRegisterClass(hInstance);

	//Create the window
	if ((serverWindow = InitInstance(hInstance, nCmdShow)) == NULL)
		return FALSE;

	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		MessageBox(serverWindow, L"Winsock 2.2 is not supported.", L"Error!", MB_OK);
		return 0;
	}

	//Construct socket	
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);  // use atoi to convert char to int and use Command-Line Arguments
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);    // SERVER_ADDR

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot associate a local address with server socket.", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot place server socket in state LISTEN.", L"Error!", MB_OK);
		return 0;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	int i;
	for (i = 0; i <MAX_CLIENT; i++)
		client[i] = 0;
	hWnd = CreateWindow(L"WindowClass", L"Login TCP Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_SOCKET	- process the events on the sockets
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int ret, clientAddrLen = sizeof(clientAddr), i;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];

	switch (message) {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					continue;
				}
		}

		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_ACCEPT:
		{
			connSock = accept((SOCKET)wParam, (sockaddr *)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET) {
				break;
			}
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == 0) {
					client[i] = connSock;
					break;
					//requests Windows message-based notification of network events for listenSock
					WSAAsyncSelect(client[i], hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
				}
			if (i == MAX_CLIENT)
				MessageBox(hWnd, L"Too many clients!", L"Notice", MB_OK);
		}
		break;

		case FD_READ:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam)
					break;

			ret = recv(client[i], (char*)&Client_Post, sizeof(POST_STRUCT), 0);
			if (ret > 0) {
				// communicate with client
				if (Client_Post.function == 1) {
					// option 1 LOGIN
					if (checkUser(Client_Post.post) == TRUE) {
						saveStatus(Client_Post.function, Client_Post.post, "+OK");
						Server_Post.function = 1;
						Server_Post.log_status = 1;
						strcpy(Server_Post.post, "+OK you are logged in");
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
					else
					{
						saveStatus(Client_Post.function, Client_Post.post, "-ERR");
						Server_Post.function = 1;
						strcpy(Server_Post.post, "-ERR you are not logged in");
						Server_Post.log_status = 0;
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
				}
				else if (Client_Post.function == 2) {
					// option 2 POST
					if (Client_Post.log_status == 1) {
						saveStatus(Client_Post.function, Client_Post.post, "+OK");
						Server_Post.function = 2;
						strcpy(Server_Post.post, "+OK you have successfully posted");
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
					else
					{
						saveStatus(Client_Post.function, " ", "-ERR");
						Server_Post.function = 2;
						strcpy(Server_Post.post, "-ERR You are not logged in");
						Server_Post.log_status = 0;
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}

				}
				else if (Client_Post.function == 3)
				{
					// option 1 LOGOUT
					if (Client_Post.log_status == 0) {
						saveStatus(Client_Post.function, " ", "+OK");
						Server_Post.function = 3;
						strcpy(Server_Post.post, "+OK you are logged out");
						Server_Post.log_status = 0;
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}
					else if (Client_Post.log_status == 1)
					{
						saveStatus(Client_Post.function, " ", "-ERR");
						Server_Post.function = 3;
						strcpy(Server_Post.post, "-ERR You are not logged in, can't log out");
						ret = send(client[i], (char*)&Server_Post, sizeof(POST_STRUCT), 0);
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data.", WSAGetLastError());
					}

				}
				else if (Client_Post.function == 4)
				{
					// option 4 QUIT
					saveStatus(Client_Post.function, "QUIT", "+OK");
				}
			}
		}
		break;

		case FD_CLOSE:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					break;
				}
		}
		break;
		}
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}