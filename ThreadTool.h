
#pragma once

#include <wtypes.h>
class CSync
{
public:
  CSync() { InitializeCriticalSection(&m_CriticalSection); }
  ~CSync() { DeleteCriticalSection(&m_CriticalSection); }
  void Acquire() { EnterCriticalSection(&m_CriticalSection); }
  void Release() { LeaveCriticalSection(&m_CriticalSection); }


private:
  CRITICAL_SECTION m_CriticalSection;              // Synchronization object
};

class CLockGuard
{
public:
  CLockGuard(CSync &refSync) : m_refSync(refSync) { Lock(); }
  ~CLockGuard() { Unlock(); }


private:
  CSync &m_refSync;                     // Synchronization object

  CLockGuard(const CLockGuard &refcSource);
  CLockGuard &operator=(const CLockGuard &refcSource);
  void Lock() { m_refSync.Acquire(); }
  void Unlock() { m_refSync.Release(); }
};


class CThreadControl 
{
    HANDLE hEvent_Begin;
    HANDLE hEvent_End;
    HANDLE hThread;
    bool bKillMe;
    DWORD nThreadId;

public:

    CThreadControl();
    ~CThreadControl();

    int CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);
    void CloseThread();
    void ResetEnd()
    {
        ResetEvent(hEvent_End);
    }
    void ResetBegin()
    {
        ResetEvent(hEvent_Begin);
    }
    void SignalEnd()
    {
        SetEvent(hEvent_End);
    }
    DWORD WaitBegin(DWORD tmTimeOut = INFINITE)
    {
        return WaitForSingleObject(hEvent_Begin, tmTimeOut);
    }
    void SignalBegin()
    {
        SetEvent(hEvent_Begin);
    }
    DWORD WaitComplete(DWORD tmTimeOut = INFINITE)
    {
        return WaitForSingleObject(hEvent_End, tmTimeOut);
    }
    bool TimeToDie() { return bKillMe; }
    BOOL SetPriority(const int nPriority)
    {
        return ::SetThreadPriority(hThread, nPriority);
    }
};
