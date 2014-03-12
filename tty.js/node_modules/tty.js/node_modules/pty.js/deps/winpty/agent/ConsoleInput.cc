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

#include "ConsoleInput.h"
#include "Win32Console.h"
#include "DsrSender.h"
#include "../shared/DebugClient.h"
#include <string.h>
#include <stdio.h>

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif

const int kIncompleteEscapeTimeoutMs = 1000;

#define ESC "\x1B"
#define CSI ESC"["
#define DIM(x) (sizeof(x) / sizeof((x)[0]))

ConsoleInput::KeyDescriptor ConsoleInput::keyDescriptorTable[] = {
    // Ctrl-<letter/digit> seems to be handled OK by the default code path.
    // TODO: Alt-ESC is encoded as ESC ESC.  Can it be handled?

    {   ESC,            VK_ESCAPE,  '\x1B', 0,              },

    // Alt-<letter/digit>
    {   ESC"O",         'O',        0,  LEFT_ALT_PRESSED    },
    {   ESC"[",         '[',        0,  LEFT_ALT_PRESSED    },

    // F1-F4 function keys.  F5-F12 seem to be handled more consistently among
    // various TERM=xterm terminals (gnome-terminal, konsole, xterm, mintty),
    // using a CSI-prefix with an optional extra modifier digit.  (putty is
    // also TERM=xterm, though, and has completely different modified F5-F12
    // encodings.)
    {   ESC"OP",        VK_F1,      0,  0,                  }, // xt gt kon
    {   ESC"OQ",        VK_F2,      0,  0,                  }, // xt gt kon
    {   ESC"OR",        VK_F3,      0,  0,                  }, // xt gt kon
    {   ESC"OS",        VK_F4,      0,  0,                  }, // xt gt kon

    {   "\x7F",         VK_BACK,    '\x08', 0,                  },
    {   ESC"\x7F",      VK_BACK,    '\x08', LEFT_ALT_PRESSED,   },
    {   ESC"OH",        VK_HOME,    0,  0,                      }, // gnome-terminal
    {   ESC"OF",        VK_END,     0,  0,                      }, // gnome-terminal
};

