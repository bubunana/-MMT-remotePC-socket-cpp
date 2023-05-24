// client.cpp : Defines the entry point for the application.
//
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "afxsock.h"
#include "GlobalVariables.h"

#include "LiveScreen.h"
#include "Heading.h"
#include "DirectoryTree.h"
#include "ListProcess.h"
#include "KeyLog.h"
#include "ListApp.h"
#include "utils.h"

#include "framework.h"
#include "client.h"
#include <objidl.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <commctrl.h> //treeview
#include <sstream>
#include <filesystem>
#pragma comment(lib, "shlwapi.lib")
#include <Shlwapi.h>
#pragma comment(lib,"COMCTL32.LIB")
#pragma comment(lib, "User32.lib")
#pragma comment(lib,"gdiplus.lib")
#include <algorithm> // for sort list view

using namespace Gdiplus;
using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND hWnd;
//
HWND chWnd;
int Width = 960;
int Height = 540;
//
HWND hwndTreeView;
HWND hwndListApp;
HWND hwndListProcess;
HWND hwndLiveScreen;
HWND hwndLog; 
HWND buttons[6]; 
HWND IPinput;

CSocket client;
int commandNum;
wchar_t serverIP[20];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//
LRESULT CALLBACK    DisplayResultWndProc(HWND, UINT, WPARAM, LPARAM);
HWND CreateDisplayResultWnd(HWND);
void RegisterDisplayResultWndClass(HWND);
INT_PTR CALLBACK ProcessDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


//
// Socket and process
bool liveScreenMainLoop();
bool listAppMainLoop();
bool listProcessMainLoop();
bool directoryTreeMainLoop();
bool keyLogMainLoop();
unsigned __stdcall connectToServer(void* id);

//Global variables for socket
//For list running app
ListApp la;
WCHAR inputName[MAX_HEADING_LEN];

// For list process
ListProcess lp;

// For screen live
LiveScreen liveScreen;

//For tree directory
LRESULT CALLBACK    TreeViewWndProc(HWND, UINT, WPARAM, LPARAM);
void RegisterTreeViewWndClass(HWND);

DirectoryTree dt;


