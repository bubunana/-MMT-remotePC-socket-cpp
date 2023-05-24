#include "DirectoryTree.h"
#include "resource.h"
#include "GlobalVariables.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <strsafe.h>

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace std;

bool DirectoryTree::showAvailableDrive() {
    WCHAR myDrives[64];
    WCHAR volumeName[MAX_PATH];
    WCHAR fileSystemName[MAX_PATH];
    DWORD serialNumber, maxComponentLen, fileSystemFlags;
    UINT driveType;

    if (!GetLogicalDriveStringsW(ARRAYSIZE(myDrives) - 1, myDrives))
    {
        return false;
    }
    else
    {
        for (LPWSTR drive = myDrives; *drive != 0; drive += 4)
        {
            g_rgDocHeadings.push_back(Heading{ *drive, 1 });
        }
    }

    return true;
}

HWND DirectoryTree::CreateATreeView(HWND hwndParent, HINSTANCE hInst) {
    RECT rcClient; // dimensions of client area
    HWND hwndTV; // handle to tree-view control

    // Ensure that the common control DLL is loaded.
    InitCommonControls();

    // Get the dimensions of the parent window's client area, and create the tree view control
    GetClientRect(hwndParent, &rcClient);
    hwndTV = CreateWindowEx(0,
        WC_TREEVIEW,
        TEXT("Tree View"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
        0,
        0,
        rcClient.right,
        rcClient.bottom,
        hwndParent,
        (HMENU)IDC_TREE,
        hInst,
        NULL);
    // Initialize the image list, and add items to the control. 
    // InitTreeViewImageLists and InitTreeViewItems are application- 
    // defined functions, shown later. 
    if (!(InitTreeViewItems(hwndTV, hwndParent)))
    {
        DestroyWindow(hwndTV);
        return FALSE;
    }
    return hwndTV;
}

BOOL DirectoryTree::InitTreeViewItems(HWND hwndTV, HWND hwndParent) {
    HTREEITEM hti;

    // g_rgDocHeadings is an application-defined global array of 
    // the following structures: 
    //     typedef struct 
    //       { 
    //         TCHAR tchHeading[MAX_HEADING_LEN]; 
    //         int tchLevel; 
    //     } Heading; 
    

    for (int i = 0; i < g_rgDocHeadings.size(); i++)
    {
        // Add the item to the tree-view control. 

        hti = AddItemToTree(hwndTV, hwndParent, g_rgDocHeadings[i].tchHeading,
            g_rgDocHeadings[i].tchLevel, NULL);

        if (hti == NULL)
            return FALSE;
    }

    return TRUE;
}

HTREEITEM DirectoryTree::AddItemToTree(HWND hwndTV, HWND hwndParent, LPTSTR lpszItem, int nLevel, HTREEITEM hTParent) {
    TVITEM tvi;
    TVINSERTSTRUCT tvins;
    static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;
    static HTREEITEM hPrevRootItem = NULL;
    static HTREEITEM hPrevLev2Item = NULL;
    HTREEITEM hti;
    tvi.mask = TVIF_TEXT | TVIF_IMAGE
        | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    // Set the text of the item.
    tvi.pszText = lpszItem;
    tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);
    // Assume the item is not a parent item, so give it a
    // document image.
    tvi.iImage = 0;
    tvi.iSelectedImage = 0;
    // Save the heading level in the item's application-defined
    // data area.
    //tvi.lParam = (LPARAM)nLevel;
    tvi.lParam = (LPARAM)nLevel;
    tvins.item = tvi;
    tvins.hInsertAfter = hPrev;
    // Set the parent item based on the specified level.
    if (hTParent) {
        tvins.hParent = hTParent;
    }
    else {
        if (nLevel == 1)
            tvins.hParent = TVI_ROOT;
        else if (nLevel == 2)
            tvins.hParent = hPrevRootItem;
        else
            tvins.hParent = hPrevLev2Item;
    }
    // Add the item to the tree-view control.
    hPrev = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM,
        0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
    if (hPrev == NULL)
        return NULL;
    // Save the handle to the item.
    if (nLevel == 1)
        hPrevRootItem = hPrev;
    else if (nLevel == 2)
        hPrevLev2Item = hPrev;
    // The new item is a child item. Give the parent item a
    // closed folder bitmap to indicate it now has child items.
    if (nLevel > 1)
    {
        hti = TreeView_GetParent(hwndTV, hPrev);
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = hti;
        tvi.iImage = 0;
        tvi.iSelectedImage = 0;
        TreeView_SetItem(hwndTV, &tvi);
    }
    return hPrev;
}

