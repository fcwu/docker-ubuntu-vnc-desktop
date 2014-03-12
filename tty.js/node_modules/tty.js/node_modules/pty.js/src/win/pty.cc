/**
* pty.js
* Copyright (c) 2012, Christopher Jeffrey, Peter Sunde (MIT License)
*
* pty.cc:
*   This file is responsible for starting processes
*   with pseudo-terminal file descriptors.
*/

#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <string.h>
#include <stdlib.h>
#include <winpty.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

using namespace v8;
using namespace std;
using namespace node;

/**
* Misc
*/
extern "C" void init(Handle<Object>);

static std::vector<winpty_t *> ptyHandles;
static volatile LONG ptyCounter;

struct winpty_s {
	winpty_s();
	HANDLE controlPipe;
	HANDLE dataPipe;
};

winpty_s::winpty_s() : controlPipe(NULL), dataPipe(NULL)
{
}

/**
* Helpers
*/
const wchar_t* ToWChar(const String::Utf8Value& str)
{
	const char *bytes = *str;
	unsigned int iSizeOfStr = MultiByteToWideChar(CP_ACP, 0, bytes, -1, NULL, 0);  
	wchar_t* wszTgt = new wchar_t[iSizeOfStr];  	   
    MultiByteToWideChar(CP_ACP, 0, bytes, -1, wszTgt, iSizeOfStr);  
	return wszTgt;
}

template <typename T>
void remove(std::vector<T>& vec, size_t pos)
{
	std::vector<T>::iterator it = vec.begin();
	std::advance(it, pos);
	vec.erase(it);
}

static winpty_t *getControlPipeHandle(int handle) {
	for(unsigned int i = 0; i < ptyHandles.size(); i++) {
		winpty_t *ptyHandle = ptyHandles[i];
		if((int)ptyHandle->controlPipe == handle) {
			return ptyHandle;
		}
	}
	return NULL;
}

static bool removePipeHandle(int handle) {
	for(unsigned int i = 0; i < ptyHandles.size(); i++) {
		winpty_t *ptyHandle = ptyHandles[i];
		if((int)ptyHandle->controlPipe == handle) {
			remove(ptyHandles, i);
		}
	}
	return false;
}

/*
* PtyOpen
* pty.open(dataPipe, cols, rows)
* 
* If you need to debug winpty-agent.exe do the following:
* ======================================================
* 
* 1) Install python 2.7
* 2) Install http://sourceforge.net/projects/pywin32/
* 3) Start deps/winpty/misc/DebugServer.py (Before you start node)
* 
* Then you'll see output from winpty-agent.exe.
* 
* Important part:
* ===============
* CreateProcess: success 8896 0 (Windows error code)
* 
* Create test.js:
* ===============
*
* var pty = require('./');
*
* var term = pty.fork('cmd', [], {
*   name: 'Windows Shell',
*	cols: 80,
*	rows: 30,
*	cwd: process.env.HOME,
*	env: process.env,
*	debug: true
* });
*
* process.stdin.pipe(term);
* process.stdin.resume();
* term.pipe(process.stdout);
*
*/

static Handle<Value> PtyOpen(const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 4
		|| !args[0]->IsString() // dataPipe
		|| !args[1]->IsNumber() // cols
		|| !args[2]->IsNumber() // rows
		|| !args[3]->IsBoolean()) // debug
	{
		return ThrowException(Exception::Error(
			String::New("Usage: pty.open(dataPipe, cols, rows, debug)")));
	}

	// Cols, rows, debug
	int cols = (int) args[1]->Int32Value();
	int rows = (int) args[2]->Int32Value();
	bool debug = (bool) args[3]->BooleanValue;

	// If debug is enabled, set environment variable
	if(debug) {
		SetEnvironmentVariableW(L"WINPTYDBG", L"1");
	}

	// Open a new pty session.
	winpty_t *pc = winpty_open_use_own_datapipe(ToWChar(String::Utf8Value(args[0]->ToString())), rows, cols);

	// Error occured during startup of agent process.
	if(pc == NULL) {
		return ThrowException(Exception::Error(String::New("Unable to start agent process.")));
	}

	// Save pty struct fpr later use.
	ptyHandles.insert(ptyHandles.end(), pc);

	// Pty object values.
	Local<Object> obj = Object::New();

	// Control pipe handle.
	obj->Set(String::New("pid"), Number::New((int)pc->controlPipe));

	// File descriptor is not available on Windows.
	obj->Set(String::New("fd"), Number::New(-1));

	// Id of current pty session.
	obj->Set(String::New("pty"), Number::New(InterlockedIncrement(&ptyCounter)));

	return scope.Close(obj);

}