ConsoleInput::ConsoleInput(Win32Console *console, DsrSender *dsrSender) :
    m_console(console),
    m_dsrSender(dsrSender),
    m_dsrSent(false),
    lastWriteTick(0)
{
    // Generate CSI encodings and add them to the table.
    struct CsiEncoding {
        int id;
        char letter;
        int virtualKey;
    };
    static const CsiEncoding csiEncodings[] = {
        {   0,  'A',    VK_UP       },
        {   0,  'B',    VK_DOWN     },
        {   0,  'C',    VK_RIGHT    },
        {   0,  'D',    VK_LEFT     },
        {   0,  'E',    VK_NUMPAD5  },
        {   0,  'F',    VK_END      },
        {   0,  'H',    VK_HOME     },
        {   0,  'P',    VK_F1       },  // mod+F1 for xterm and mintty
        {   0,  'Q',    VK_F2       },  // mod+F2 for xterm and mintty
        {   0,  'R',    VK_F3       },  // mod+F3 for xterm and mintty
        {   0,  'S',    VK_F4       },  // mod+F4 for xterm and mintty
        {   1,  '~',    VK_HOME     },
        {   2,  '~',    VK_INSERT   },
        {   3,  '~',    VK_DELETE   },
        {   4,  '~',    VK_END      },  // gnome-terminal keypad home/end
        {   5,  '~',    VK_PRIOR    },
        {   6,  '~',    VK_NEXT     },
        {   7,  '~',    VK_HOME     },
        {   8,  '~',    VK_END      },
        {   15, '~',    VK_F5       },
        {   17, '~',    VK_F6       },
        {   18, '~',    VK_F7       },
        {   19, '~',    VK_F8       },
        {   20, '~',    VK_F9       },
        {   21, '~',    VK_F10      },
        {   23, '~',    VK_F11      },
        {   24, '~',    VK_F12      },
    };
    const int kCsiShiftModifier = 1;
    const int kCsiAltModifier   = 2;
    const int kCsiCtrlModifier  = 4;
    char encoding[32];
    for (size_t i = 0; i < DIM(csiEncodings); ++i) {
        const CsiEncoding *e = &csiEncodings[i];
        if (e->id == 0)
            sprintf(encoding, CSI"%c", e->letter);
        else
            sprintf(encoding, CSI"%d%c", e->id, e->letter);
        KeyDescriptor *k = new KeyDescriptor;
        k->encoding = NULL;
        k->encodingLen = strlen(encoding);
        k->keyState = 0;
        k->unicodeChar = 0;
        k->virtualKey = csiEncodings[i].virtualKey;
        m_lookup.set(encoding, k);
        int id = !e->id ? 1 : e->id;
        for (int mod = 2; mod <= 8; ++mod) {
            sprintf(encoding, CSI"%d;%d%c", id, mod, e->letter);
            KeyDescriptor *k2 = new KeyDescriptor;
            *k2 = *k;
            k2->encodingLen = strlen(encoding);
            if ((mod - 1) & kCsiShiftModifier)  k2->keyState |= SHIFT_PRESSED;
            if ((mod - 1) & kCsiAltModifier)    k2->keyState |= LEFT_ALT_PRESSED;
            if ((mod - 1) & kCsiCtrlModifier)   k2->keyState |= LEFT_CTRL_PRESSED;
            m_lookup.set(encoding, k2);
        }
    }

    // Modified F1-F4 on gnome-terminal and konsole.
    for (int mod = 2; mod <= 8; ++mod) {
        for (int fn = 0; fn < 4; ++fn) {
            for (int fmt = 0; fmt < 1; ++fmt) {
                if (fmt == 0) {
                    // gnome-terminal
                    sprintf(encoding, ESC"O1;%d%c", mod, 'P' + fn);
                } else {
                    // konsole
                    sprintf(encoding, ESC"O%d%c", mod, 'P' + fn);
                }
                KeyDescriptor *k = new KeyDescriptor;
                k->encoding = NULL;
                k->encodingLen = strlen(encoding);
                k->keyState = 0;
                if ((mod - 1) & kCsiShiftModifier)  k->keyState |= SHIFT_PRESSED;
                if ((mod - 1) & kCsiAltModifier)    k->keyState |= LEFT_ALT_PRESSED;
                if ((mod - 1) & kCsiCtrlModifier)   k->keyState |= LEFT_CTRL_PRESSED;
                k->unicodeChar = 0;
                k->virtualKey = VK_F1 + fn;
                m_lookup.set(encoding, k);
            }
        }
    }

    // Static key encodings.
    for (size_t i = 0; i < sizeof(keyDescriptorTable) / sizeof(keyDescriptorTable[0]); ++i) {
        KeyDescriptor *k = new KeyDescriptor;
        *k = keyDescriptorTable[i];
        k->encodingLen = strlen(k->encoding);
        m_lookup.set(k->encoding, k);
    }
}

void ConsoleInput::writeInput(const std::string &input)
{
    trace("writeInput: %d bytes", input.size());
    if (input.size() == 0)
        return;
    m_byteQueue.append(input);
    doWrite(false);
    if (!m_byteQueue.empty() && !m_dsrSent) {
        trace("send DSR");
        m_dsrSender->sendDsr();
        m_dsrSent = true;
    }
    lastWriteTick = GetTickCount();
}

void ConsoleInput::flushIncompleteEscapeCode()
{
    if (!m_byteQueue.empty() &&
            (int)(GetTickCount() - lastWriteTick) > kIncompleteEscapeTimeoutMs) {
        doWrite(true);
        m_byteQueue.clear();
    }
}

ConsoleInput::KeyLookup::KeyLookup() : match(NULL), children(NULL)
{
}

ConsoleInput::KeyLookup::~KeyLookup()
{
    delete match;
    if (children != NULL) {
        for (int i = 0; i < 256; ++i)
            delete (*children)[i];
    }
    delete [] children;
}

void ConsoleInput::KeyLookup::set(const char *encoding,
                                  const KeyDescriptor *descriptor)
{
    unsigned char ch = encoding[0];
    if (ch == '\0') {
        match = descriptor;
        return;
    }
    if (children == NULL) {
        children = (KeyLookup*(*)[256])new KeyLookup*[256];
        memset(children, 0, sizeof(KeyLookup*) * 256);
    }
    if ((*children)[ch] == NULL) {
        (*children)[ch] = new KeyLookup;
    }
    (*children)[ch]->set(encoding + 1, descriptor);
}

