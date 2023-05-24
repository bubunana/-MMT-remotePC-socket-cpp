#pragma once

#include "Heading.h"
#include "afxsock.h"
#include <vector>
#include <string>

using namespace std;

class DirectoryTree {
public:
    vector<Heading> g_rgDocHeadings;
public:
    //void accessDirectoryTree(CSocket& client);
    void findAvailableDrive();
    vector<wstring> getChildDirectory(wstring path);
    int DeleteDirectory(const wstring& refcstrRootDirectory,
        bool              bDeleteSubdirectories = true);
    bool receivePath(wstring& fullPath, CSocket& client);
    bool SHCopy(LPCTSTR from, LPCTSTR to);
};