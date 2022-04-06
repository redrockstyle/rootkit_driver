/****************************************************************************

    ������ install_driver.cpp

    �������� �������� ������� ���������� ���������
    ����� ������� � ��������� �����.

    ������ ���� �������������               01.04.2009

****************************************************************************/

#include <windows.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

//----------------------------------------

// ������� �� ������� ��������� ��������� ����������� ���� ������
void PrintErrorMessage(DWORD err){
	char *msg;
    char *conmsg;
	DWORD res= FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			err,    // ��� ������
			0,      // ������������� ����� ��-���������
			(LPTSTR) &msg, 0, NULL);
	if(res) {
        conmsg=new char[strlen(msg)+1];
        CharToOemA(msg,conmsg);
		printf("%s",conmsg);    // ����� ��������� �� �����
		LocalFree(msg);     // ������������ ������ � ������� ���������
		delete [] conmsg;
	    }
    
    return;
}

//----------------------------------------

BOOLEAN InstallDriver(char* DriverName, char* LoadPath){
	
SC_HANDLE   SCManager;
SC_HANDLE   service;
DWORD       err;

    // �������� �������� �����
    SCManager=OpenSCManager(0,          // ��������� ���������
                            0,          // ������� ���� ������ ��������
                            SC_MANAGER_ALL_ACCESS   // ����� �������
                            );

    if(!SCManager){
        PrintErrorMessage(GetLastError());
        return FALSE;
        }
    

    service = CreateServiceA(SCManager,              // ��������� ���������� ��������
                               DriverName,              // ��� ���������������� �������
                               DriverName,              // ������������ ��� ���������������� �������
                               SERVICE_ALL_ACCESS,      // ��� ������� � �������
                               SERVICE_KERNEL_DRIVER,   // ��� �������
                               SERVICE_DEMAND_START,    // �������� �������:
                                                        // SERVICE_AUTO_START - ����������� ������������� ���������� ��������
                                                        // SERVICE_BOOT_START - ����������� ��� �������� �������
                                                        // SERVICE_DEMAND_START - ����������� ���������� �������� �� ����������
                               SERVICE_ERROR_NORMAL,    // ������� �� ������ ��� ������ �������
                               LoadPath,                // ���� � ������������ �����
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL
                               );

    if(service == NULL){
        err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS)
            return TRUE;
        else{
            PrintErrorMessage(err);
            return FALSE;
            }
        }

    CloseServiceHandle(service);

    CloseServiceHandle(SCManager);

    return TRUE;
}

//----------------------------------------

BOOLEAN RemoveDriver(char* DriverName){

SC_HANDLE   service;
SC_HANDLE   SCManager;

    SCManager=OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(!SCManager){
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    service = OpenServiceA(SCManager,
                          DriverName,
                          SERVICE_ALL_ACCESS
                          );

    if (service == NULL) {
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    if(!DeleteService(service)) {
        PrintErrorMessage(GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(SCManager);
        return FALSE;
        }

    CloseServiceHandle(service);

    CloseServiceHandle(SCManager);

    return TRUE;
}

//----------------------------------------

BOOLEAN StartDriver(char* DriverName){
    
SC_HANDLE   service;
SC_HANDLE   SCManager;
DWORD err;

    SCManager=OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(!SCManager){
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    service = OpenServiceA(SCManager, DriverName, SERVICE_ALL_ACCESS);

    if (service == NULL) {
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    if (!StartService(service, 0, NULL)) {

        err = GetLastError();

        if(err == ERROR_SERVICE_ALREADY_RUNNING)
            return TRUE;
        else{
            PrintErrorMessage(GetLastError());
            return FALSE;
            }
        }

    CloseServiceHandle(service);

    CloseServiceHandle(SCManager);

    return TRUE;
}

//----------------------------------------

BOOLEAN StopDriver(char* DriverName){

SC_HANDLE   service;
SC_HANDLE   SCManager;
SERVICE_STATUS serstatus;

    SCManager=OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(!SCManager){
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    service = OpenServiceA(SCManager, DriverName, SERVICE_ALL_ACCESS);

    if (service == NULL) {
        PrintErrorMessage(GetLastError());
        return FALSE;
        }

    if (!ControlService(service, SERVICE_CONTROL_STOP, &serstatus)){
        PrintErrorMessage(GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(SCManager);
        return FALSE;
        }

    CloseServiceHandle(service);

    CloseServiceHandle(SCManager);

    return TRUE;

}

//----------------------------------------


