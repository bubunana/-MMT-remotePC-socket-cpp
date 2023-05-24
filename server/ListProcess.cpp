#include "ListProcess.h"
#include <tlhelp32.h>
#include "resource.h"


//ListProcess::ListProcess(){
//    GetProcessList();
//}

BOOL ListProcess::GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        //printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        //printError(TEXT("Process32First")); // show cause of failure
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do
    {
        Process p;
        //Get name
        memcpy(p.name, pe32.szExeFile, sizeof(pe32.szExeFile));

        //// Retrieve the priority class.
        //dwPriorityClass = 0;
        //hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        //if (hProcess == NULL) {
        //    //printError(TEXT("OpenProcess"));
        //}
        //else
        //{
        //    dwPriorityClass = GetPriorityClass(hProcess);
        //    if (!dwPriorityClass) {
        //        //printError(TEXT("GetPriorityClass"));
        //    }
        //    CloseHandle(hProcess);
        //}

        //_tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
        //Get ID
        p.id = pe32.th32ProcessID;
        //_tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
        //Get thread count
        p.threadCount = pe32.cntThreads;
        //_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
        //_tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
        //if (dwPriorityClass)
        //    _tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

        // List the modules and threads associated with this process
        //ListProcessModules(pe32.th32ProcessID);
        //ListProcessThreads(pe32.th32ProcessID);
        m_processes.push_back(p);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}

bool ListProcess::sendList(CSocket& client) {
    m_processes.clear();
    GetProcessList();   
    //Size
    int countProcess = m_processes.size();
    client.Send(&countProcess, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    //Content
    if (countProcess != 0) {
        client.Send(&m_processes[0], countProcess * sizeof(Process), 0);
        if (GetLastError() != 0)
            return false;
        else
            return true;
    }
}
