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

#ifndef NAMEDPIPE_H
#define NAMEDPIPE_H

#include <windows.h>
#include <string>
#include <vector>

class EventLoop;

class NamedPipe
{
private:
    // The EventLoop uses these private members.
    friend class EventLoop;
    NamedPipe();
    ~NamedPipe();
    bool serviceIo(std::vector<HANDLE> *waitHandles);

private:
    class IoWorker
    {
    public:
        IoWorker(NamedPipe *namedPipe);
        virtual ~IoWorker();
        int service();
        HANDLE getWaitEvent();
    protected:
        NamedPipe *m_namedPipe;
        bool m_pending;
        int m_currentIoSize;
        HANDLE m_event;
        OVERLAPPED m_over;
        enum { kIoSize = 64 * 1024 };
        char m_buffer[kIoSize];
        virtual void completeIo(int size) = 0;
        virtual bool shouldIssueIo(int *size, bool *isRead) = 0;
    };

    class InputWorker : public IoWorker
    {
    public:
        InputWorker(NamedPipe *namedPipe) : IoWorker(namedPipe) {}
    protected:
        virtual void completeIo(int size);
        virtual bool shouldIssueIo(int *size, bool *isRead);
    };

    class OutputWorker : public IoWorker
    {
    public:
        OutputWorker(NamedPipe *namedPipe) : IoWorker(namedPipe) {}
        int getPendingIoSize();
    protected:
        virtual void completeIo(int size);
        virtual bool shouldIssueIo(int *size, bool *isRead);
    };

public:
    bool connectToServer(LPCWSTR pipeName);
    int bytesToSend();
    void write(const void *data, int size);
    void write(const char *text);
    int readBufferSize();
    void setReadBufferSize(int size);
    int bytesAvailable();
    int peek(void *data, int size);
    std::string read(int size);
    std::string readAll();
    void closePipe();
    bool isClosed();

private:
    // Input/output buffers
    int m_readBufferSize;
    std::string m_inQueue;
    std::string m_outQueue;
    HANDLE m_handle;
    InputWorker *m_inputWorker;
    OutputWorker *m_outputWorker;
};

#endif // NAMEDPIPE_H
