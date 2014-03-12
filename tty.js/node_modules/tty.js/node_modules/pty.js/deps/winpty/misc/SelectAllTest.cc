#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <windows.h>

const int SC_CONSOLE_MARK = 0xFFF2;
const int SC_CONSOLE_SELECT_ALL = 0xFFF5;

CALLBACK DWORD pausingThread(LPVOID dummy)
{
    HWND hwnd = GetConsoleWindow();
    while (true) {
        SendMessage(hwnd, WM_SYSCOMMAND, SC_CONSOLE_SELECT_ALL, 0);
        SendMessage(hwnd, WM_CHAR, 27, 0x00010001);
    }
}

int main()
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(out, &info);
    COORD initial = info.dwCursorPosition;

    CreateThread(NULL, 0,
                 pausingThread, NULL,
                 0, NULL);

    while (true) {
        GetConsoleScreenBufferInfo(out, &info);
        if (memcmp(&info.dwCursorPosition, &initial, sizeof(COORD)) != 0) {
            printf("cursor moved to [%d,%d]\n", 
                   info.dwCursorPosition.X,
                   info.dwCursorPosition.Y);
            GetConsoleScreenBufferInfo(out, &info);
            initial = info.dwCursorPosition;
        }
    }
    return 0;
}
