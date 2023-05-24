#include "LiveScreen.h"
#include "resource.h"

#include <commdlg.h>
#include <fstream>
#include <thread>

#include <gdiplus.h>
#include <gdiplusheaders.h>
#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

bool LiveScreen::captureScreen() {
    OPENFILENAME ofn;
    WCHAR srcPath[MAX_PATH];
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Bitmap (*.bmp)\0*.bmp\0PNG (*.png)\0*.png\0JPEG (*.jpeg)\0*.jpeg\0";
    GetCurrentDirectory(MAX_PATH, srcPath);
    ofn.lpstrFile = srcPath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"bmp";

    if (GetSaveFileName(&ofn))
    {
        //switch (ofn.nFilterIndex) {
        //    case 0: {
                // Do something usefull with the filename stored in szFileName 
        ofstream fout(ofn.lpstrFile, ios::binary);
        if (fout) {
            fout.flush();
            fout.write((char*)bmp_data.data(), bmp_data.size());
            fout.close();
        }
    }

    return true;
}

BITMAPFILEHEADER* bmfh = nullptr;
BITMAPINFOHEADER* bmih = nullptr;
BITMAPINFO* bmi = nullptr;
void* bits = nullptr;
HBITMAP hbmp;

int  LiveScreen::DisplayCapture(HDC hDC, vector<BYTE> data, HWND hwndLiveScreen, int Width, int Height) {
    hDC = GetDC(hwndLiveScreen);

    if ((!hDC))
        return 1;

    bmfh = (BITMAPFILEHEADER*)&data[0];

    bmih = (BITMAPINFOHEADER*)(&data[0] + sizeof(BITMAPFILEHEADER));
 
    bmi = (BITMAPINFO*)(&data[0] + sizeof(BITMAPFILEHEADER));

    bits = (void*)(&data[0] + bmfh->bfOffBits);

    //if (!bmfh || !bmih || !bmi || !bits)
    //    return 1;

    hbmp = CreateDIBitmap(hDC, bmih, CBM_INIT, bits, bmi, DIB_RGB_COLORS);

    if (!hbmp)
        return 0;

    {
        Graphics graphics(hDC);
        Bitmap bitmap(hbmp, nullptr);

        Bitmap newBitmap(Width, Height);
        Graphics gdraw(&newBitmap);
        static float xscale = (float)Width / bmih->biWidth;
        static float yscale = (float)Height / bmih->biHeight;
        //if (gdraw.ScaleTransform(xscale, yscale) != 0)
        //    return 1;

        //if (gdraw.DrawImage(&bitmap, 0, 0) != 0)
        //    return 1;
        gdraw.ScaleTransform(xscale, yscale);
        gdraw.DrawImage(&bitmap, 0, 0);
        graphics.DrawImage(&newBitmap, 0, 0);
    }

    DeleteObject(hbmp);
    ::ReleaseDC(0, hDC);

    return 0;
}


void LiveScreen::hideButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_HIDE);
}

void LiveScreen::showButtons() {
    for (int i = 0; i < size(buttons); ++i)
        ShowWindow(buttons[i], SW_SHOWNORMAL);
}
