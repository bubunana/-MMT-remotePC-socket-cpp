#pragma once
#include "afxsock.h"
#include "Process.h"
#include <vector>

using namespace std;

class ListProcess {
public:
    vector<Process> m_processes;
public:
    //ListProcess();
    //~ListProcess();
    BOOL GetProcessList();
    bool sendList(CSocket& client);
};