// Copyright (c) 2011-2012 Ryan Prichard
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "Win32Console.h"
#include "AgentAssert.h"
#include "../shared/DebugClient.h"
#include <windows.h>

Win32Console::Win32Console()
{
    m_conin = GetStdHandle(STD_INPUT_HANDLE);
    m_conout = GetStdHandle(STD_OUTPUT_HANDLE);
}

Win32Console::~Win32Console()
{
    CloseHandle(m_conin);
    CloseHandle(m_conout);
}

HANDLE Win32Console::conin()
{
    return m_conin;
}

HANDLE Win32Console::conout()
{
    return m_conout;
}

HWND Win32Console::hwnd()
{
    return GetConsoleWindow();
}

void Win32Console::postCloseMessage()
{
    HWND h = hwnd();
    if (h != NULL)
        PostMessage(h, WM_CLOSE, 0, 0);
}

// A Windows console window can never be larger than the desktop window.  To
// maximize the possible size of the console in rows*cols, try to configure
// the console with a small font.
void Win32Console::setSmallFont()
{
    // Some of these types and functions are missing from the MinGW headers.
    // Others are undocumented.

    struct AGENT_CONSOLE_FONT_INFO {
        DWORD nFont;
        COORD dwFontSize;
    };

    struct AGENT_CONSOLE_FONT_INFOEX {
        ULONG cbSize;
        DWORD nFont;
        COORD dwFontSize;
        UINT FontFamily;
        UINT FontWeight;
        WCHAR FaceName[LF_FACESIZE];
    };

    typedef BOOL WINAPI SetConsoleFontType(
                HANDLE hOutput,
                DWORD dwFontIndex);
    typedef BOOL WINAPI GetCurrentConsoleFontType(
                HANDLE hOutput,
                BOOL bMaximize,
                AGENT_CONSOLE_FONT_INFO *pFontInfo);
    typedef BOOL WINAPI SetCurrentConsoleFontExType(
                HANDLE hConsoleOutput,
                BOOL bMaximumWindow,
                AGENT_CONSOLE_FONT_INFOEX *lpConsoleCurrentFontEx);
    typedef COORD WINAPI GetConsoleFontSizeType(
                HANDLE hConsoleOutput,
                DWORD nFont);

    HINSTANCE dll = LoadLibrary(L"kernel32.dll");
    ASSERT(dll != NULL);

    SetConsoleFontType *pSetConsoleFont =
            (SetConsoleFontType*)GetProcAddress(dll, "SetConsoleFont");
    GetCurrentConsoleFontType *pGetCurrentConsoleFont =
            (GetCurrentConsoleFontType*)GetProcAddress(dll, "GetCurrentConsoleFont");
    SetCurrentConsoleFontExType *pSetCurrentConsoleFontEx =
            (SetCurrentConsoleFontExType*)GetProcAddress(dll, "SetCurrentConsoleFontEx");
    GetConsoleFontSizeType *pGetConsoleFontSize =
            (GetConsoleFontSizeType*)GetProcAddress(dll, "GetConsoleFontSize");

    BOOL success;

    // The undocumented GetNumberOfConsoleFonts API reports that my Windows 7
    // system has 12 fonts on it.  Each font is really just a differently-sized
    // raster/Terminal font.  Font index 0 is the smallest font, so we want to
    // choose it.
    if (pGetConsoleFontSize == NULL) {
        // This API should exist even on Windows XP.
        trace("error: GetConsoleFontSize API is missing");
        return;
    }
    if (pGetCurrentConsoleFont == NULL) {
        // This API should exist even on Windows XP.
        trace("error: GetCurrentConsoleFont API is missing");
        return;
    }

    AGENT_CONSOLE_FONT_INFO fi;
    success = pGetCurrentConsoleFont(m_conout, FALSE, &fi);
    if (!success) {
        trace("error: GetCurrentConsoleFont failed");
        return;
    }
    COORD smallest = pGetConsoleFontSize(m_conout, 0);
    if (smallest.X == 0 || smallest.Y == 0) {
        trace("error: GetConsoleFontSize failed");
        return;
    }
    trace("font #0: X=%d Y=%d", smallest.X, smallest.Y);
    trace("current font: idx=%d X=%d Y=%d",
          (int)fi.nFont, fi.dwFontSize.X, fi.dwFontSize.Y);
    if (fi.dwFontSize.X <= smallest.X && fi.dwFontSize.Y <= smallest.Y)
        return;

    // First try to call the documented Vista API.
    if (pSetCurrentConsoleFontEx != NULL) {
        AGENT_CONSOLE_FONT_INFOEX fix = {0};
        fix.cbSize = sizeof(fix);
        fix.nFont = 0;
        fix.dwFontSize = smallest;
        success = pSetCurrentConsoleFontEx(m_conout, FALSE, &fix);
        trace("SetCurrentConsoleFontEx call %s",
              success ? "succeeded" : "failed");
        return;
    }

    // Then try to call the undocumented Windows XP API.
    //
    // Somewhat described here:
    // http://blogs.microsoft.co.il/blogs/pavely/archive/2009/07/23/changing-console-fonts.aspx
    //
    if (pSetConsoleFont != NULL) {
        success = pSetConsoleFont(m_conout, 0);
        trace("SetConsoleFont call %s", success ? "succeeded" : "failed");
        return;
    }

    trace("Not setting console font size -- "
          "neither SetConsoleFont nor SetCurrentConsoleFontEx API exists");
}