//For keylog
KeyLog kl;
bool isQuit = false;



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    //Gdiplus
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //Shut it down
    Gdiplus::GdiplusShutdown(gdiplusToken); 

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, 1320, 670, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//void DisableOriginalButton()
//{
//    for (int i = 1; i < size(buttons); ++i)
//        EnableWindow(buttons[i], FALSE);
//}
//
//void EnableOriginalButton()
//{
//    for (int i = 1; i < size(buttons); ++i)
//        EnableWindow(buttons[i], TRUE);
//}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        RegisterDisplayResultWndClass(hWnd);

        IPinput = CreateWindow(L"EDIT", 0, WS_BORDER | WS_CHILD | WS_VISIBLE, 30, 10, 200, 30, hWnd, 0, hInst, 0);

        buttons[0] = CreateWindow(TEXT("button"), TEXT("Connect to this IP"), WS_VISIBLE | WS_CHILD, 
            30, 50, 200, 50, hWnd, (HMENU)CONNECT_SERVER_BUTTON, NULL, NULL);

        buttons[1] = CreateWindow(TEXT("button"), TEXT("List App"), WS_VISIBLE | WS_CHILD,
            30, 190, 200, 50, hWnd, (HMENU)LIST_APPLICATION_BUTTON, NULL, NULL);

        buttons[2] = CreateWindow(TEXT("button"), TEXT("List Process"), WS_VISIBLE | WS_CHILD,
            30, 250, 200, 50, hWnd, (HMENU)LIST_PROCESS_BUTTON, NULL, NULL);

        buttons[3] = CreateWindow(TEXT("button"), TEXT("Live screen"), WS_VISIBLE | WS_CHILD,
            30, 310, 200, 50, hWnd, (HMENU)LIVE_SCREEN_BUTTON, NULL, NULL);

        buttons[4] = CreateWindow(TEXT("button"), TEXT("Keylog"), WS_VISIBLE | WS_CHILD,
            30, 370, 200, 50, hWnd, (HMENU)KEYLOG_BUTTON, NULL, NULL);

        buttons[5] = CreateWindow(TEXT("button"), TEXT("Access Folder Tree"), WS_VISIBLE | WS_CHILD,
            30, 430, 200, 50, hWnd, (HMENU)DIRECTORY_TREE_BUTTON, NULL, NULL);

        DisableOriginalButton(buttons);

        chWnd = CreateDisplayResultWnd(hWnd);
        
        //buttons for list running app
        la.buttons[0] = CreateWindow(TEXT("button"), TEXT("Refresh"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)REFRESH_APP_BUTTON, NULL, NULL);

        la.buttons[1] = CreateWindow(TEXT("button"), TEXT("Start an app (by name)"), WS_CHILD,
            10, 90, 200, 50, hWnd, (HMENU)START_APP_BUTTON, NULL, NULL);

        la.buttons[2] = CreateWindow(TEXT("button"), TEXT("End an app (by ID)"), WS_CHILD,
            10, 170, 200, 50, hWnd, (HMENU)END_APP_BUTTON, NULL, NULL);

        la.buttons[3] = CreateWindow(TEXT("button"), TEXT("Quit"), WS_CHILD,
            10, 250, 200, 50, hWnd, (HMENU)QUIT_BUTTON, NULL, NULL);

        //buttons for list process
        lp.buttons[0] = CreateWindow(TEXT("button"), TEXT("Refresh"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)REFRESH_PROCESS_BUTTON, NULL, NULL); 
        
        lp.buttons[1] = CreateWindow(TEXT("button"), TEXT("Start a process (by name)"), WS_CHILD,
            10, 90, 200, 50, hWnd, (HMENU)START_PROCESS_BUTTON, NULL, NULL);

        lp.buttons[2] = CreateWindow(TEXT("button"), TEXT("End a process (by ID)"), WS_CHILD,
            10, 170, 200, 50, hWnd, (HMENU)END_PROCESS_BUTTON, NULL, NULL);

        lp.buttons[3] = CreateWindow(TEXT("button"), TEXT("Quit"), WS_CHILD,
            10, 250, 200, 50, hWnd, (HMENU)QUIT_BUTTON, NULL, NULL);

        //buttons for livescreen
        liveScreen.buttons[0] = CreateWindow(TEXT("button"), TEXT("Pause"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)PAUSE_SCREEN_BUTTON, NULL, NULL);
        
        liveScreen.continueButton = CreateWindow(TEXT("button"), TEXT("Continue"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)CONTINUE_SCREEN_BUTTON, NULL, NULL);

        liveScreen.buttons[1] = CreateWindow(TEXT("button"), TEXT("Capture Screen"), WS_CHILD,
            10, 90, 200, 50, hWnd, (HMENU)CAPTURE_SCREEN_BUTTON, NULL, NULL);

        liveScreen.buttons[2] = CreateWindow(TEXT("button"), TEXT("Quit"), WS_CHILD,
            10, 170, 200, 50, hWnd, (HMENU)QUIT_BUTTON, NULL, NULL);

        //buttons for directory tree
        dt.buttons[0] = CreateWindow(TEXT("button"), TEXT("Delete"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)DELETE_F_BUTTON, NULL, NULL);

        dt.buttons[1] = CreateWindow(TEXT("button"), TEXT("Copy"), WS_CHILD,
            10, 90, 200, 50, hWnd, (HMENU)COPY_F_BUTTON, NULL, NULL);

        dt.buttons[2] = CreateWindow(TEXT("button"), TEXT("Cut"), WS_CHILD,
            10, 170, 200, 50, hWnd, (HMENU)CUT_F_BUTTON, NULL, NULL);

        dt.buttons[3] = CreateWindow(TEXT("button"), TEXT("Send a FILE"), WS_CHILD,
            10, 250, 200, 50, hWnd, (HMENU)SEND_F_BUTTON, NULL, NULL);

        dt.buttons[4] = CreateWindow(TEXT("button"), TEXT("Receive a FILE"), WS_CHILD,
            10, 330, 200, 50, hWnd, (HMENU)RECEIVE_F_BUTTON, NULL, NULL);

        dt.buttons[5] = CreateWindow(TEXT("button"), TEXT("Quit"), WS_CHILD,
            10, 410, 200, 50, hWnd, (HMENU)QUIT_BUTTON, NULL, NULL);

        dt.pasteButton = CreateWindow(TEXT("button"), TEXT("Paste"), WS_CHILD,
            10, 500, 130, 50, hWnd, (HMENU)PASTE_BUTTON, NULL, NULL);

        dt.cancelButton = CreateWindow(TEXT("button"), TEXT("Cancel"), WS_CHILD,
            150, 500, 130, 50, hWnd, (HMENU)CANCEL_BUTTON, NULL, NULL);

        //buttons for keylog
        kl.buttons[0] = CreateWindow(TEXT("button"), TEXT("Stop"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)STOP_KEYLOG_BUTTON, NULL, NULL);

        kl.continueButton = CreateWindow(TEXT("button"), TEXT("Continue"), WS_CHILD,
            10, 10, 200, 50, hWnd, (HMENU)CONTINUE_KEYLOG_BUTTON, NULL, NULL);

        kl.buttons[1] = CreateWindow(TEXT("button"), TEXT("Refresh"), WS_CHILD,
            10, 90, 200, 50, hWnd, (HMENU)REFRESH_KEYLOG_BUTTON, NULL, NULL);

        kl.buttons[2] = CreateWindow(TEXT("button"), TEXT("Save log file"), WS_CHILD,
            10, 170, 200, 50, hWnd, (HMENU)SAVE_LOGFILE_BUTTON, NULL, NULL);

        kl.buttons[3] = CreateWindow(TEXT("button"), TEXT("Quit"), WS_CHILD,
            10, 250, 200, 50, hWnd, (HMENU)QUIT_BUTTON, NULL, NULL);

        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case CONNECT_SERVER_BUTTON:
                //Get IP of server
                GetWindowText(IPinput, serverIP, sizeof(serverIP));
                //Hide button and set read only for edit control
                EnableWindow(IPinput, FALSE);
                EnableWindow(buttons[0], FALSE);

                _beginthreadex(nullptr, 0, connectToServer, nullptr, 0, nullptr);
                break;

            //List application
            case LIST_APPLICATION_BUTTON: 
                commandNum = LIST_APPLICATION;
                break;

            case REFRESH_APP_BUTTON:
                commandNum = UPDATE_LIST_APP;
                break;

            case START_APP_BUTTON:
                commandNum = START_APP;
                break;

            case END_APP_BUTTON:
                commandNum = END_APP;
                break;

            //List process
            case LIST_PROCESS_BUTTON: 
                commandNum = LIST_PROCESS;
                break;

            case REFRESH_PROCESS_BUTTON:
                commandNum = UPDATE_LIST;
                break;

            case START_PROCESS_BUTTON:
                commandNum = START_PROCESS;
                break;

            case END_PROCESS_BUTTON:
                commandNum = END_PROCESS;
                break;

            //Live screen
            case LIVE_SCREEN_BUTTON: 
                commandNum = LIVE_SCREEN;
                break;

            case PAUSE_SCREEN_BUTTON: //Pause live screen
                commandNum = PAUSE_SCREEN;
                break;

            case CONTINUE_SCREEN_BUTTON: //Continue live screen
                commandNum = CONTINUE_SCREEN;
                break;

            case CAPTURE_SCREEN_BUTTON: //Capture screen
                liveScreen.isCapture = true;
                break;

            //Directory Tree
            case DIRECTORY_TREE_BUTTON:
                commandNum = DIRECTORY_TREE;
                break;
            
            case DELETE_F_BUTTON: {
                commandNum = DELETE_F;               
                break;
            }

            case CUT_F_BUTTON: {
                commandNum = CUT_F;
                break;
            }

            case PASTE_BUTTON: {
                commandNum = PASTE_F;
                break;
            }

            case CANCEL_BUTTON: {
                commandNum = CANCEL_F;
                break;
            }

            case COPY_F_BUTTON: {
                commandNum = COPY_F;
                break;
            }

            case SEND_F_BUTTON: {
                commandNum = SEND_F;
                break;
            }

            case RECEIVE_F_BUTTON: {
                commandNum = RECEIVE_F;
                break;
            }

            case QUIT_BUTTON: {
                commandNum = QUIT_FEATURE;
                break;
            }

            //Keylog
            case KEYLOG_BUTTON:
                commandNum = KEYLOG;
                break;

            case STOP_KEYLOG_BUTTON:
                commandNum = STOP_KEYLOG;
                break;

            case CONTINUE_KEYLOG_BUTTON:
                commandNum = CONTINUE_KEYLOG;
                break;
                
            case REFRESH_KEYLOG_BUTTON:
                kl.isRefresh = true;
                ShowWindow(kl.buttons[1], SW_HIDE);
                ShowWindow(kl.buttons[1], SW_NORMAL);

                break;

            case SAVE_LOGFILE_BUTTON:
                kl.isSave = true;
                break;

            case CREATE_LOG_WND: {
                hwndLog = kl.createLogWindow(hWnd, hInst);
                break;
            }

            case SUCCESS_CONNECT: //connect successfully

                MessageBox(
                    NULL,
                    L"Connection successful!",
                    L"Notification",
                    MB_ICONINFORMATION | MB_OK
                    //MB_ICONEXCLAMATION | MB_YESNO
                );
                break;

            case SUCCESSFUL_FEATURE: {
                LPTSTR message = (LPTSTR)lParam;
                DisplayNotiBox(message);
                break;
            }

            case FAILED_FEATURE: {
                LPTSTR message = (LPTSTR)lParam;
                DisplayErrorBox(message);
                break;
            }

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            //DisplayCapture(nullptr);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        isQuit = true;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void RegisterDisplayResultWndClass(HWND hwnd)
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = (WNDPROC)DisplayResultWndProc;
    wc.hInstance = hInst;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("DisplayResult");
    RegisterClassEx(&wc);
}

