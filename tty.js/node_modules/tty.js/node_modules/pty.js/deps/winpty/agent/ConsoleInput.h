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

#ifndef CONSOLEINPUT_H
#define CONSOLEINPUT_H

#include <string>
#include <vector>
#include <windows.h>

class Win32Console;
class DsrSender;

class ConsoleInput
{
public:
    ConsoleInput(Win32Console *console, DsrSender *dsrSender);
    void writeInput(const std::string &input);
    void flushIncompleteEscapeCode();

private:
    struct KeyDescriptor {
        const char *encoding;
        int virtualKey;
        int unicodeChar;
        int keyState;
        int encodingLen;
    };

    class KeyLookup {
    public:
        KeyLookup();
        ~KeyLookup();
        void set(const char *encoding, const KeyDescriptor *descriptor);
        const KeyDescriptor *getMatch() const { return match; }
        bool hasChildren() const { return children != NULL; }
        KeyLookup *getChild(int i) { return children != NULL ? (*children)[i] : NULL; }
    private:
        const KeyDescriptor *match;
        KeyLookup *(*children)[256];
    };

    void doWrite(bool isEof);
    int scanKeyPress(std::vector<INPUT_RECORD> &records,
                     const char *input,
                     int inputSize,
                     bool isEof);
    void appendUtf8Char(std::vector<INPUT_RECORD> &records,
                        const char *charBuffer,
                        int charLen,
                        int keyState);
    void appendKeyPress(std::vector<INPUT_RECORD> &records,
                        int virtualKey,
                        int unicodeChar,
                        int keyState);
    void appendInputRecord(std::vector<INPUT_RECORD> &records,
                           BOOL keyDown,
                           int virtualKey,
                           int unicodeChar,
                           int keyState);
    static int utf8CharLength(char firstByte);
    const KeyDescriptor *lookupKey(const char *encoding, bool isEof, bool *incomplete);
    static int matchDsr(const char *encoding);

private:
    static KeyDescriptor keyDescriptorTable[];
    Win32Console *m_console;
    DsrSender *m_dsrSender;
    bool m_dsrSent;
    std::string m_byteQueue;
    KeyLookup m_lookup;
    DWORD lastWriteTick;
};

#endif // CONSOLEINPUT_H
