#pragma once
#include "stdafx.h"
#include "afxsock.h"
#include "framework.h"
#include <vector>
#include "Heading.h"

using namespace std;

class DirectoryTree {
public:
    vector<Heading> g_rgDocHeadings;
    //HWND hwndTreeView;
    HWND buttons[6];
    HWND pasteButton;
    HWND cancelButton;
public:
    bool showAvailableDrive();
    bool updateDirectoryTree(CSocket& client, HWND hwndTreeView, HTREEITEM hTree = NULL, HWND hwnd = NULL);
    ////utils
    HWND CreateATreeView(HWND hwndParent, HINSTANCE hInst);
    BOOL InitTreeViewItems(HWND hwndTV, HWND hwndParent);
    HTREEITEM AddItemToTree(HWND hwndTV, HWND hwndParent, LPTSTR lpszItem, int nLevel, HTREEITEM hTParent);
    wstring getPathOfSelectedItem(HWND hwndTreeView, HTREEITEM hTree = NULL);
    bool checkIfSelectedItemHaveChild(HWND hwndTreeView, HTREEITEM hTree);
    WCHAR* getTextOfSelectedItem(HWND hwndTreeView, WCHAR szItem[MAX_HEADING_LEN]);
    void hideButtons();
    void showButtons();
    bool sendPath(wstring fullPath, CSocket& client);
};