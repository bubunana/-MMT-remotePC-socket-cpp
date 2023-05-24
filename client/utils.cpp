#include "utils.h"
#include <strsafe.h>
#include "GlobalVariables.h"

using namespace std;


void hideOriginalButtons(HWND buttons[], HWND IPinput) {
    for (int i = 0; i < 6; ++i)
        ShowWindow(buttons[i], SW_HIDE);
    ShowWindow(IPinput, SW_HIDE);

}

void showOriginalButtons(HWND buttons[], HWND IPinput) {
    for (int i = 0; i < 6; ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
    ShowWindow(IPinput, SW_NORMAL);

}

void DisableOriginalButton(HWND buttons[])
{
    for (int i = 1; i < 6; ++i)
        EnableWindow(buttons[i], FALSE);
}

void EnableOriginalButton(HWND buttons[])
{
    for (int i = 1; i < 6; ++i)
        EnableWindow(buttons[i], TRUE);
}

void DisableCloseButton(HWND hwnd)
{
    EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE,
        MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}

void EnableCloseButton(HWND hwnd)
{
    EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE,
        MF_BYCOMMAND | MF_ENABLED);
}

void DisplayNotiBox(LPTSTR message) {
    MessageBox(
        NULL,
        message,
        L"Notification",
        MB_ICONINFORMATION | MB_OK);
}

void DisplayErrorBox(LPTSTR message) {
    MessageBox(
        NULL,
        message,
        L"Error",
        MB_ICONERROR | MB_OK);
}

//void DisplayErrorBox(LPTSTR lpszFunction)
//{
//    // Retrieve the system error message for the last-error code
//
//    LPVOID lpMsgBuf;
//    LPVOID lpDisplayBuf;
//    DWORD dw = GetLastError();
//
//    FormatMessage(
//        FORMAT_MESSAGE_ALLOCATE_BUFFER |
//        FORMAT_MESSAGE_FROM_SYSTEM |
//        FORMAT_MESSAGE_IGNORE_INSERTS,
//        NULL,
//        dw,
//        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//        (LPTSTR)&lpMsgBuf,
//        0, NULL);
//
//    // Display the error message and clean up
//
//    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
//        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
//    StringCchPrintf((LPTSTR)lpDisplayBuf,
//        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
//        TEXT("%s failed with error %d: %s"),
//        lpszFunction, dw, lpMsgBuf);
//    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
//
//    LocalFree(lpMsgBuf);
//    LocalFree(lpDisplayBuf);
//}
