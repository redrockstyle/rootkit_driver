#include <Windows.h>
#include <stdio.h>

#define sysenter __asm _emit 0x0F __asm _emit 0x34

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
	
	unsigned int cc = (unsigned int)NULL;
	unsigned int signature = 0xBAD0FACE;

	if (argc > 1) {
		if (!strcmp(argv[1], "syscall")) {
			__asm {
				push cc
				push cc
				push cc
				push cc
				push cc
				push cc
				push cc
				push cc
				push cc
				push cc
				push signature
				mov eax, 0x26
			}
			AddressSystemCall = (unsigned int)FastSystemCall;
			SysCall();
		}
	}

	return 0;
}