#pragma once
#include "framework.h"
#include <vector>
#include "stdafx.h"
#include "GlobalVariables.h"

using namespace std;

class LiveScreen {
public:
    vector<BYTE> bmp_data;
    HWND buttons[3];
    HWND continueButton;
    bool pauseScreen = false;
    bool isCapture = false;
public:
    bool captureScreen();
    //unsigned __stdcall mainLoop(CSocket& client, HWND hwnd);
    void hideButtons();
    void showButtons();
    int DisplayCapture(HDC hDC, vector<BYTE> data, HWND hwndLiveScreen, int Width, int Height);
};