HWND CreateDisplayResultWnd(HWND hWnd)
{
    return CreateWindow(TEXT("DisplayResult"), nullptr , WS_CHILD | WS_BORDER | WS_VISIBLE,
        300, 10, Width, Height, hWnd, NULL, NULL, NULL);
}

LRESULT CALLBACK DisplayResultWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    case WM_CREATE:
    {
        break;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
            case CREATE_LISTAPP_WND: {
                hwndListApp = la.CreateAListApp(hwnd, hInst);
                break;
            }
            case CREATE_LISTPROCESS_WND: {
                hwndListProcess = lp.CreateAListProcess(hwnd, hInst);
                break;
            }

            case CREATE_TREEVIEW_WND:
            {               
                hwndTreeView = dt.CreateATreeView(hwnd, hInst);
                break;
            }

            case DESTROY_TREE_VIEW: {
                DestroyWindow(hwndTreeView);
                break;
            }

            case DESTROY_LIST_APP: {
                DestroyWindow(hwndListApp);
                break;
            }

            case DESTROY_LIST_VIEW: {
                DestroyWindow(hwndListProcess);
                break;
            }

            case DESTROY_LOG_VIEW: {
                DestroyWindow(hwndLog);
                break;
            }
            
            default:
                DestroyWindow(hwnd);
        }
        break;
    }
    
    case WM_NOTIFY:
    {
        LPNMHDR lpnmh = (LPNMHDR)lParam;
        if (!lpnmh)
            return 0;
        if (!lpnmh->hwndFrom)
            return 0;
        if (lpnmh->idFrom == IDC_TREE)
        {
            if (lpnmh->code == NM_DBLCLK)
                commandNum = GET_FOLDER;  
            
            return NULL;
       
        }

        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
    }
    break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    }
    return (DefWindowProc(hwnd, msg, wParam, lParam));
}

