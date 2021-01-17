
//#include "stdafx.h"
#include "ThreadTool.h"

CThreadControl::CThreadControl() 
{
    bKillMe = false;
    nThreadId = -1;
    hThread = INVALID_HANDLE_VALUE;
    hEvent_Begin = CreateEvent(NULL, FALSE, FALSE, NULL);
    hEvent_End = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CThreadControl::~CThreadControl() 
{
    CloseThread();
    CloseHandle(hEvent_Begin);
    CloseHandle(hEvent_End);
}

int CThreadControl::CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter) 
{
    bKillMe = false;
    hThread = ::CreateThread(NULL, NULL, lpStartAddress, lpParameter, NULL, &nThreadId);
    if(hThread == INVALID_HANDLE_VALUE)
        return -1;
    return 0;
}

void CThreadControl::CloseThread()
{
    if(hThread == INVALID_HANDLE_VALUE)
        return;
    bKillMe = true;
    SetEvent(hEvent_Begin);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    hThread = INVALID_HANDLE_VALUE;
    bKillMe = false;
}