Coord Win32Console::bufferSize()
{
    // TODO: error handling
    CONSOLE_SCREEN_BUFFER_INFO info;
    memset(&info, 0, sizeof(info));
    if (!GetConsoleScreenBufferInfo(m_conout, &info)) {
        trace("GetConsoleScreenBufferInfo failed");
    }
    return info.dwSize;
}

SmallRect Win32Console::windowRect()
{
    // TODO: error handling
    CONSOLE_SCREEN_BUFFER_INFO info;
    memset(&info, 0, sizeof(info));
    if (!GetConsoleScreenBufferInfo(m_conout, &info)) {
        trace("GetConsoleScreenBufferInfo failed");
    }
    return info.srWindow;
}

void Win32Console::resizeBuffer(const Coord &size)
{
    // TODO: error handling
    if (!SetConsoleScreenBufferSize(m_conout, size)) {
        trace("SetConsoleScreenBufferSize failed");
    }
}

void Win32Console::moveWindow(const SmallRect &rect)
{
    // TODO: error handling
    if (!SetConsoleWindowInfo(m_conout, TRUE, &rect)) {
        trace("SetConsoleWindowInfo failed");
    }
}

void Win32Console::reposition(const Coord &newBufferSize,
                              const SmallRect &newWindowRect)
{
    // Windows has one API for resizing the screen buffer and a different one
    // for resizing the window.  It seems that either API can fail if the
    // window does not fit on the screen buffer.

    const SmallRect origWindowRect(windowRect());
    const SmallRect origBufferRect(Coord(), bufferSize());

    ASSERT(!newBufferSize.isEmpty());
    SmallRect bufferRect(Coord(), newBufferSize);
    ASSERT(bufferRect.contains(newWindowRect));

    SmallRect tempWindowRect = origWindowRect.intersected(bufferRect);
    if (tempWindowRect.width() <= 0) {
        tempWindowRect.setLeft(newBufferSize.X - 1);
        tempWindowRect.setWidth(1);
    }
    if (tempWindowRect.height() <= 0) {
        tempWindowRect.setTop(newBufferSize.Y - 1);
        tempWindowRect.setHeight(1);
    }

    // Alternatively, if we can immediately use the new window size,
    // do that instead.
    if (origBufferRect.contains(newWindowRect))
        tempWindowRect = newWindowRect;

    if (tempWindowRect != origWindowRect)
        moveWindow(tempWindowRect);
    resizeBuffer(newBufferSize);
    if (newWindowRect != tempWindowRect)
        moveWindow(newWindowRect);
}

Coord Win32Console::cursorPosition()
{
    // TODO: error handling
    CONSOLE_SCREEN_BUFFER_INFO info;
    memset(&info, 0, sizeof(info));
    if (!GetConsoleScreenBufferInfo(m_conout, &info)) {
        trace("GetConsoleScreenBufferInfo failed");
    }
    return info.dwCursorPosition;
}

void Win32Console::setCursorPosition(const Coord &coord)
{
    // TODO: error handling
    if (!SetConsoleCursorPosition(m_conout, coord)) {
        trace("SetConsoleCursorPosition failed");
    }
}

void Win32Console::writeInput(const INPUT_RECORD *ir, int count)
{
    // TODO: error handling
    DWORD dummy = 0;
    if (!WriteConsoleInput(m_conin, ir, count, &dummy)) {
        trace("WriteConsoleInput failed");
    }
}

bool Win32Console::processedInputMode()
{
    // TODO: error handling
    DWORD mode = 0;
    if (!GetConsoleMode(m_conin, &mode)) {
        trace("GetConsoleMode failed");
    }
    return (mode & ENABLE_PROCESSED_INPUT) == ENABLE_PROCESSED_INPUT;
}

void Win32Console::read(const SmallRect &rect, CHAR_INFO *data)
{
    // TODO: error handling
    SmallRect tmp(rect);
    if (!ReadConsoleOutput(m_conout, data, rect.size(), Coord(), &tmp)) {
        trace("ReadConsoleOutput failed [x:%d,y:%d,w:%d,h:%d]",
              rect.Left, rect.Top, rect.width(), rect.height());
    }
}

void Win32Console::write(const SmallRect &rect, const CHAR_INFO *data)
{
    // TODO: error handling
    SmallRect tmp(rect);
    if (!WriteConsoleOutput(m_conout, data, rect.size(), Coord(), &tmp)) {
        trace("WriteConsoleOutput failed");
    }
}