void ConsoleInput::doWrite(bool isEof)
{
    const char *data = m_byteQueue.c_str();
    std::vector<INPUT_RECORD> records;
    size_t idx = 0;
    while (idx < m_byteQueue.size()) {
        int charSize = scanKeyPress(records, &data[idx], m_byteQueue.size() - idx, isEof);
        if (charSize == -1)
            break;
        idx += charSize;
    }
    m_byteQueue.erase(0, idx);
    m_console->writeInput(records.data(), records.size());
}

int ConsoleInput::scanKeyPress(std::vector<INPUT_RECORD> &records,
                               const char *input,
                               int inputSize,
                               bool isEof)
{
    trace("scanKeyPress: %d bytes", inputSize);

    // Ctrl-C.
    if (input[0] == '\x03' && m_console->processedInputMode()) {
        trace("Ctrl-C");
        BOOL ret = GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        trace("GenerateConsoleCtrlEvent: %d", ret);
        return 1;
    }

    // Attempt to match the Device Status Report (DSR) reply.
    int dsrLen = matchDsr(input);
    if (dsrLen > 0) {
        trace("Received a DSR reply");
        m_dsrSent = false;
        return dsrLen;
    } else if (!isEof && dsrLen == -1) {
        // Incomplete DSR match.
        trace("Incomplete DSR match");
        return -1;
    }

    // Recognize Alt-<character>.
    if (input[0] == '\x1B' &&
            input[1] != '\0' &&
            input[1] != '\x1B' &&
            m_lookup.getChild('\x1B')->getChild(input[1]) == NULL) {
        int len = utf8CharLength(input[1]);
        if (1 + len > inputSize) {
            // Incomplete character.
            trace("Incomplete Alt-char match");
            return -1;
        }
        appendUtf8Char(records, &input[1], len, LEFT_ALT_PRESSED);
        return 1 + len;
    }

    // Recognize an ESC-encoded keypress.
    bool incomplete;
    const KeyDescriptor *match = lookupKey(input, isEof, &incomplete);
    if (incomplete) {
        // Incomplete match -- need more characters (or wait for a
        // timeout to signify flushed input).
        trace("Incomplete ESC-keypress match");
        return -1;
    } else if (match != NULL) {
        appendKeyPress(records,
                       match->virtualKey,
                       match->unicodeChar,
                       match->keyState);
        return match->encodingLen;
    }

    // A UTF-8 character.
    int len = utf8CharLength(input[0]);
    if (len > inputSize) {
        // Incomplete character.
        trace("Incomplete UTF-8 character");
        return -1;
    }
    appendUtf8Char(records, &input[0], len, 0);
    return len;
}

void ConsoleInput::appendUtf8Char(std::vector<INPUT_RECORD> &records,
                                  const char *charBuffer,
                                  int charLen,
                                  int keyState)
{
    WCHAR wideInput[2];
    int wideLen = MultiByteToWideChar(CP_UTF8,
                                      0,
                                      charBuffer,
                                      charLen,
                                      wideInput,
                                      sizeof(wideInput) / sizeof(wideInput[0]));
    // TODO: Characters outside the BMP.
    if (wideLen != 1)
        return;

    short charScan = VkKeyScan(wideInput[0]);
    int virtualKey = 0;
    if (charScan != -1) {
        virtualKey = charScan & 0xFF;
        if (charScan & 0x100)
            keyState |= SHIFT_PRESSED;
        else if (charScan & 0x200)
            keyState |= LEFT_CTRL_PRESSED;
        else if (charScan & 0x400)
            keyState |= LEFT_ALT_PRESSED;
    }
    appendKeyPress(records, virtualKey, wideInput[0], keyState);
}

