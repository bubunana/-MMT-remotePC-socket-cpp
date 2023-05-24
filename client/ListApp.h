#pragma once
#pragma once
#include "afxsock.h"
#include <vector>
#include "framework.h"
#include "Wnd.h"

using namespace std;

#define MAX_COLUMN_LENGTH 100

class ListApp {
public:
    vector<Wnd> openingWnds;
    HWND buttons[4];
    HWND hwndInput;
public:
    void mainLoop(CSocket& client, HWND hWnd, HWND chWnd, HWND hwndListProcess, HINSTANCE hInst, int& commandNum);
    bool receiveList(CSocket& client);
    bool updateList(HWND hwndListProcess, HWND hwndParent, CSocket& client);
    bool startProcess(WCHAR* processName);
    bool endProcess();

    //utils
    void hideButtons();
    void showButtons();
    HWND CreateAListApp(HWND hwndParent, HINSTANCE hInst);
    BOOL InitListAppItems(HWND hwndTV, HWND hwndParent);
    LVITEM AddItemToList(HWND hwndTV, HWND hwndParent, LPTSTR lpszItem, int index, DWORD id);
};
