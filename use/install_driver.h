/****************************************************************************

    Файл install_driver.h

    Заголовочный файл модуля install_driver.c

    Маткин Илья Александрович               01.04.2009

****************************************************************************/

#ifndef _INSTALL_DRIVER_H_
#define _INSTALL_DRIVER_H_

#include <windows.h>

//----------------------------------------

BOOLEAN InstallDriver(char* DriverName, char* LoadPath);

BOOLEAN RemoveDriver(char* DriverName);

BOOLEAN StartDriver(char* DriverName);

BOOLEAN StopDriver(char* DriverName);

#endif  // _INSTALL_DRIVER_H_
