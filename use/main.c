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

	memset(&cmd, 0, sizeof(COMMAND));

	if (argc > 1) {
		if (!strcmp(argv[1], "0x26")) {
			cmd.type = TestCommand;
			cmd.bufByte = NULL;
			cmd.bufInt = 0;
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
			if (argc == 3) {
				cmd.bufInt = (unsigned int)strtoul(argv[2], NULL, 0);
				cmd.bufByte = NULL;
				cmd.type = RenameProcess;

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
			if (argc == 3) {
				unsigned int len = strlen(argv[2]);
				cmd.bufInt = 0;
				cmd.bufByte = (unsigned char*)malloc(len);
				strcpy(cmd.bufByte, argv[2]);
				cmd.type = RenameProcess;

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