bool liveScreenMainLoop() {
    isContinueFeature = true;
    liveScreen.isCapture = false; 
    hwndLiveScreen = CreateWindow(TEXT("edit"), nullptr, WS_VISIBLE | WS_CHILD | WS_BORDER,
        0, 0, Width, Height, chWnd, NULL, NULL, NULL);

    int data_size;
    client.Receive(&data_size, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    
    int sizeOfData = data_size / sizeof(BYTE);
    liveScreen.bmp_data.resize(sizeOfData);
    int count = 0;
    while (count < data_size) {
        count += client.Receive(&liveScreen.bmp_data[0] + count, data_size - count, 0);
        if (GetLastError() != 0)
            return false;
    }

    liveScreen.DisplayCapture(nullptr, liveScreen.bmp_data, hwndLiveScreen, Width, Height);

    commandNum = CONTINUE_SCREEN;
    do {
        switch (commandNum) {
            case CONTINUE_SCREEN: {
                client.Send(&commandNum, sizeof(int), 0); 
                if (GetLastError() != 0)
                    return false;
                ShowWindow(liveScreen.continueButton, SW_HIDE);
                ShowWindow(liveScreen.buttons[0], SW_NORMAL);

                int count = 0;
                while (count < data_size) {
                    count += client.Receive(&liveScreen.bmp_data[0] + count, data_size - count, 0);
                    if (GetLastError() != 0)
                        return false;
                }
                liveScreen.DisplayCapture(nullptr, liveScreen.bmp_data, hwndLiveScreen, Width, Height);
                break;
            }

            case PAUSE_SCREEN: {
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                ShowWindow(liveScreen.buttons[0], SW_HIDE);
                ShowWindow(liveScreen.continueButton, SW_NORMAL);

                break;
            }

            case QUIT_FEATURE: {
                client.Send(&commandNum, sizeof(int), 0); 
                if (GetLastError() != 0)
                    return false;
                isContinueFeature = false;
                break;
            }
        }
        if (liveScreen.isCapture) {
            int temp = commandNum;
            commandNum = 0;
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false; 
            liveScreen.captureScreen();
            liveScreen.isCapture = false;
            commandNum = temp;
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
        }
    } while (isContinueFeature);
    return true;
}

INT_PTR CALLBACK ProcessDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD cchName;

    switch (message)
    {
    case WM_INITDIALOG:
        // Set the default push button to "Cancel." 
        SendMessage(hDlg,
            DM_SETDEFID,
            (WPARAM)IDCANCEL,
            (LPARAM)0);

        return TRUE;

    case WM_COMMAND:
        // Set the default push button to "OK" when the user enters text. 
        if (HIWORD(wParam) == EN_CHANGE &&
            LOWORD(wParam) == IDE_NAMEEDIT)
        {
            SendMessage(hDlg,
                DM_SETDEFID,
                (WPARAM)IDOK,
                (LPARAM)0);
        }
        switch (wParam)
        {
        case IDOK:
            // Get number of characters. 
            cchName = (WORD)SendDlgItemMessage(hDlg,
                IDE_NAMEEDIT,
                EM_LINELENGTH,
                (WPARAM)0,
                (LPARAM)0);
            if (cchName >= MAX_HEADING_LEN)
            {
                MessageBox(hDlg,
                    L"Too many characters.",
                    L"Error",
                    MB_OK);

                EndDialog(hDlg, TRUE);
                return FALSE;
            }
            else if (MAX_HEADING_LEN == 0)
            {
                MessageBox(hDlg,
                    L"Too long name!",
                    L"Error",
                    MB_OK);

                EndDialog(hDlg, TRUE);
                return FALSE;
            }

            // Put the number of characters into first word of buffer. 
            *((LPWORD)inputName) = cchName;

            // Get the characters. 
            SendDlgItemMessage(hDlg,
                IDE_NAMEEDIT,
                EM_GETLINE,
                (WPARAM)0,       // line 0 
                (LPARAM)inputName);

            // Null-terminate the string. 
            inputName[cchName] = 0;

            EndDialog(hDlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        return 0;
    }
    return FALSE;

    UNREFERENCED_PARAMETER(lParam);
}

bool listAppMainLoop() {
    isContinueFeature = true;
    //Receive running process(es)
    if (!la.receiveList(client))
        return false;
    {
        PostMessageA(chWnd, WM_COMMAND, CREATE_LISTAPP_WND, 0);
    }

    //Display 
    do {
        switch (commandNum) {
        case UPDATE_LIST_APP: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if (la.updateList(hwndListApp, chWnd, client))
                commandNum = 0;
            else
                return false;

            break;
        }

        case START_APP: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;

            la.hwndInput = (HWND)DialogBox(hInst,                   // application instance
                MAKEINTRESOURCE(IDD_INPUT), // dialog box resource
                hWnd,                          // owner window
                ProcessDialogProc                    // dialog box window procedure
            );

            client.Send(&inputName, sizeof(inputName), 0);
            if (GetLastError() != 0)
                return false;
            
            //result
            client.Receive(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if(commandNum == 0)
                PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Start successfully."));
            else if (commandNum == 1)
                PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Start failed."));

            Sleep(500); //wait for server to open
            commandNum = UPDATE_LIST_APP;
            break;
        }

        case END_APP: {
            LVITEM LvItem;
            WCHAR buffer[MAX_HEADING_LEN];
            memset(&LvItem, 0, sizeof(LvItem));
            LvItem.mask = LVIF_TEXT;
            LvItem.iSubItem = 1;
            LvItem.cchTextMax = sizeof(buffer) / sizeof(wchar_t);
            LvItem.pszText = buffer;

            int listid = SendMessage(hwndListApp, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if (listid != -1) {
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                LvItem.iItem = listid;           // choose item  
                if (ListView_GetItem(hwndListApp, &LvItem))
                    //Send thread id to server
                    client.Send(buffer, sizeof(buffer), 0);
                if (GetLastError() != 0)
                    return false;

                //Update list 
                Sleep(100); //wait for server to end
                commandNum = UPDATE_LIST_APP;
                client.Receive(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
            }
            else {
                commandNum = 1;
            }

            if (commandNum == 0)
                PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"End successfully."));
            else if (commandNum == 1)
                PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"End failed."));
            break;
        }

        case QUIT_FEATURE: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            isContinueFeature = false;
            break;
        }
        }
    } while (isContinueFeature);
}