/*
* PtyStartProcess
* pty.startProcess(pid, file, env, cwd);
*/

static Handle<Value> PtyStartProcess(const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 5
		|| !args[0]->IsNumber() // pid
		|| !args[1]->IsString() // file
		|| !args[2]->IsString() // cmdline
		|| !args[3]->IsString() // env
		|| !args[4]->IsString()) // cwd
	{
		return ThrowException(Exception::Error(
			String::New("Usage: pty.startProcess(pid, file, cmdline, env, cwd)")));
	}

	// Native values.
	const wchar_t *file = ToWChar(String::Utf8Value(args[1]->ToString()));
	const wchar_t *cmdline = ToWChar(String::Utf8Value(args[2]->ToString()));
	const wchar_t *env = ToWChar(String::Utf8Value(args[3]->ToString()));
	const wchar_t *cwd = ToWChar(String::Utf8Value(args[4]->ToString()));

	// Get winpty_t by control pipe handle
	winpty_t *pc = getControlPipeHandle((int) args[0]->Int32Value());
	
	// Start new terminal
	if(pc != NULL) {
		winpty_start_process(pc, file, cmdline, cwd, env);	
	} else {
		return ThrowException(Exception::Error(
			String::New("Invalid control pipe handle.")));
	}

	return scope.Close(Undefined());
}

/*
* PtyResize
* pty.resize(pid, cols, rows);
*/
static Handle<Value> PtyResize(const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 3
		|| !args[0]->IsNumber() // pid
		|| !args[1]->IsNumber() // cols
		|| !args[2]->IsNumber()) // rows
	{
		return ThrowException(Exception::Error(
			String::New("Usage: pty.resize(pid, cols, rows)")));
	}

	int handle = (int) args[0]->Int32Value();
	int cols = (int) args[1]->Int32Value();
	int rows = (int) args[2]->Int32Value();

	winpty_t *pc = getControlPipeHandle(handle);

	if(pc == NULL) {
		return ThrowException(Exception::Error(String::New("Invalid pid.")));
	}

	winpty_set_size(pc, cols, rows);

	return scope.Close(Undefined());
}

/*
* PtyKill
* pty.kill(pid);
*/
static Handle<Value> PtyKill(const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 1
		|| !args[0]->IsNumber()) // pid
	{
		return ThrowException(Exception::Error(
			String::New("Usage: pty.kill(pid)")));
	}

	int handle = (int) args[0]->Int32Value();

	winpty_t *pc = getControlPipeHandle(handle);

	if(pc == NULL) {
		return ThrowException(Exception::Error(
			String::New("Invalid pid.")));
	}

	winpty_exit(pc);
	removePipeHandle(handle);

	return scope.Close(Undefined());

}

/**
* Init
*/

extern "C" void init(Handle<Object> target) {
	HandleScope scope;
	NODE_SET_METHOD(target, "open", PtyOpen);
	NODE_SET_METHOD(target, "startProcess", PtyStartProcess);
	NODE_SET_METHOD(target, "resize", PtyResize);
	NODE_SET_METHOD(target, "kill", PtyKill);
};

NODE_MODULE(pty, init);
