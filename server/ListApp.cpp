#include "ListApp.h"
#include <tlhelp32.h>
#include "resource.h"

//ListApp::ListApp() {
//    GetAppList();
//}

vector<Wnd> oWnds;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam)
{
    int length = GetWindowTextLength(hwnd);
    wchar_t* buffer = new wchar_t[length + 1];
    GetWindowText(hwnd, buffer, length + 1);
    //wstring windowTitle(buffer);

    DWORD process_ID;
    // List visible windows with a non-empty title
    if (IsWindowVisible(hwnd) && (hwnd == GetAncestor(hwnd, GA_ROOT)) && length != 0) {
        GetWindowThreadProcessId(hwnd, &process_ID);
        Wnd w;
        w.id = process_ID;
        memcpy(w.name, buffer, (length + 1) * sizeof(WCHAR));
        delete[] buffer;
        oWnds.push_back(w);
    }

    return TRUE;
}

BOOL ListApp::GetAppList()
{
    EnumWindows(EnumWindowsProc, 0);
    for (int i = 0; i < oWnds.size(); i++)
        openingWnds.push_back(oWnds[i]);
    oWnds.clear();
    return(TRUE);
}


bool ListApp::sendList(CSocket& client) {
    openingWnds.clear();
    GetAppList();
    //Size
    int countProcess = openingWnds.size();
    client.Send(&countProcess, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    //Content
    if (countProcess != 0) {
        client.Send(&openingWnds[0], countProcess * sizeof(Wnd), 0);
        if (GetLastError() != 0)
            return false;
        else
            return true;
    }
}
