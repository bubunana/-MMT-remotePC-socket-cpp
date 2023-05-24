#include "ListProcess.h"
#include "GlobalVariables.h"
#include <chrono>
#include <thread>
#include <string>
#include "resource.h"

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

bool ListProcess::receiveList(CSocket &client) {
    //Size
    int countProcess;
    client.Receive(&countProcess, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    //Content
    m_processes.resize(countProcess);
    sleep_for(10ms); //time between update list?
    if (countProcess != 0) {
        client.Receive(&m_processes[0], countProcess * sizeof(Process), 0);
        if (GetLastError() != 0)
            return false;
        else
            return true;
    }
}

bool ListProcess::updateList(HWND hwndListProcess, HWND hwndParent, CSocket& client) {
    int listId = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, 0);
    while (listId != -1)
    {
        ListView_DeleteItem(hwndListProcess, listId);
        listId = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, 0);
    }

    m_processes.clear();
    if (!receiveList(client))
        return false;

    for (int i = 0; i < m_processes.size(); i++)
    {
        AddItemToList(hwndListProcess, hwndParent, m_processes[i].name, i,
            m_processes[i].id, m_processes[i].threadCount);
        //ListProcess_GetItem(hwndListProcess, 0);
        //ListProcess_EnsureVisible(hwndListProcess, 0, TRUE);
    }

    ListView_SortItems(hwndListProcess, 0, 0);

    return true;
}

HWND ListProcess::CreateAListProcess(HWND hwndParent, HINSTANCE hInst) {
    RECT rcClient; // dimensions of client area
    HWND hwndLV; // handle to list-view control

    // Ensure that the common control DLL is loaded.
    InitCommonControls();

    // Get the dimensions of the parent window's client area, and create the list view control
    GetClientRect(hwndParent, &rcClient);
    hwndLV = CreateWindowEx(0,
        WC_LISTVIEW,
        TEXT("List View"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT | LVS_EDITLABELS,
        //WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
        0,
        0,
        rcClient.right,
        rcClient.bottom,
        hwndParent,
        (HMENU)IDC_LIST,
        hInst,
        NULL);
    //CreateWindow(TEXT("static"), TEXT("!!!!!!!!!!!!!!!!!"), WS_VISIBLE | WS_CHILD,
    //    0, 0, 0x42, 0x42, hwndLV, (HMENU)1, NULL, NULL);
    // Initialize the image list, and add items to the control. 
    if (!InitListProcessItems(hwndLV, hwndParent))
    {
        DestroyWindow(hwndLV);
        return FALSE;
    }

    ListView_SortItems(hwndLV, 0, 0);

    ////Extended styles
    //SendMessage(hwndLV, LVM_SETEXTENDEDListProcessSTYLE,
    //    0, LVS_EX_FULLROWSELECT); // Set style

    return hwndLV;
}

BOOL ListProcess::InitListProcessItems(HWND hwndLV, HWND hwndParent) {
    LVCOLUMN LvCol;
    wchar_t buffer[MAX_COLUMN_LENGTH];
    LVITEM LvItem;

    // Here we put the info on the Coulom headers
    // this is not data, only name of each header we like
    memset(&LvCol, 0, sizeof(LvCol));                  // Zero Members
    std::memcpy(buffer, L"Name of process", sizeof(buffer));
    LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    // Type of mask
    LvCol.cx = 30;                                   // width between each coloum
    LvCol.pszText = buffer;                            // First Header Text
    LvCol.cx = 350;                                   // width of column

    ////Extended styles
    //SendMessage(hwndLV, LVM_SETEXTENDEDListProcessSTYLE,
    //    0, LVS_EX_FULLROWSELECT); // Set style

    // Insert Columns as much as we want
    SendMessage(hwndLV, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol); // Insert/Show the coloum
    std::memcpy(buffer, L"Process ID", sizeof(buffer));                           // Next coloum
    std::memcpy(buffer, L"Process ID", sizeof(buffer));                           // Next coloum
    SendMessage(hwndLV, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol); // Insert/Show the coloum
    std::memcpy(buffer, L"Thread count", sizeof(buffer));                           //
    SendMessage(hwndLV, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol); // ...

    for (int i = 0; i < m_processes.size(); i++)
    {
        AddItemToList(hwndLV, hwndParent, m_processes[i].name, i,
            m_processes[i].id, m_processes[i].threadCount);
    }

    return TRUE;
}

LVITEM ListProcess::AddItemToList(HWND hwndLV, HWND hwndParent, LPTSTR lpszItem, int index, DWORD id, DWORD threadCount) {
    LVITEM LvItem;
    wchar_t buffer[MAX_HEADING_LEN];
    wstring temp;

    //Add name
    std::memcpy(buffer, lpszItem, sizeof(buffer));
    LvItem.mask = LVIF_TEXT;
    LvItem.iItem = index;           // choose item  
    LvItem.iSubItem = 0;        // Put in first coluom
    LvItem.cchTextMax = sizeof(buffer) / sizeof(wchar_t);  // Text to display 
    LvItem.pszText = buffer;  // Text to display 
    SendMessage(hwndLV, LVM_INSERTITEM, 0, (LPARAM)&LvItem); // Send to the ListProcess

    //Add ID
    temp = to_wstring(id);
    if (temp.size() != 0)
        std::memcpy(buffer, &temp[0], sizeof(buffer));
    else
        temp = L"";
    LvItem.iItem = index;           // choose item  
    LvItem.iSubItem = 1;        // Put in first coluom
    LvItem.pszText = buffer;  // Text to display 
    SendMessage(hwndLV, LVM_SETITEM, 0, (LPARAM)&LvItem); // Send to the ListProcess

    //Add thread count
    temp = to_wstring(threadCount);
    if (temp.size() != 0)
        std::memcpy(buffer, &temp[0], sizeof(buffer));
    else
        temp = L"";
    LvItem.iItem = index;           // choose item  
    LvItem.iSubItem = 2;        // Put in second coluom
    LvItem.pszText = buffer;  // Text to display 
    SendMessage(hwndLV, LVM_SETITEM, 0, (LPARAM)&LvItem); // Send to the ListProcess

    return LvItem;
}

void ListProcess::hideButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_HIDE);
}

