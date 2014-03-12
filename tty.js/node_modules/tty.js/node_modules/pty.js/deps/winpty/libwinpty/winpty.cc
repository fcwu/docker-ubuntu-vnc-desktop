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

#include <winpty.h>
#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

std::string to_utf8(const wchar_t* buffer, int len)
{
        int nChars = ::WideCharToMultiByte(
                CP_UTF8,
                0,
                buffer,
                len,
                NULL,
                0,
                NULL,
                NULL);
        if (nChars == 0) return "";

        std::string newbuffer;
        newbuffer.resize(nChars) ;
        ::WideCharToMultiByte(
                CP_UTF8,
                0,
                buffer,
                len,
                const_cast< char* >(newbuffer.c_str()),
                nChars,
                NULL,
                NULL); 

        return newbuffer;
}

std::string to_utf8(const std::wstring& str)
{
        return to_utf8(str.c_str(), (int)str.size());
}

std::wstring to_wstring(const std::string& s)
{
 int len;
 int slength = (int)s.length() + 1;
 len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
 wchar_t* buf = new wchar_t[len];
 MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
 std::wstring r(buf);
 delete[] buf;
 return r;
}



#include "../shared/DebugClient.h"
#include "../shared/AgentMsg.h"
#include "../shared/Buffer.h"
#include "../shared/c99_snprintf.h"

// TODO: Error handling, handle out-of-memory.

#define AGENT_EXE L"winpty-agent.exe"

static volatile LONG consoleCounter;

struct winpty_s {
    winpty_s();
    HANDLE controlPipe;
    HANDLE dataPipe;
};

winpty_s::winpty_s() : controlPipe(NULL), dataPipe(NULL)
{
}

static HMODULE getCurrentModule()
{
    HMODULE module;
    if (!GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCTSTR)getCurrentModule,
                &module)) {
        assert(false);
    }
    return module;
}

static std::wstring getModuleFileName(HMODULE module)
{
    const int bufsize = 4096;
    wchar_t path[bufsize];
    int size = GetModuleFileName(module, path, bufsize);
    assert(size != 0 && size != bufsize);
    return std::wstring(path);
}

static std::wstring dirname(const std::wstring &path)
{
    std::wstring::size_type pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos)
        return L"";
    else
        return path.substr(0, pos);
}

static bool pathExists(const std::wstring &path)
{
    return GetFileAttributes(path.c_str()) != 0xFFFFFFFF;
}

static std::wstring findAgentProgram()
{
    std::wstring progDir = dirname(getModuleFileName(getCurrentModule()));
    std::wstring ret = progDir + L"\\"AGENT_EXE;
    assert(pathExists(ret));
    return ret;
}

// Call ConnectNamedPipe and block, even for an overlapped pipe.  If the
// pipe is overlapped, create a temporary event for use connecting.
static bool connectNamedPipe(HANDLE handle, bool overlapped)
{
    OVERLAPPED over, *pover = NULL;
    if (overlapped) {
        pover = &over;
        memset(&over, 0, sizeof(over));
        over.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        assert(over.hEvent != NULL);
    }
    bool success = ConnectNamedPipe(handle, pover);
    if (overlapped && !success && GetLastError() == ERROR_IO_PENDING) {
        DWORD actual;
        success = GetOverlappedResult(handle, pover, &actual, TRUE);
    }
    if (!success && GetLastError() == ERROR_PIPE_CONNECTED)
        success = TRUE;
    if (overlapped)
        CloseHandle(over.hEvent);
    return success;
}

static void writePacket(winpty_t *pc, const WriteBuffer &packet)
{
    std::string payload = packet.str();

    int32_t payloadSize = payload.size();
    DWORD actual;
    BOOL success = WriteFile(pc->controlPipe, &payloadSize, sizeof(int32_t), &actual, NULL);

    assert(success && actual == sizeof(int32_t));

    success = WriteFile(pc->controlPipe, payload.c_str(), payloadSize, &actual, NULL);
    assert(success && (int32_t)actual == payloadSize);
}