bool listProcessMainLoop() {
    isContinueFeature = true;
    //Receive running process(es)
    if (!lp.receiveList(client))
        return false;
    {
        PostMessageA(chWnd, WM_COMMAND, CREATE_LISTPROCESS_WND, 0);
    }

    //Display 
    do {
        switch (commandNum) {
        case UPDATE_LIST: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if (lp.updateList(hwndListProcess, chWnd, client))
                commandNum = 0;
            else
                commandNum = 1;

            break;
        }

        case START_PROCESS: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;

            lp.hwndInput = (HWND)DialogBox(hInst,                   // application instance
                MAKEINTRESOURCE(IDD_INPUT), // dialog box resource
                hWnd,                          // owner window
                ProcessDialogProc                    // dialog box window procedure
            );

            client.Send(&inputName, sizeof(inputName), 0);
            if (GetLastError() != 0)
                return false;
            
            client.Receive(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if (commandNum == 0)
                PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Start successfully."));
            else if (commandNum == 1)
                PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Start failed."));

            Sleep(500); //wait for server
            commandNum = UPDATE_LIST;
            break;
        }

        case END_PROCESS: {

            LVITEM LvItem;
            WCHAR buffer[MAX_HEADING_LEN];
            memset(&LvItem, 0, sizeof(LvItem));
            LvItem.mask = LVIF_TEXT;
            LvItem.iSubItem = 1;
            LvItem.cchTextMax = sizeof(buffer) / sizeof(wchar_t);
            LvItem.pszText = buffer;

            int listid = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if (listid != -1) {
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                LvItem.iItem = listid;           // choose item  
                if (ListView_GetItem(hwndListProcess, &LvItem))
                    //Send thread id to server
                    client.Send(buffer, sizeof(buffer), 0);
                if (GetLastError() != 0)
                    return false;

                client.Receive(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false; 
                if (commandNum == 0)
                    PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"End successfully."));
                else if (commandNum == 1)
                    PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"End failed."));
                //Update list 
                Sleep(100); //wait for server
                commandNum = UPDATE_LIST;
            }
            else
                commandNum = 0;
            break;
        }

        case QUIT_FEATURE: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            isContinueFeature = false;
            break;
        }
        }
    } while (isContinueFeature);
    return true;
}