void ListProcess::showButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
}

//void ListProcess::mainLoop(CSocket& client, HWND hWnd, HWND chWnd, HWND hwndListProcess, HINSTANCE hInst, int& commandNum) {
//    isContinueFeature = true;
//    //Receive running process(es)
//    receiveList(client);
//    //lp.sortAlphabet(); 
//    {
//        PostMessageA(chWnd, WM_COMMAND, 4001, 0);
//    }
//
//    //Display 
//    do {
//        switch (commandNum) {
//        case UPDATE_LIST: {
//            client.Send(&commandNum, sizeof(int), 0);
//            if (updateList(hwndListProcess, chWnd, client))
//                commandNum = 0;
//            else
//                commandNum = 1;
//
//            break;
//        }
//
//        case START_PROCESS: {
//            client.Send(&commandNum, sizeof(int), 0);
//
//            hwndInput = (HWND)DialogBox(hInst,                   // application instance
//                MAKEINTRESOURCE(IDD_INPUT), // dialog box resource
//                hWnd,                          // owner window
//                ProcessDialogProc                    // dialog box window procedure
//            );
//
//            client.Send(&inputName, sizeof(inputName), 0);
//            client.Receive(&commandNum, sizeof(int), 0);
//
//
//            commandNum = UPDATE_LIST;
//            break;
//        }
//
//        case END_PROCESS: {
//            client.Send(&commandNum, sizeof(int), 0);
//            LVITEM LvItem;
//            WCHAR buffer[MAX_HEADING_LEN];
//            memset(&LvItem, 0, sizeof(LvItem));
//            LvItem.mask = LVIF_TEXT;
//            LvItem.iSubItem = 1;
//            LvItem.cchTextMax = sizeof(buffer) / sizeof(wchar_t);
//            LvItem.pszText = buffer;
//
//            int listid = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
//            if (listid) {
//                LvItem.iItem = listid;           // choose item  
//                if (!ListProcess_GetItem(hwndListProcess, &LvItem))
//                    return;
//                else {
//                    //Send thread id to server
//                    client.Send(buffer, sizeof(buffer), 0);
//
//                }
//            }
//
//            //Update list 
//            commandNum = UPDATE_LIST;
//            break;
//        }
//
//        case QUIT_FEATURE: {
//            client.Send(&commandNum, sizeof(int), 0);
//            hideButtons();
//            {
//                PostMessageA(chWnd, WM_COMMAND, DESTROY_LIST_VIEW, 0);
//            }
//            m_processes.clear();
//            isContinueFeature = false;
//            break;
//        }
//        }
//    } while (isContinueFeature);
//}
