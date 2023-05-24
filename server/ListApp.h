#pragma once

#pragma once
#include "afxsock.h"
#include "Wnd.h"
#include <vector>

using namespace std;

class ListApp {
public:
    vector<Wnd> openingWnds;
    HWND g_HWND = NULL;
public:
    //ListApp();
    BOOL GetAppList();
    bool sendList(CSocket& client);
};