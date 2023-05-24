#pragma once
#include "stdafx.h"

class KeyLog {
public:
    HWND buttons[4]; //stop, refresh, save log, quit
    HWND continueButton;
    bool isRefresh;
    bool isSave;
public:
    HWND createLogWindow(HWND hwndParent, HINSTANCE hInst);
    void AppendText(HWND hEditWnd, LPCTSTR Text);
    void saveLogFile(HWND hEditWnd);
    void hideButtons();
    void showButtons();
};