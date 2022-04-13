#ifndef _COMMAND_H_
#define _COMMAND_H_

typedef enum _TYPE_COMMAND {
	TestCommand = 0,
	RenameProcess
} TYPE_COMMAND, *PTYPE_COMMAND;

typedef struct _COMMAND {

	enum _TYPE_COMMAND type;
	PVOID target;
	PVOID change;

} COMMAND, *PCOMMAND;

#endif // !_COMMAND_H_
