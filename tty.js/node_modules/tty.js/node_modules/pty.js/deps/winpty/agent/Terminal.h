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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <windows.h>
#include "Coord.h"
#include <utility>

class NamedPipe;

class Terminal
{
public:
    explicit Terminal(NamedPipe *output);
    void reset(bool sendClearFirst, int newLine);
    void sendLine(int line, CHAR_INFO *lineData, int width);
    void finishOutput(const std::pair<int, int> &newCursorPos);

private:
    void hideTerminalCursor();
    void moveTerminalToLine(int line);

private:
    NamedPipe *m_output;
    int m_remoteLine;
    bool m_cursorHidden;
    std::pair<int, int> m_cursorPos;
    int m_remoteColor;
};

#endif // TERMINAL_H
