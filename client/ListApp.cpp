#include "ListApp.h"
#include "GlobalVariables.h"
#include <chrono>
#include <thread>
#include <string>
#include "resource.h"

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

bool ListApp::receiveList(CSocket& client) {
    //Size
    int countProcess;
    client.Receive(&countProcess, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    //Content
    openingWnds.resize(countProcess);
    sleep_for(10ms); //time between update list?
    if (countProcess != 0) {
        client.Receive(&openingWnds[0], countProcess * sizeof(Wnd), 0);
        if (GetLastError() != 0)
            return false;
        else
            return true;
    }
}

bool ListApp::updateList(HWND hwndListProcess, HWND hwndParent, CSocket& client) {
    int listId = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, 0);
    while (listId != -1)
    {
        ListView_DeleteItem(hwndListProcess, listId);
        listId = SendMessage(hwndListProcess, LVM_GETNEXTITEM, -1, 0);
    }

    openingWnds.clear();
    if (!receiveList(client))
        return false;

    for (int i = 0; i < openingWnds.size(); i++)
    {
        AddItemToList(hwndListProcess, hwndParent, openingWnds[i].name, i,
            openingWnds[i].id);
    }

    ListView_SortItems(hwndListProcess, 0, 0);

    return true;
}

HWND ListApp::CreateAListApp(HWND hwndParent, HINSTANCE hInst) {
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

    if (!InitListAppItems(hwndLV, hwndParent))
    {
        DestroyWindow(hwndLV);
        return FALSE;
    }

    ListView_SortItems(hwndLV, 0, 0);

    return hwndLV;
}

BOOL ListApp::InitListAppItems(HWND hwndLV, HWND hwndParent) {
    LVCOLUMN LvCol;
    wchar_t buffer[50];
    LVITEM LvItem;

    // Here we put the info on the Coulom headers
    // this is not data, only name of each header we like
    memset(&LvCol, 0, sizeof(LvCol));                  // Zero Members
    std::memcpy(buffer, L"Name of running applications' window", sizeof(buffer));
    LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    // Type of mask
    LvCol.cx = 30;                                   // width between each coloum
    LvCol.pszText = buffer;                            // First Header Text
    LvCol.cx = 500;                                   // width of column

    ////Extended styles
    //SendMessage(hwndLV, LVM_SETEXTENDEDListProcessSTYLE,
    //    0, LVS_EX_FULLROWSELECT); // Set style

    // Insert Columns as much as we want
    SendMessage(hwndLV, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol); // Insert/Show the coloum
    std::memcpy(buffer, L"Process ID", sizeof(buffer));                           // Next coloum
    SendMessage(hwndLV, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol); // Insert/Show the coloum

    for (int i = 0; i < openingWnds.size(); i++)
    {
        AddItemToList(hwndLV, hwndParent, openingWnds[i].name, i, openingWnds[i].id);
    }

    return TRUE;
}

LVITEM ListApp::AddItemToList(HWND hwndLV, HWND hwndParent, LPTSTR lpszItem, int index, DWORD id) {
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

    LvItem.iItem = index;           // choose item  
    LvItem.iSubItem = 2;        // Put in second coluom
    LvItem.pszText = buffer;  // Text to display 
    SendMessage(hwndLV, LVM_SETITEM, 0, (LPARAM)&LvItem); // Send to the ListProcess

    return LvItem;
}

void ListApp::hideButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_HIDE);
}

void ListApp::showButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
}
