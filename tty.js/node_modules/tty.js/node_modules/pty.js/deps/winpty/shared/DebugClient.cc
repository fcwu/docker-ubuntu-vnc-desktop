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

#include "DebugClient.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "c99_snprintf.h"

char *tracingConfig;

static void sendToDebugServer(const char *message)
{
    char response[16];
    DWORD responseSize;
    CallNamedPipeA(
        "\\\\.\\pipe\\DebugServer",
        (void*)message, strlen(message),
        response, sizeof(response), &responseSize,
        NMPWAIT_WAIT_FOREVER);
}

// Get the current UTC time as milliseconds from the epoch (ignoring leap
// seconds).  Use the Unix epoch for consistency with DebugClient.py.  There
// are 134774 days between 1601-01-01 (the Win32 epoch) and 1970-01-01 (the
// Unix epoch).
static long long unixTimeMillis()
{
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    long long msTime = (((long long)fileTime.dwHighDateTime << 32) +
                       fileTime.dwLowDateTime) / 10000;
    return msTime - 134774LL * 24 * 3600 * 1000;
}

static const char *getTracingConfig()
{
    if (tracingConfig == NULL) {
        const int bufSize = 256;
        char buf[bufSize];
        DWORD actualSize = GetEnvironmentVariableA("WINPTYDBG", buf, bufSize);
        if (actualSize == 0 || actualSize >= (DWORD)bufSize)
            buf[0] = '\0';
        tracingConfig = new char[strlen(buf) + 1];
        strcpy(tracingConfig, buf);
    }
    return tracingConfig;
}

bool isTracingEnabled()
{
    return getTracingConfig()[0] != '\0';
}

void trace(const char *format, ...)
{
    if (!isTracingEnabled())
        return;

    char message[1024];

    va_list ap;
    va_start(ap, format);
    c99_vsnprintf(message, sizeof(message), format, ap);
    message[sizeof(message) - 1] = '\0';
    va_end(ap);

    const int currentTime = (int)(unixTimeMillis() % (100000 * 1000));

    char moduleName[1024];
    moduleName[0] = '\0';
    GetModuleFileNameA(NULL, moduleName, sizeof(moduleName));
    const char *baseName = strrchr(moduleName, '\\');
    baseName = (baseName != NULL) ? baseName + 1 : moduleName;

    char fullMessage[1024];
    c99_snprintf(fullMessage, sizeof(fullMessage),
             "[%05d.%03d %s,p%04d,t%04d]: %s",
             currentTime / 1000, currentTime % 1000,
             baseName, (int)GetCurrentProcessId(), (int)GetCurrentThreadId(),
             message);
    fullMessage[sizeof(fullMessage) - 1] = '\0';

    sendToDebugServer(fullMessage);
}