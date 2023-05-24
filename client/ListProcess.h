#pragma once
#include "afxsock.h"
#include <vector>
#include "framework.h"
#include "Process.h"

using namespace std;

#define MAX_COLUMN_LENGTH 100

class ListProcess {
public:
    vector<Process> m_processes;
    HWND buttons[4];
    HWND hwndInput;
public:
    //ListProcess();
    //~ListProcess();
    void mainLoop(CSocket& client, HWND hWnd, HWND chWnd, HWND hwndListProcess, HINSTANCE hInst, int& commandNum);
    bool receiveList(CSocket& client);
    bool updateList(HWND hwndListProcess, HWND hwndParent, CSocket& client);
    bool startProcess(WCHAR* processName);
    bool endProcess();

    //utils
    void hideButtons();
    void showButtons();
    HWND CreateAListProcess(HWND hwndParent, HINSTANCE hInst);
    BOOL InitListProcessItems(HWND hwndTV, HWND hwndParent);
    LVITEM AddItemToList(HWND hwndTV, HWND hwndParent, LPTSTR lpszItem, int index, DWORD id, DWORD threadCount);
};