bool directoryTreeMainLoop() {
    isContinueFeature = true;
    int sz_rgDocHeadings;
    client.Receive(&sz_rgDocHeadings, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    
    for (int i = 0; i < sz_rgDocHeadings; ++i) {
        TCHAR heading[64];
        int level;
        Heading h1;
        client.Receive(&h1, sizeof(Heading), 0);
        if (GetLastError() != 0)
            return false;
        dt.g_rgDocHeadings.push_back(h1);
    }
    PostMessageA(chWnd, WM_COMMAND, CREATE_TREEVIEW_WND, 0);
    //hwndTreeView = dt.CreateATreeView(chWnd);

    do {
        switch (commandNum) {
        case GET_FOLDER: {
            commandNum = GET_FOLDER;
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            dt.updateDirectoryTree(client, hwndTreeView, NULL, NULL);
            commandNum = 0;
            break;
        }

        case DELETE_F: {
            HTREEITEM hTree = TreeView_GetSelection(hwndTreeView);

            if (hTree) {
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                wstring fullPath = dt.getPathOfSelectedItem(hwndTreeView, hTree);
                if (!dt.sendPath(fullPath, client))
                    return false;

                //Update directory tree
                client.Receive(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                if (commandNum == 0) {
                    //success, update directory tree
                    PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Delete successfully."));
                    commandNum = GET_FOLDER;
                    client.Send(&commandNum, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    commandNum = 0;
                    HTREEITEM parentHTree = TreeView_GetNextItem(hwndTreeView, hTree, TVGN_PARENT);
                    dt.updateDirectoryTree(client, hwndTreeView, parentHTree);
                }
                else if (commandNum == 1) {
                    PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Delete failed."));
                }
            }
            else
                commandNum = 0;
            break;
        }

        case CUT_F: {
            HTREEITEM srcItem = TreeView_GetSelection(hwndTreeView);
            if (srcItem) {
                WCHAR srcName[MAX_HEADING_LEN];
                dt.getTextOfSelectedItem(hwndTreeView, srcName); 
                wstring srcPath = dt.getPathOfSelectedItem(hwndTreeView);

                dt.hideButtons();
                ShowWindow(dt.pasteButton, SW_SHOWNORMAL);
                ShowWindow(dt.cancelButton, SW_NORMAL);
                
                do {
                    if (commandNum == PASTE_F || commandNum == CANCEL_F || commandNum == QUIT_FEATURE)
                        break;
                } while (1);
                
                if (commandNum == PASTE_F) {
                    commandNum = CUT_F;
                    client.Send(&commandNum, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    HTREEITEM dstItem = TreeView_GetSelection(hwndTreeView);
                    wstring dstPath = dt.getPathOfSelectedItem(hwndTreeView);

                    //send
                    if (!dt.sendPath(srcPath, client))
                        return false;
                    if (!dt.sendPath(srcName, client))
                        return false;
                    if (!dt.sendPath(dstPath, client))
                        return false;

                    client.Receive(&commandNum, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    if (commandNum == 0) {
                        PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Move successfully."));
                        commandNum = GET_FOLDER;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            return false;
                        dt.updateDirectoryTree(client, hwndTreeView, dstItem);
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            return false;
                        dt.updateDirectoryTree(client, hwndTreeView, srcItem);
                        commandNum = 0;
                    }
                    else if (commandNum == 1) {
                        //error
                        PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Move failed."));
                    }
                }
                else if (commandNum == CANCEL_F)
                    commandNum = 0;

                dt.showButtons();
                ShowWindow(dt.pasteButton, SW_HIDE);
                ShowWindow(dt.cancelButton, SW_HIDE);

            }
            else
                commandNum = 0;
            break;
        }

        case COPY_F: {
            HTREEITEM srcItem = TreeView_GetSelection(hwndTreeView);
            if (srcItem) {
                WCHAR srcName[MAX_HEADING_LEN];
                dt.getTextOfSelectedItem(hwndTreeView, srcName);
                wstring srcPath = dt.getPathOfSelectedItem(hwndTreeView);

                dt.hideButtons();
                ShowWindow(dt.pasteButton, SW_SHOWNORMAL);
                ShowWindow(dt.cancelButton, SW_NORMAL);

                do {
                    if (commandNum == PASTE_F || commandNum == CANCEL_F || commandNum == QUIT_FEATURE)
                        break;
                } while (1);

                if (commandNum == PASTE_F) {
                    commandNum = COPY_F;
                    HTREEITEM dstItem = TreeView_GetSelection(hwndTreeView);
                    wstring dstPath = dt.getPathOfSelectedItem(hwndTreeView);
                    client.Send(&commandNum, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    //send
                    if (!dt.sendPath(srcPath, client))
                        return false;
                    if (!dt.sendPath(srcName, client))
                        return false;
                    if (!dt.sendPath(dstPath, client))
                        return false;

                    client.Receive(&commandNum, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    if (commandNum == 0) {
                        //dt.AddItemToTree(hwndTreeView, chWnd, srcName, 2, dstItem);
                        PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Copy successfully."));
                        commandNum = GET_FOLDER;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            return false;
                        commandNum = 0;
                        dt.updateDirectoryTree(client, hwndTreeView, dstItem);
                    }
                    else if (commandNum == 1) {
                        //error
                        PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Copy failed."));
                    }
                }
                else if (commandNum == CANCEL_F)
                    commandNum = 0;

                dt.showButtons();
                ShowWindow(dt.pasteButton, SW_HIDE);
                ShowWindow(dt.cancelButton, SW_HIDE);
            }
            else
                commandNum = 0;
            break;
        }

        case SEND_F: {

            //Choose a file to send
            WCHAR srcPath[MAX_PATH] = { 0 };
            GetModuleFileName(NULL, srcPath, MAX_PATH);
            OPENFILENAME ofn;

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = NULL;
            ofn.lpstrFile = srcPath;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

            if (GetOpenFileName(&ofn))
            {
                //Choose location of server that will receive  
                dt.hideButtons();
                ShowWindow(dt.pasteButton, SW_SHOWNORMAL);
                ShowWindow(dt.cancelButton, SW_NORMAL);
                do {
                    if (commandNum == PASTE_F || commandNum == CANCEL_F || commandNum == QUIT_FEATURE)
                        break;
                } while (1);
                if (commandNum == PASTE_F) {
                    commandNum = SEND_F;
                    client.Send(&commandNum, sizeof(int), 0);

                    //Send
                    ifstream fl(srcPath, ios::binary);
                    fl.seekg(0, fl.end);
                    int length = fl.tellg();
                    client.Send(&length, sizeof(int), 0);
                    if (GetLastError() != 0)
                        return false;
                    if (length != 0) {
                        char* buffer = new char[length];
                        fl.seekg(0, fl.beg);
                        fl.read((char*)buffer, length);
                        fl.close();
                        client.Send(buffer, length, 0);
                        if (GetLastError() != 0)
                            return false;
                    }
                    PathStripPath(ofn.lpstrFile);
                    HTREEITEM dstItem = TreeView_GetSelection(hwndTreeView);
                    wstring dstPath = dt.getPathOfSelectedItem(hwndTreeView);
                    dstPath.resize(MAX_PATH);
                    StringCchCat(&dstPath[0], MAX_PATH, TEXT("\\"));
                    StringCchCat(&dstPath[0], MAX_PATH, ofn.lpstrFile);
                    if (!dt.sendPath(dstPath, client))
                        return false;
                    client.Receive(&commandNum, sizeof(int), 0);
                    if (commandNum == 0) {
                        PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Send successfully."));
                        commandNum = GET_FOLDER;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            return false;
                        commandNum = 0;
                        dt.updateDirectoryTree(client, hwndTreeView, dstItem);
                    }
                    else if (commandNum == 1) {
                        //error
                        PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Send failed."));
                    }
                }
                else if (commandNum == CANCEL_F)
                    commandNum = 0;

                dt.showButtons();
                ShowWindow(dt.pasteButton, SW_HIDE);
                ShowWindow(dt.cancelButton, SW_HIDE);
                }
            else {
                commandNum = 2;
            }
            break;
        }

        case RECEIVE_F: { //only apply to file
            HTREEITEM srcItem = TreeView_GetSelection(hwndTreeView);
            if (srcItem) {
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;

                //Get file want to download/receive
                //HTREEITEM hTree = TreeView_GetSelection(hwndTreeView);
                WCHAR srcName[MAX_HEADING_LEN];
                dt.getTextOfSelectedItem(hwndTreeView, srcName);
                wstring srcPath = dt.getPathOfSelectedItem(hwndTreeView);
                if (!dt.sendPath(srcPath, client))
                    return false;
                //dt.sendPath(srcName);

                vector<char> buffer;
                int length;
                client.Receive(&length, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;

                srcPath.resize(MAX_PATH);
                
                if (length == -1) {
                    commandNum = 1;
                }
                else if (length == 0) {
                    OPENFILENAME ofn;

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
                    ofn.hwndOwner = hWnd;
                    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                    ofn.lpstrFile = &srcPath[0];
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                    ofn.lpstrDefExt = L"txt";

                    if (GetSaveFileName(&ofn))
                    {
                        ofstream fout(ofn.lpstrFile, ios::binary);
                        fout.close();
                        if (fout) {
                            commandNum = 0;
                        }
                        else
                            commandNum = 1;
                    }
                    else
                        commandNum = 2;
                }
                else {
                    buffer.resize(length);
                    if (client.Receive(&buffer[0], length, 0)) {
                        if (GetLastError() != 0)
                            return false;
                        OPENFILENAME ofn;
                        //WCHAR szFileName[MAX_PATH] = "";
                        ZeroMemory(&ofn, sizeof(ofn));

                        ofn.lStructSize = sizeof(ofn); 
                        ofn.hwndOwner = hWnd;
                        ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                        ofn.lpstrFile = &srcPath[0];
                        ofn.nMaxFile = MAX_PATH;
                        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                        ofn.lpstrDefExt = L"txt";

                        if (GetSaveFileName(&ofn))
                        {
                            // Do something usefull with the filename stored in szFileName 
                            ofstream fout(ofn.lpstrFile, ios::binary);
                            if (fout) {
                                fout.flush();
                                fout.write((char*)&buffer[0], buffer.size());
                                fout.close();
                                commandNum = 0;
                            }
                            else
                                commandNum = 1;
                        }
                        else
                            commandNum = 2;
                    }
                }
            }
            else
                commandNum = 2;
            if (commandNum == 0)
                PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Receive successfully."));
            else if (commandNum == 1)
                PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Receive failed."));
            break;
        }

        case QUIT_FEATURE: {
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            isContinueFeature = false;
            break;
        }
        }

    } while (isContinueFeature);
    return true;
}

bool keyLogMainLoop() {
    isContinueFeature = true; 
    kl.isRefresh = false;
    kl.isSave = false;
    {
        ShowWindow(chWnd, SW_HIDE);
        PostMessageA(hWnd, WM_COMMAND, CREATE_LOG_WND, 0);
    }
    
    commandNum = CONTINUE_KEYLOG;
    char key[20];

    do {
        if (kl.isRefresh) {
            SetWindowText(hwndLog, TEXT(""));
            //SendMessage(kl.buttons[1], WM_KILLFOCUS, (WPARAM)hWnd, 0);
            kl.isRefresh = false;
        }

        if (kl.isSave) {
            kl.saveLogFile(hwndLog);
            kl.isSave = false;
        }

        switch (commandNum) {
        case STOP_KEYLOG: {
            client.Send(&commandNum, sizeof(int), 0);
            ShowWindow(kl.buttons[0], SW_HIDE);
            client.Send(&commandNum, sizeof(int), 0);
            ShowWindow(kl.continueButton, SW_NORMAL);

            break;
        }

        case CONTINUE_KEYLOG: {
            SendMessage(hwndLog, WM_SETFOCUS, (WPARAM)hwndLog, 0);
            client.Send(&commandNum, sizeof(int), 0);    
            if (GetLastError() != 0)
                return false;
            ShowWindow(kl.buttons[0], SW_NORMAL);
            ShowWindow(kl.continueButton, SW_HIDE);

            //receive from server
            client.Receive(key, sizeof(key), 0);
            if (GetLastError() != 0)
                return false;
            //set text
            const size_t cSize = sizeof(key) + 1; 
            wstring wc(cSize, L'#');
            mbstowcs(&wc[0], key, cSize);
            if (wc[0] != '\0')
                 kl.AppendText(hwndLog, &wc[0]);
            ShowWindow(hwndLog, SW_NORMAL);
            if (commandNum != QUIT_FEATURE && commandNum != STOP_KEYLOG)
                commandNum = CONTINUE_KEYLOG;
            break;
        }

        case QUIT_FEATURE: {
            client.Send(&commandNum, sizeof(int), 0);
            isContinueFeature = false;
            break;
        }
        }
    } while (isContinueFeature);
    return true;
}

unsigned __stdcall connectToServer(void* id) {
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(NULL);

    if (hModule != NULL)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            nRetCode = 1;
        }
        else
        {
            //DisplayCapture(hdc);
            //// TODO: code your application's behavior here.
            AfxSocketInit(NULL);
            ////CSocket client;

            client.Create();
            bool isSuccessful = client.Connect(serverIP, 4567);
            if (isSuccessful) {
                PostMessageA(hWnd, WM_COMMAND, SUCCESS_CONNECT, 0);
                EnableOriginalButton(buttons);
                do {
                    showOriginalButtons(buttons, IPinput);
                    EnableCloseButton(hWnd);
                    if (isQuit) {
                        commandNum = CLOSE_CONNECTION;
                        client.Send(&commandNum, sizeof(int), 0);
                        client.Close();
                        PostMessageA(hWnd, WM_COMMAND, SUCCESSFUL_FEATURE, (LPARAM)(L"Stop socket."));

                    }
                    switch (commandNum) {
                    case LIVE_SCREEN: {
                        ShowWindow(chWnd, SW_NORMAL);
                        DisableCloseButton(hWnd);
                        hideOriginalButtons(buttons, IPinput);
                        liveScreen.showButtons();
                        commandNum = LIVE_SCREEN;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            isQuit = true;
                        else {
                            liveScreenMainLoop();
                            liveScreen.hideButtons();
                            ShowWindow(liveScreen.continueButton, SW_HIDE);

                            DestroyWindow(hwndLiveScreen);
                        }
                        break;
                    }

                    case DIRECTORY_TREE: {
                        ShowWindow(chWnd, SW_NORMAL);
                        DisableCloseButton(hWnd);
                        hideOriginalButtons(buttons, IPinput);
                        dt.showButtons();
                        //DirectoryTree dt;
                        commandNum = DIRECTORY_TREE;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            isQuit = true;
                        else {
                            directoryTreeMainLoop();
                            dt.hideButtons();
                            ShowWindow(dt.pasteButton, SW_HIDE);
                            PostMessageA(chWnd, WM_COMMAND, DESTROY_TREE_VIEW, 0);
                            dt.g_rgDocHeadings.clear();
                        }

                        break;
                    }

                    case LIST_PROCESS: {
                        ShowWindow(chWnd, SW_NORMAL);
                        DisableCloseButton(hWnd);
                        hideOriginalButtons(buttons, IPinput);
                        lp.showButtons();
                        commandNum = LIST_PROCESS;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            isQuit = true;
                        else {
                            listProcessMainLoop();
                            lp.hideButtons();

                            {
                                PostMessageA(chWnd, WM_COMMAND, DESTROY_LIST_VIEW, 0);
                            }
                            lp.m_processes.clear();
                        }

                        break;
                    }

                    case LIST_APPLICATION: {
                        ShowWindow(chWnd, SW_NORMAL);
                        DisableCloseButton(hWnd);
                        hideOriginalButtons(buttons, IPinput);
                        la.showButtons();
                        commandNum = LIST_APPLICATION;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            isQuit = true;
                        else {
                            listAppMainLoop();
                            la.hideButtons();
                            {
                                PostMessageA(chWnd, WM_COMMAND, DESTROY_LIST_APP, 0);
                            }
                            la.openingWnds.clear();
                        }
                        //lp.mainLoop(client, hWnd, chWnd, hwndListProcess, hInst, commandNum);

                        break;
                    }

                    case KEYLOG: {
                        DisableCloseButton(hWnd);
                        hideOriginalButtons(buttons, IPinput);
                        kl.showButtons();
                        commandNum = KEYLOG;
                        client.Send(&commandNum, sizeof(int), 0);
                        if (GetLastError() != 0)
                            isQuit = true;
                        else {
                            keyLogMainLoop();
                            kl.hideButtons();
                            ShowWindow(kl.continueButton, SW_HIDE);
                            {
                                PostMessageA(chWnd, WM_COMMAND, DESTROY_LOG_VIEW, 0);
                            }
                            showOriginalButtons(buttons, IPinput);
                            ShowWindow(chWnd, SW_NORMAL);
                        }

                        break;
                    }
                    }
                } while (commandNum != CLOSE_CONNECTION);

                nRetCode = 0;
            } else {
                client.Close();
                EnableWindow(IPinput, TRUE);
                EnableWindow(buttons[0], TRUE);
                PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Connection failed."));
                return 1;
            }
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        PostMessageA(hWnd, WM_COMMAND, FAILED_FEATURE, (LPARAM)(L"Socket failed."));

        nRetCode = 1;
    }

    return nRetCode;
}