bool DirectoryTree::sendPath(wstring fullPath, CSocket& client) {
    int szPath = fullPath.length() * sizeof(wchar_t);
    client.Send(&szPath, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    if (szPath == 0)
        return false;
    client.Send(&fullPath[0], szPath, 0);
    if (GetLastError() != 0)
        return false;
    else
        return true;
}

bool DirectoryTree::updateDirectoryTree(CSocket& client, HWND hwndTreeView, HTREEITEM hTree, HWND hwnd) {
    if (!hTree)
        hTree = TreeView_GetSelection(hwndTreeView);
    while (HTREEITEM childHTree = TreeView_GetNextItem(hwndTreeView, hTree, TVGN_CHILD)) {
        TreeView_DeleteItem(hwndTreeView, childHTree);
    }
    //if (!checkIfSelectedItemHaveChild()) {
    WCHAR szItem[MAX_HEADING_LEN];
    //getTextOfSelectedItem(szItem);
    wstring fullPath = getPathOfSelectedItem(hwndTreeView, hTree);
    vector<wchar_t> fullPathVector(fullPath.begin(), fullPath.end());

    if (!sendPath(fullPath, client))
        return false;

    int szDirec;
    wchar_t fileName[MAX_HEADING_LEN];
    vector<wstring>childDirectory;
    int numDirec;
    
    sleep_for(1ms);
    client.Receive(&numDirec, sizeof(int), 0);
    if (GetLastError() != 0)
        return false;
    
    for (int i = 0; i < numDirec; ++i) {
        wstring fileName;
        fileName.resize(MAX_HEADING_LEN);
        client.Receive(&fileName[0], MAX_HEADING_LEN * sizeof(wchar_t), 0);
        if (GetLastError() != 0)
            return false;
        childDirectory.push_back(fileName);
    }
    wstring drop1 = L".";
    wstring drop2 = L"..";
    drop1.resize(MAX_HEADING_LEN);
    drop2.resize(MAX_HEADING_LEN);

    for (int i = 0; i < childDirectory.size(); ++i) {
        //int pos = childDirectory[i].find(L".", childDirectory[i].size() - 3);
        if (childDirectory[i].compare(drop1) != 0) {
            AddItemToTree(hwndTreeView, hwnd, &childDirectory[i][0], 2, hTree);
        }
    }
    return true;
    //}
}

wstring DirectoryTree::getPathOfSelectedItem(HWND hwndTreeView, HTREEITEM hTree) {
    //Get selected item
    if (!hTree)
        hTree = TreeView_GetSelection(hwndTreeView);
    TVITEM tvi;
    if (!hTree)
        return 0;
    WCHAR szItem[100][MAX_HEADING_LEN];
    int count = 0;
    ZeroMemory(&tvi, sizeof(tvi));
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hTree;
    tvi.cchTextMax = 128;
    tvi.pszText = szItem[count];
    if (!TreeView_GetItem(hwndTreeView, &tvi)) {
        return NULL;
    }

    wostringstream wos;
    ++count;
    HTREEITEM hTParent;
    while ((hTParent = TreeView_GetParent(hwndTreeView, hTree)) && count < 100) {
        //ZeroMemory(&tvi, sizeof(tvi));
        TVITEM tvi1;
        ZeroMemory(&tvi1, sizeof(tvi1));
        tvi1.mask = TVIF_TEXT;
        tvi1.hItem = hTParent;
        tvi1.cchTextMax = 128;
        tvi1.pszText = szItem[count];
        if (!TreeView_GetItem(hwndTreeView, &tvi1)) {
            break;
        }
        hTree = hTParent;
        ++count;

    }
    wos << szItem[--count];
    while (count > 1) {
        wos << szItem[--count] << "\\";
    }
    wos << szItem[--count];
    wstring fullPath(wos.str());
    return fullPath;
}

bool DirectoryTree::checkIfSelectedItemHaveChild(HWND hwndTreeView, HTREEITEM hTree) {
    hTree = TreeView_GetSelection(hwndTreeView);
    if (TreeView_GetChild(hwndTreeView, hTree))
        return true;
    return false;
}

WCHAR* DirectoryTree::getTextOfSelectedItem(HWND hwndTreeView, WCHAR szItem[MAX_HEADING_LEN]) {
    HTREEITEM hTree = TreeView_GetSelection(hwndTreeView);
    TVITEM tvi;
    if (!hTree)
        return NULL;
    //WCHAR szItem[MAX_HEADING_LEN];
    int count = 0;
    //WCHAR path[1024];
    ZeroMemory(&tvi, sizeof(tvi));
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hTree;
    tvi.cchTextMax = 128;
    tvi.pszText = szItem;
    if (!TreeView_GetItem(hwndTreeView, &tvi)) {
        return NULL;
    }
    return szItem;
}

void DirectoryTree::hideButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_HIDE);
}

void DirectoryTree::showButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
}

