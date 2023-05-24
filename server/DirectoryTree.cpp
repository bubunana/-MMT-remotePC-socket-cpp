#include "DirectoryTree.h"
#include <strsafe.h>

vector<wstring> DirectoryTree::getChildDirectory(wstring path)
{
    vector<wstring> result;
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;
    TCHAR szDir[MAX_PATH];
    size_t length_of_arg;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;


    StringCchCopy(szDir, MAX_PATH, (STRSAFE_LPCWSTR)(&path[0]));
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    // Find the first file in the directory.

    hFind = FindFirstFile(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        //DisplayErrorBox(TEXT("FindFirstFile"));
        //return dwError;
        return result;
    }

    // List all the files in the directory with some info about them.

    do
    {
        wstring fileName;
        fileName.resize(sizeof(ffd.cFileName) / sizeof(wchar_t));
        memcpy(&fileName[0], ffd.cFileName, sizeof(ffd.cFileName));
        result.push_back(fileName);

    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError(); //Handle error

    FindClose(hFind);
    return result;
}

int DirectoryTree::DeleteDirectory(const wstring& refcstrRootDirectory,
    bool              bDeleteSubdirectories)
{
    bool            bSubdirectory = false;       // Flag, indicating whether
                                                 // subdirectories have been found
    HANDLE          hFile;                       // Handle to directory
    TCHAR     strFilePath[MAX_PATH];                 // Filepath
    TCHAR     strPattern[MAX_PATH];                  // Pattern
    WIN32_FIND_DATA FileInformation;             // File information

    StringCchCopy(strPattern, MAX_PATH, (STRSAFE_LPCWSTR)(&refcstrRootDirectory[0]));

    //Check if this is a file
    hFile = FindFirstFile(strPattern, &FileInformation);
    if (!(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        //Yes this is
        if (DeleteFile(strPattern) == FALSE)
            return ::GetLastError();
        return 0;
    }

    StringCchCat(strPattern, MAX_PATH, TEXT("\\*.*"));
    hFile = FindFirstFile(strPattern, &FileInformation);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!((FileInformation.cFileName[0] == '.' && FileInformation.cFileName[1] == '\0') || (FileInformation.cFileName[0] == '.' && FileInformation.cFileName[1] == '.' && FileInformation.cFileName[2] == '\0')))
            {
                memset(strFilePath, 0, MAX_PATH);
                StringCchCopy(strFilePath, MAX_PATH, (STRSAFE_LPCWSTR)(&refcstrRootDirectory[0]));
                StringCchCat(strFilePath, MAX_PATH, TEXT("\\"));
                StringCchCat(strFilePath, MAX_PATH, FileInformation.cFileName);

                if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (bDeleteSubdirectories)
                    {
                        // Delete subdirectory
                        int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
                        if (iRC)
                            return iRC;
                    }
                    else
                        bSubdirectory = true;
                }
                else
                {
                    // Set file attributes
                    if (SetFileAttributes(strFilePath,
                        FILE_ATTRIBUTE_NORMAL) == FALSE)
                        return ::GetLastError();

                    // Delete file
                    if (DeleteFile(strFilePath) == FALSE)
                        return ::GetLastError();
                }
            }
        } while (::FindNextFile(hFile, &FileInformation) == TRUE);

        // Close handle
        ::FindClose(hFile);

        DWORD dwError = ::GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
            return dwError;
        else
        {
            if (!bSubdirectory)
            {
                // Set directory attributes
                if (::SetFileAttributes(refcstrRootDirectory.c_str(),
                    FILE_ATTRIBUTE_NORMAL) == FALSE)
                    return ::GetLastError();

                // Delete directory
                if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
                    return ::GetLastError();
            }
        }
    }

    return 0;
}

bool DirectoryTree::receivePath(wstring& fullPath, CSocket& client) {
    int szPath;
    if (client.Receive(&szPath, sizeof(int), 0)) {
        if (GetLastError() != 0)
            return false;
        if (szPath == 0)
            return false;
        fullPath.resize(szPath / sizeof(wchar_t));
        client.Receive(&fullPath[0], szPath, 0);
        if (GetLastError() != 0)
            return false;
    }
    return true;
}

void DirectoryTree::findAvailableDrive() {

    WCHAR myDrives[64];

    if (GetLogicalDriveStringsW(ARRAYSIZE(myDrives) - 1, myDrives)) {
        for (LPWSTR drive = myDrives; *drive != 0; drive += 4)
        {
            //drive += '0';
            Heading h1;
            wcscpy_s(h1.tchHeading, drive);
            h1.tchLevel = 1;
            g_rgDocHeadings.push_back(h1);
        }
    }
    return;
}

bool DirectoryTree::SHCopy(LPCTSTR from, LPCTSTR to)
{
    SHFILEOPSTRUCT fileOp = { 0 };
    fileOp.wFunc = FO_COPY;
    TCHAR newFrom[MAX_PATH];
    _tcscpy_s(newFrom, from);
    newFrom[_tcsclen(from) + 1] = NULL;
    fileOp.pFrom = newFrom;
    TCHAR newTo[MAX_PATH];
    _tcscpy_s(newTo, to);
    newTo[_tcsclen(to) + 1] = NULL;
    fileOp.pTo = newTo;
    fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

    int result = SHFileOperation(&fileOp);

    return result == 0;
}


