#include "KeyLog.h"
#include <vector>
#include <fstream>

using namespace std;

HWND KeyLog::createLogWindow(HWND hwndParent, HINSTANCE hInst) {
    //HWND hwndLogWnd = CreateWindow(L"EDIT", // We are creating an Edit control
    //    NULL,   // Leave the control empty
    //    WS_CHILD | WS_VISIBLE | WS_HSCROLL |
    //    WS_VSCROLL | ES_LEFT | ES_MULTILINE |
    //    ES_AUTOHSCROLL | ES_AUTOVSCROLL,
    //    0, 0, 960, 540,
    //    hwndParent,
    //    0,
    //    hInst, NULL);
    //RECT rcClient; // dimensions of client area
    InitCommonControls();
    //GetClientRect(hwndParent, &rcClient);

    HWND hwndLogWnd = CreateWindowEx(0,
        L"edit",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY,
        300, 10, 960, 540,
        hwndParent,
        NULL,
        hInst,
        NULL);
    return hwndLogWnd;
}

void KeyLog::AppendText(HWND hEditWnd, LPCTSTR Text)
{
    int idx = GetWindowTextLength(hEditWnd);
    SendMessage(hEditWnd, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
    SendMessage(hEditWnd, EM_REPLACESEL, 0, (LPARAM)Text);

    //// get new length to determine buffer size
    //int outLength = GetWindowTextLength(hEditWnd) + lstrlen(Text) + 1;

    //// create buffer to hold current and new text
    //std::vector<TCHAR> buf(outLength);
    //TCHAR* pbuf = &buf[0];

    //// get existing text from edit control and put into buffer
    //GetWindowText(hEditWnd, pbuf, outLength);

    //// append the newText to the buffer
    //_tcscat_s(pbuf, outLength, Text);

    //// Set the text in the edit control
    //SetWindowText(hEditWnd, pbuf);
}

void KeyLog::saveLogFile(HWND hEditWnd) {
    int outLength = GetWindowTextLength(hEditWnd) + 1;

    // create buffer to hold current and new text
    std::vector<TCHAR> buf(outLength);
    TCHAR* pbuf = &buf[0];

    // get existing text from edit control and put into buffer
    GetWindowText(hEditWnd, pbuf, outLength);

    OPENFILENAME ofn;
    WCHAR srcPath[MAX_PATH];
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Text file (*.txt)\0*.txt\0";
    GetCurrentDirectory(MAX_PATH, srcPath);
    ofn.lpstrFile = srcPath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileName(&ofn))
    {
        ofstream fout(ofn.lpstrFile, ios::binary);
        if (fout) {
            fout.flush();

            if (buf.size() > 0)
                fout.write((char*)pbuf, outLength * sizeof(TCHAR));

            fout.close();
            //commandNum = 0;
        }
        //else
            //commandNum = 1;
        //        break;
        //    }

    }
}

void KeyLog::hideButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_HIDE);
}

void KeyLog::showButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
}