static int32_t readInt32(winpty_t *pc)
{
    int32_t result;
    DWORD actual;
    BOOL success = ReadFile(pc->controlPipe, &result, sizeof(int32_t), &actual, NULL);
    assert(success && actual == sizeof(int32_t));
    return result;
}

static HANDLE createNamedPipe(const std::wstring &name, bool overlapped)
{
    return CreateNamedPipe(name.c_str(),
                           /*dwOpenMode=*/
                           PIPE_ACCESS_DUPLEX |
                           FILE_FLAG_FIRST_PIPE_INSTANCE |
                           (overlapped ? FILE_FLAG_OVERLAPPED : 0),
                           /*dwPipeMode=*/0,
                           /*nMaxInstances=*/1,
                           /*nOutBufferSize=*/0,
                           /*nInBufferSize=*/0,
                           /*nDefaultTimeOut=*/3000,
                           NULL);
}

struct BackgroundDesktop {
    HWINSTA originalStation;
    HWINSTA station;
    HDESK desktop;
    std::wstring desktopName;
};

static std::wstring getObjectName(HANDLE object)
{
    BOOL success;
    DWORD lengthNeeded = 0;
    GetUserObjectInformation(object, UOI_NAME,
                             NULL, 0,
                             &lengthNeeded);
    assert(lengthNeeded % sizeof(wchar_t) == 0);
    wchar_t *tmp = new wchar_t[lengthNeeded / 2];
    success = GetUserObjectInformation(object, UOI_NAME,
                                       tmp, lengthNeeded,
                                       NULL);
    assert(success);
    std::wstring ret = tmp;
    delete [] tmp;
    return ret;
}

// Get a non-interactive window station for the agent.
// TODO: review security w.r.t. windowstation and desktop.
static BackgroundDesktop setupBackgroundDesktop()
{
    BackgroundDesktop ret;
    ret.originalStation = GetProcessWindowStation();
    ret.station = CreateWindowStation(NULL, 0, WINSTA_ALL_ACCESS, NULL);
    bool success = SetProcessWindowStation(ret.station);
    assert(success);
    ret.desktop = CreateDesktop(L"Default", NULL, NULL, 0, GENERIC_ALL, NULL);
    assert(ret.originalStation != NULL);
    assert(ret.station != NULL);
    assert(ret.desktop != NULL);
    ret.desktopName =
        getObjectName(ret.station) + L"\\" + getObjectName(ret.desktop);
    return ret;
}

static void restoreOriginalDesktop(const BackgroundDesktop &desktop)
{
    SetProcessWindowStation(desktop.originalStation);
    CloseDesktop(desktop.desktop);
    CloseWindowStation(desktop.station);
}

static std::wstring getDesktopFullName()
{
    // MSDN says that the handle returned by GetThreadDesktop does not need
    // to be passed to CloseDesktop.
    HWINSTA station = GetProcessWindowStation();
    HDESK desktop = GetThreadDesktop(GetCurrentThreadId());
    assert(station != NULL);
    assert(desktop != NULL);
    return getObjectName(station) + L"\\" + getObjectName(desktop);
}