void ConsoleInput::appendKeyPress(std::vector<INPUT_RECORD> &records,
                                  int virtualKey,
                                  int unicodeChar,
                                  int keyState)
{
    bool ctrl = keyState & LEFT_CTRL_PRESSED;
    bool alt = keyState & LEFT_ALT_PRESSED;
    bool shift = keyState & SHIFT_PRESSED;
    int stepKeyState = 0;
    if (ctrl) {
        stepKeyState |= LEFT_CTRL_PRESSED;
        appendInputRecord(records, TRUE, VK_CONTROL, 0, stepKeyState);
    }
    if (alt) {
        stepKeyState |= LEFT_ALT_PRESSED;
        appendInputRecord(records, TRUE, VK_MENU, 0, stepKeyState);
    }
    if (shift) {
        stepKeyState |= SHIFT_PRESSED;
        appendInputRecord(records, TRUE, VK_SHIFT, 0, stepKeyState);
    }
    if (ctrl && alt) {
        // This behavior seems arbitrary, but it's what I see in the Windows 7
        // console.
        unicodeChar = 0;
    }
    appendInputRecord(records, TRUE, virtualKey, unicodeChar, stepKeyState);
    if (alt) {
        // This behavior seems arbitrary, but it's what I see in the Windows 7
        // console.
        unicodeChar = 0;
    }
    appendInputRecord(records, FALSE, virtualKey, unicodeChar, stepKeyState);
    if (shift) {
        stepKeyState &= ~SHIFT_PRESSED;
        appendInputRecord(records, FALSE, VK_SHIFT, 0, stepKeyState);
    }
    if (alt) {
        stepKeyState &= ~LEFT_ALT_PRESSED;
        appendInputRecord(records, FALSE, VK_MENU, 0, stepKeyState);
    }
    if (ctrl) {
        stepKeyState &= ~LEFT_CTRL_PRESSED;
        appendInputRecord(records, FALSE, VK_CONTROL, 0, stepKeyState);
    }
}

void ConsoleInput::appendInputRecord(std::vector<INPUT_RECORD> &records,
                                     BOOL keyDown,
                                     int virtualKey,
                                     int unicodeChar,
                                     int keyState)
{
    INPUT_RECORD ir;
    memset(&ir, 0, sizeof(ir));
    ir.EventType = KEY_EVENT;
    ir.Event.KeyEvent.bKeyDown = keyDown;
    ir.Event.KeyEvent.wRepeatCount = 1;
    ir.Event.KeyEvent.wVirtualKeyCode = virtualKey;
    ir.Event.KeyEvent.wVirtualScanCode =
            MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    ir.Event.KeyEvent.uChar.UnicodeChar = unicodeChar;
    ir.Event.KeyEvent.dwControlKeyState = keyState;
    records.push_back(ir);
}

// Return the byte size of a UTF-8 character using the value of the first
// byte.
int ConsoleInput::utf8CharLength(char firstByte)
{
    // This code would probably be faster if it used __builtin_clz.
    if ((firstByte & 0x80) == 0) {
        return 1;
    } else if ((firstByte & 0xE0) == 0xC0) {
        return 2;
    } else if ((firstByte & 0xF0) == 0xE0) {
        return 3;
    } else if ((firstByte & 0xF8) == 0xF0) {
        return 4;
    } else if ((firstByte & 0xFC) == 0xF8) {
        return 5;
    } else if ((firstByte & 0xFE) == 0xFC) {
        return 6;
    } else {
        // Malformed UTF-8.
        return 1;
    }
}

// Find the longest matching key and node.
const ConsoleInput::KeyDescriptor *
ConsoleInput::lookupKey(const char *encoding, bool isEof, bool *incomplete)
{
    trace("lookupKey");
    for (int i = 0; encoding[i] != '\0'; ++i)
        trace("%d", encoding[i]);

    *incomplete = false;
    KeyLookup *node = &m_lookup;
    const KeyDescriptor *longestMatch = NULL;
    for (int i = 0; encoding[i] != '\0'; ++i) {
        unsigned char ch = encoding[i];
        node = node->getChild(ch);
        trace("ch: %d --> node:%p", ch, node);
        if (node == NULL) {
            return longestMatch;
        } else if (node->getMatch() != NULL) {
            longestMatch = node->getMatch();
        }
    }
    if (isEof) {
        return longestMatch;
    } else if (node->hasChildren()) {
        *incomplete = true;
        return NULL;
    } else {
        return longestMatch;
    }
}

// Match the Device Status Report console input:  ESC [ nn ; mm R
// Returns:
// 0   no match
// >0  match, returns length of match
// -1  incomplete match
int ConsoleInput::matchDsr(const char *encoding)
{
    const char *pch = encoding;
#define CHECK(cond) \
        do { \
            if (cond) { pch++; } \
            else if (*pch == '\0') { return -1; } \
            else { return 0; } \
        } while(0)
    CHECK(*pch == '\x1B');
    CHECK(*pch == '[');
    CHECK(isdigit(*pch));
    while (isdigit(*pch))
        pch++;
    CHECK(*pch == ';');
    CHECK(isdigit(*pch));
    while (isdigit(*pch))
        pch++;
    CHECK(*pch == 'R');
    return pch - encoding;
#undef CHECK
}
