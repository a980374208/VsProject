#include <windows.h>
#include <iostream>

bool SaveBitmap(HBITMAP hBitmap, LPCWSTR filename) {
    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
        return false;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;

    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = bmp.bmWidth;
    infoHeader.biHeight = bmp.bmHeight;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = bmp.bmBitsPixel;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = 0;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    DWORD dwBmpSize = bmp.bmWidthBytes * bmp.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    HDC hdc = GetDC(NULL);
    HDC hDCMem = CreateCompatibleDC(hdc);
    SelectObject(hDCMem, hBitmap);
    GetDIBits(hDCMem, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&infoHeader, DIB_RGB_COLORS);

    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&fileHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&infoHeader, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    CloseHandle(hFile);
    DeleteDC(hDCMem);
    ReleaseDC(NULL, hdc);

    return true;
}

int main() {
    // 定义要捕获的屏幕区域
    int x = 100;
    int y = 100;
    int width = 400;
    int height = 300;

    // 获取设备上下文
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // 创建一个与屏幕兼容的位图
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);

    // 从屏幕上拷贝区域到位图
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, x, y, SRCCOPY);

    // 保存位图到文件
    if (!SaveBitmap(hBitmap, L"capture.bmp")) {
        std::cerr << "Failed to save bitmap." << std::endl;
    }

    // 释放资源
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return 0;
}