static void startAgentProcess(const BackgroundDesktop &desktop,
                              std::wstring &controlPipeName,
                              std::wstring &dataPipeName, 
                              int cols, int rows)
{
    bool success;

    std::wstring agentProgram = findAgentProgram();
    std::wstringstream agentCmdLineStream;
    agentCmdLineStream << L"\"" << agentProgram << L"\" "
                       << controlPipeName << " " << dataPipeName << " "
                       << cols << " " << rows;
    std::wstring agentCmdLine = agentCmdLineStream.str();

    // Start the agent.
    STARTUPINFO sui;
    memset(&sui, 0, sizeof(sui));
    sui.cb = sizeof(sui);
    sui.lpDesktop = (LPWSTR)desktop.desktopName.c_str();
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));
    std::vector<wchar_t> cmdline(agentCmdLine.size() + 1);
    agentCmdLine.copy(&cmdline[0], agentCmdLine.size());
    cmdline[agentCmdLine.size()] = L'\0';
    success = CreateProcess(agentProgram.c_str(),
                            &cmdline[0],
                            NULL, NULL,
                            /*bInheritHandles=*/FALSE,
                            /*dwCreationFlags=*/CREATE_NEW_CONSOLE,
                            NULL, NULL,
                            &sui, &pi);
    if (!success) {
        fprintf(stderr,
                "Error %#x starting %ls\n",
                (unsigned int)GetLastError(),
                agentCmdLine.c_str());
        exit(1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

WINPTY_API winpty_t *winpty_open(int cols, int rows)
{
    winpty_t *pc = new winpty_t;

    // Start pipes.
    std::wstringstream pipeName;

	// Allow custom pipe names.
	pipeName << L"\\\\.\\pipe\\winpty-" << GetCurrentProcessId()
				<< L"-" << InterlockedIncrement(&consoleCounter);
	std::wstring controlPipeName = pipeName.str() + L"-control";
	std::wstring dataPipeName = pipeName.str() + L"-data";

    pc->controlPipe = createNamedPipe(controlPipeName, false);
    if (pc->controlPipe == INVALID_HANDLE_VALUE) {
        delete pc;
        return NULL;
    }
    pc->dataPipe = createNamedPipe(dataPipeName, true);
    if (pc->dataPipe == INVALID_HANDLE_VALUE) {
        delete pc;
        return NULL;
    }

    // Setup a background desktop for the agent.
    BackgroundDesktop desktop = setupBackgroundDesktop();

    // Start the agent.
    startAgentProcess(desktop, controlPipeName, dataPipeName, cols, rows);

    // TODO: Frequently, I see the CreateProcess call return successfully,
    // but the agent immediately dies.  The following pipe connect calls then
    // hang.  These calls should probably timeout.  Maybe this code could also
    // poll the agent process handle?

    // Connect the pipes.
    bool success;
    success = connectNamedPipe(pc->controlPipe, false);
    if (!success) {
        delete pc;
        return NULL;
    }
    success = connectNamedPipe(pc->dataPipe, true);
    if (!success) {
        delete pc;
        return NULL;
    }

    // Close handles to the background desktop and restore the original window
    // station.  This must wait until we know the agent is running -- if we
    // close these handles too soon, then the desktop and windowstation will be
    // destroyed before the agent can connect with them.
    restoreOriginalDesktop(desktop);

    // The default security descriptor for a named pipe allows anyone to connect
    // to the pipe to read, but not to write.  Only the "creator owner" and
    // various system accounts can write to the pipe.  By sending and receiving
    // a dummy message on the control pipe, we should confirm that something
    // trusted (i.e. the agent we just started) successfully connected and wrote
    // to one of our pipes.
    WriteBuffer packet;
    packet.putInt(AgentMsg::Ping);
    writePacket(pc, packet);
    if (readInt32(pc) != 0) {
        delete pc;
        return NULL;
    }

    // TODO: On Windows Vista and forward, we could call
    // GetNamedPipeClientProcessId and verify that the PID is correct.  We could
    // also pass the PIPE_REJECT_REMOTE_CLIENTS flag on newer OS's.
    // TODO: I suppose this code is still subject to a denial-of-service attack
    // from untrusted accounts making read-only connections to the pipe.  It
    // should probably provide a SECURITY_DESCRIPTOR for the pipe, but the last
    // time I tried that (using SDDL), I couldn't get it to work (access denied
    // errors).

    // Aside: An obvious way to setup these handles is to open both ends of the
    // pipe in the parent process and let the child inherit its handles.
    // Unfortunately, the Windows API makes inheriting handles problematic.
    // MSDN says that handles have to be marked inheritable, and once they are,
    // they are inherited by any call to CreateProcess with
    // bInheritHandles==TRUE.  To avoid accidental inheritance, the library's
    // clients would be obligated not to create new processes while a thread
    // was calling winpty_open.  Moreover, to inherit handles, MSDN seems
    // to say that bInheritHandles must be TRUE[*], but I don't want to use a
    // TRUE bInheritHandles, because I want to avoid leaking handles into the
    // agent process, especially if the library someday allows creating the
    // agent process under a different user account.
    //
    // [*] The way that bInheritHandles and STARTF_USESTDHANDLES work together
    // is unclear in the documentation.  On one hand, for STARTF_USESTDHANDLES,
    // it says that bInheritHandles must be TRUE.  On Vista and up, isn't
    // PROC_THREAD_ATTRIBUTE_HANDLE_LIST an acceptable alternative to
    // bInheritHandles?  On the other hand, KB315939 contradicts the
    // STARTF_USESTDHANDLES documentation by saying, "Your pipe handles will
    // still be duplicated because Windows will always duplicate the STD
    // handles, even when bInheritHandles is set to FALSE."  IIRC, my testing
    // showed that the KB article was correct.

    return pc;
}

WINPTY_API winpty_t *winpty_open_use_own_datapipe(const wchar_t *dataPipe, int cols, int rows)
{

	winpty_t *pc = new winpty_t;

	// Setup pipes.
	std::wstringstream pipeName;
    pipeName << L"\\\\.\\pipe\\winpty-" << GetCurrentProcessId()
             << L"-" << InterlockedIncrement(&consoleCounter);
    std::wstring controlPipeName = pipeName.str() + L"-control";

	// The callee provides his own pipe implementation for handling sending/recieving
	// data between the started child process.
	std::wstring dataPipeName(to_wstring(to_utf8(dataPipe)));

	// Create control pipe
	pc->controlPipe = createNamedPipe(controlPipeName, false);
	if (pc->controlPipe == INVALID_HANDLE_VALUE) {
		delete pc;
		return NULL;
	}

	// Setup a background desktop for the agent.
	BackgroundDesktop desktop = setupBackgroundDesktop();

	// Start the agent.
	startAgentProcess(desktop, controlPipeName, dataPipeName, cols, rows);

	// Test control pipe connection.
	if (!connectNamedPipe(pc->controlPipe, false)) {
		delete pc;
		return NULL;
	}

	// Restore desktop.
	restoreOriginalDesktop(desktop);

	WriteBuffer packet;
	packet.putInt(AgentMsg::Ping);
	writePacket(pc, packet);
	if (readInt32(pc) != 0) {
		delete pc;
		return NULL;
	}

	return pc;
} 

WINPTY_API int winpty_start_process(winpty_t *pc,
                                    const wchar_t *appname,
                                    const wchar_t *cmdline,
                                    const wchar_t *cwd,
                                    const wchar_t *env)
{

    WriteBuffer packet;
    packet.putInt(AgentMsg::StartProcess);
    packet.putWString(appname ? appname : L"");
    packet.putWString(cmdline ? cmdline : L"");
    packet.putWString(cwd ? cwd : L"");
    std::wstring envStr;
    if (env != NULL) {
        const wchar_t *p = env;
        while (*p != L'\0') {
            p += wcslen(p) + 1;
        }
        p++;
        envStr.assign(env, p);

        // Can a Win32 environment be empty?  If so, does it end with one NUL or
        // two?  Add an extra NUL just in case it matters.
        envStr.push_back(L'\0');
    }
    packet.putWString(envStr);
    packet.putWString(getDesktopFullName());
    writePacket(pc, packet);
    return readInt32(pc);
}

WINPTY_API int winpty_get_exit_code(winpty_t *pc)
{
    WriteBuffer packet;
    packet.putInt(AgentMsg::GetExitCode);
    writePacket(pc, packet);
    return readInt32(pc);
}

WINPTY_API HANDLE winpty_get_data_pipe(winpty_t *pc)
{
    return pc->dataPipe;
}

WINPTY_API int winpty_set_size(winpty_t *pc, int cols, int rows)
{
    WriteBuffer packet;
    packet.putInt(AgentMsg::SetSize);
    packet.putInt(cols);
    packet.putInt(rows);
    writePacket(pc, packet);
    return readInt32(pc);
}

WINPTY_API void winpty_exit(winpty_t *pc)
{
	CloseHandle(pc->controlPipe);
	if(pc->dataPipe > 0) {
		CloseHandle(pc->dataPipe);
	}
}
