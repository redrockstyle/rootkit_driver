#ifndef _COMMAND_H_
#define _COMMAND_H_

//#include <windows.h>

#define COMMAND_TEST_COMMAND		0x10000000
#define COMMAND_RENAME_PROCESS		0x20000000
#define COMMAND_HIDE_FILE			0x40000000
#define COMMAND_HIDE_NET			0x80000000
#define COMMAND_RENAME_KEY			0x01000000
#define COMMAND_KEYBOARD			0x02000000

#define COMMAND_BUFFER_POINTER		0x00000001
#define COMMAND_BUFFER_NUMBER		0x00000002

#define COMMAND_BUFFER_SRC_PORT		0x00000004
#define COMMAND_BUFFER_DST_PORT		0x00000008

typedef struct _COMMAND {

	ULONG flags;
	PVOID target;
	PVOID change;

} COMMAND, *PCOMMAND;

#endif // !_COMMAND_H_
