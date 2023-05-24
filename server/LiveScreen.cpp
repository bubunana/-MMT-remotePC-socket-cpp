#include "LiveScreen.h"
#include "resource.h"

#include <commdlg.h>
#include <fstream>
#include <thread>
#include <objidl.h>

#include <gdiplus.h>
#include <gdiplusheaders.h>
#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;

bool LiveScreen::gdiscreen() {
    using namespace Gdiplus;
    IStream* istream;
    HRESULT res = CreateStreamOnHGlobal(NULL, true, &istream);
    {
        HDC scrdc, memdc;
        HBITMAP membit;
        scrdc = ::GetDC(0);
        int Height = GetSystemMetrics(SM_CYSCREEN);
        int Width = GetSystemMetrics(SM_CXSCREEN);
        /*int Height = 540;
        int Width = 960;*/
        memdc = CreateCompatibleDC(scrdc);
        membit = CreateCompatibleBitmap(scrdc, Width, Height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);

        BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

        Bitmap bitmap(membit, NULL);

        if (CreateStreamOnHGlobal(NULL, TRUE, &istream) != 0)
            return false;

        CLSID clsid_bmp;
        if (CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid_bmp) != 0)
            return false;
        Gdiplus::Status status = bitmap.Save(istream, &clsid_bmp);
        if (status != Gdiplus::Status::Ok)
            return false;

        //get memory handle associated with istream
        HGLOBAL hg = NULL;
        if (GetHGlobalFromStream(istream, &hg) != S_OK)
            return 0;

        //copy IStream to buffer
        int bufsize = GlobalSize(hg);
        data.resize(bufsize);

        //lock & unlock memory
        LPVOID pimage = GlobalLock(hg);
        if (!pimage)
            return false;
        std::memcpy(&data[0], pimage, bufsize);
        GlobalUnlock(hg);

        istream->Release();

        DeleteObject(memdc);
        DeleteObject(membit);
        ::ReleaseDC(0, scrdc);

    }
    return true;
}