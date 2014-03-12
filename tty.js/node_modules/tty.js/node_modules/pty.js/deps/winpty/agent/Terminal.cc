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

#include "Terminal.h"
#include "NamedPipe.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>

#define CSI "\x1b["

const int COLOR_ATTRIBUTE_MASK =
        FOREGROUND_BLUE |
        FOREGROUND_GREEN |
        FOREGROUND_RED |
        FOREGROUND_INTENSITY |
        BACKGROUND_BLUE |
        BACKGROUND_GREEN |
        BACKGROUND_RED |
        BACKGROUND_INTENSITY;

const int TERMINAL_BLACK = 0;
const int TERMINAL_RED   = 1;
const int TERMINAL_GREEN = 2;
const int TERMINAL_BLUE  = 4;
const int TERMINAL_WHITE = 7;

const int TERMINAL_FOREGROUND = 30;
const int TERMINAL_BACKGROUND = 40;

Terminal::Terminal(NamedPipe *output) :
    m_output(output),
    m_remoteLine(0),
    m_cursorHidden(false),
    m_remoteColor(-1)
{
}

void Terminal::reset(bool sendClearFirst, int newLine)
{
    if (sendClearFirst)
        m_output->write(CSI"1;1H"CSI"2J");
    m_remoteLine = newLine;
    m_cursorHidden = false;
    m_cursorPos = std::pair<int, int>(0, newLine);
    m_remoteColor = -1;
}

void Terminal::sendLine(int line, CHAR_INFO *lineData, int width)
{
    hideTerminalCursor();
    moveTerminalToLine(line);

    // Erase in Line -- erase entire line.
    m_output->write(CSI"2K");

    std::string termLine;
    termLine.reserve(width + 32);

    int length = 0;
    for (int i = 0; i < width; ++i) {
        int color = lineData[i].Attributes & COLOR_ATTRIBUTE_MASK;
        if (color != m_remoteColor) {
            int fore = 0;
            int back = 0;
            if (color & FOREGROUND_RED)   fore |= TERMINAL_RED;
            if (color & FOREGROUND_GREEN) fore |= TERMINAL_GREEN;
            if (color & FOREGROUND_BLUE)  fore |= TERMINAL_BLUE;
            if (color & BACKGROUND_RED)   back |= TERMINAL_RED;
            if (color & BACKGROUND_GREEN) back |= TERMINAL_GREEN;
            if (color & BACKGROUND_BLUE)  back |= TERMINAL_BLUE;

            char buffer[128];
            if (back == TERMINAL_BLACK) {
                if (fore == TERMINAL_WHITE) {
                    // Use the terminal's default colors.
                    sprintf(buffer, CSI"0");
                } else if (fore == TERMINAL_BLACK) {
                    // Attempt to hide the character, but some terminals won't
                    // hide it.  Is this an important case?
                    sprintf(buffer, CSI"0;8");
                } else {
                    sprintf(buffer, CSI"0;%d", TERMINAL_FOREGROUND + fore);
                }
                if (color & FOREGROUND_INTENSITY)
                    strcat(buffer, ";1");
            } else if (back == TERMINAL_WHITE) {
                // Use the terminal's inverted colors.
                if (fore == TERMINAL_BLACK) {
                    sprintf(buffer, CSI"0;7");
                } else if (fore == TERMINAL_WHITE) {
                    // Attempt to hide the character, but some terminals won't
                    // hide it.  Is this an important case?
                    sprintf(buffer, CSI"0;7;8");
                } else {
                    sprintf(buffer, CSI"0;7;%d", TERMINAL_BACKGROUND + fore);
                }
                // Don't worry about FOREGROUND_INTENSITY because with at least
                // one terminal (gnome-terminal 2.32.0), setting the Intensity
                // flag affects both foreground and background when Reverse
                // flag is also set.
            } else {
                sprintf(buffer, CSI"0;%d;%d",
                        TERMINAL_FOREGROUND + fore,
                        TERMINAL_BACKGROUND + back);
                if (color & FOREGROUND_INTENSITY)
                    strcat(buffer, ";1");
            }
            strcat(buffer, "m");
            termLine.append(buffer);
            length = termLine.size();
            m_remoteColor = color;
        }
        // TODO: Is it inefficient to call WideCharToMultiByte once per
        // character?
        char mbstr[16];
        int mblen = WideCharToMultiByte(CP_UTF8,
                                        0,
                                        &lineData[i].Char.UnicodeChar,
                                        1,
                                        mbstr,
                                        sizeof(mbstr),
                                        NULL,
                                        NULL);
        if (mblen <= 0) {
            mbstr[0] = '?';
            mblen = 1;
        }
        if (mblen == 1 && mbstr[0] == ' ') {
            termLine.push_back(' ');
        } else {
            termLine.append(mbstr, mblen);
            length = termLine.size();
        }
    }

    m_output->write(termLine.data(), length);
}

void Terminal::finishOutput(const std::pair<int, int> &newCursorPos)
{
    if (newCursorPos != m_cursorPos)
        hideTerminalCursor();
    if (m_cursorHidden) {
        moveTerminalToLine(newCursorPos.second);
        char buffer[32];
        sprintf(buffer, CSI"%dG"CSI"?25h", newCursorPos.first + 1);
        m_output->write(buffer);
        m_cursorHidden = false;
    }
    m_cursorPos = newCursorPos;
}

void Terminal::hideTerminalCursor()
{
    if (m_cursorHidden)
        return;
    m_output->write(CSI"?25l");
    m_cursorHidden = true;
}

void Terminal::moveTerminalToLine(int line)
{
    // Do not use CPL or CNL.  Konsole 2.5.4 does not support Cursor Previous
    // Line (CPL) -- there are "Undecodable sequence" errors.  gnome-terminal
    // 2.32.0 does handle it.  Cursor Next Line (CNL) does nothing if the
    // cursor is on the last line already.

    if (line < m_remoteLine) {
        // CUrsor Up (CUU)
        char buffer[32];
        sprintf(buffer, "\r"CSI"%dA", m_remoteLine - line);
        m_output->write(buffer);
        m_remoteLine = line;
    } else if (line > m_remoteLine) {
        while (line > m_remoteLine) {
            m_output->write("\r\n");
            m_remoteLine++;
        }
    } else {
        m_output->write("\r");
    }
}
