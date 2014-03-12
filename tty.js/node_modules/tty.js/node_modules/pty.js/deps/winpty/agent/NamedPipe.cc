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

#include "NamedPipe.h"
#include "EventLoop.h"
#include "AgentAssert.h"
#include "../shared/DebugClient.h"
#include <string.h>

NamedPipe::NamedPipe() :
    m_readBufferSize(64 * 1024),
    m_handle(NULL),
    m_inputWorker(NULL),
    m_outputWorker(NULL)
{
}

NamedPipe::~NamedPipe()
{
    closePipe();
}

// Returns true if anything happens (data received, data sent, pipe error).
bool NamedPipe::serviceIo(std::vector<HANDLE> *waitHandles)
{
    if (m_handle == NULL)
        return false;
    int readBytes = m_inputWorker->service();
    int writeBytes = m_outputWorker->service();
    if (readBytes == -1 || writeBytes == -1) {
        closePipe();
        return true;
    }
    if (m_inputWorker->getWaitEvent() != NULL)
        waitHandles->push_back(m_inputWorker->getWaitEvent());
    if (m_outputWorker->getWaitEvent() != NULL)
        waitHandles->push_back(m_outputWorker->getWaitEvent());
    return readBytes > 0 || writeBytes > 0;
}

NamedPipe::IoWorker::IoWorker(NamedPipe *namedPipe) :
    m_namedPipe(namedPipe),
    m_pending(false),
    m_currentIoSize(-1)
{
    m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    ASSERT(m_event != NULL);
}

NamedPipe::IoWorker::~IoWorker()
{
    CloseHandle(m_event);
}

int NamedPipe::IoWorker::service()
{
    int progress = 0;
    if (m_pending) {
        DWORD actual;
        BOOL ret = GetOverlappedResult(m_namedPipe->m_handle, &m_over, &actual, FALSE);
        if (!ret) {
            if (GetLastError() == ERROR_IO_INCOMPLETE) {
                // There is a pending I/O.
                return progress;
            } else {
                // Pipe error.
                return -1;
            }
        }
        ResetEvent(m_event);
        m_pending = false;
        completeIo(actual);
        m_currentIoSize = -1;
        progress += actual;
    }
    int nextSize;
    bool isRead;
    while (shouldIssueIo(&nextSize, &isRead)) {
        m_currentIoSize = nextSize;
        DWORD actual = 0;
        memset(&m_over, 0, sizeof(m_over));
        m_over.hEvent = m_event;
        BOOL ret = isRead
                ? ReadFile(m_namedPipe->m_handle, m_buffer, nextSize, &actual, &m_over)
                : WriteFile(m_namedPipe->m_handle, m_buffer, nextSize, &actual, &m_over);
        if (!ret) {
            if (GetLastError() == ERROR_IO_PENDING) {
                // There is a pending I/O.
                m_pending = true;
                return progress;
            } else {
                // Pipe error.
                return -1;
            }
        }
        ResetEvent(m_event);
        completeIo(actual);
        m_currentIoSize = -1;
        progress += actual;
    }
    return progress;
}

HANDLE NamedPipe::IoWorker::getWaitEvent()
{
    return m_pending ? m_event : NULL;
}

void NamedPipe::InputWorker::completeIo(int size)
{
    m_namedPipe->m_inQueue.append(m_buffer, size);
}

bool NamedPipe::InputWorker::shouldIssueIo(int *size, bool *isRead)
{
    *isRead = true;
    if (m_namedPipe->isClosed()) {
        return false;
    } else if ((int)m_namedPipe->m_inQueue.size() < m_namedPipe->readBufferSize()) {
        *size = kIoSize;
        return true;
    } else {
        return false;
    }
}

void NamedPipe::OutputWorker::completeIo(int size)
{
    ASSERT(size == m_currentIoSize);
}

bool NamedPipe::OutputWorker::shouldIssueIo(int *size, bool *isRead)
{
    *isRead = false;
    if (!m_namedPipe->m_outQueue.empty()) {
        int writeSize = std::min((int)m_namedPipe->m_outQueue.size(), (int)kIoSize);
        memcpy(m_buffer, m_namedPipe->m_outQueue.data(), writeSize);
        m_namedPipe->m_outQueue.erase(0, writeSize);
        *size = writeSize;
        return true;
    } else {
        return false;
    }
}

int NamedPipe::OutputWorker::getPendingIoSize()
{
    return m_pending ? m_currentIoSize : 0;
}

bool NamedPipe::connectToServer(LPCWSTR pipeName)
{
    ASSERT(isClosed());
    HANDLE handle = CreateFile(pipeName,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_OVERLAPPED,
                               NULL);
    trace("connection to [%ls], handle == 0x%x", pipeName, handle);
    if (handle == INVALID_HANDLE_VALUE)
        return false;
    m_handle = handle;
    m_inputWorker = new InputWorker(this);
    m_outputWorker = new OutputWorker(this);
    return true;
}

int NamedPipe::bytesToSend()
{
    int ret = m_outQueue.size();
    if (m_outputWorker != NULL)
        ret += m_outputWorker->getPendingIoSize();
    return ret;
}

void NamedPipe::write(const void *data, int size)
{
    m_outQueue.append((const char*)data, size);
}

void NamedPipe::write(const char *text)
{
    write(text, strlen(text));
}

int NamedPipe::readBufferSize()
{
    return m_readBufferSize;
}

void NamedPipe::setReadBufferSize(int size)
{
    m_readBufferSize = size;
}

int NamedPipe::bytesAvailable()
{
    return m_inQueue.size();
}

int NamedPipe::peek(void *data, int size)
{
    int ret = std::min(size, (int)m_inQueue.size());
    memcpy(data, m_inQueue.data(), ret);
    return ret;
}

std::string NamedPipe::read(int size)
{
    int retSize = std::min(size, (int)m_inQueue.size());
    std::string ret = m_inQueue.substr(0, retSize);
    m_inQueue.erase(0, retSize);
    return ret;
}

std::string NamedPipe::readAll()
{
    std::string ret = m_inQueue;
    m_inQueue.clear();
    return ret;
}

void NamedPipe::closePipe()
{
    if (m_handle == NULL)
        return;
    CancelIo(m_handle);
    delete m_inputWorker;
    delete m_outputWorker;
    CloseHandle(m_handle);
    m_handle = NULL;
    m_inputWorker = NULL;
    m_outputWorker = NULL;
}

bool NamedPipe::isClosed()
{
    return m_handle == NULL;
}
