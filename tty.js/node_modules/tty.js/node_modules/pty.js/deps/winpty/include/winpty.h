/*
 * Copyright (c) 2011-2012 Ryan Prichard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef WINPTY_H
#define WINPTY_H

#include <stdlib.h>
#include <windows.h>

#ifdef WINPTY
#define WINPTY_API __declspec(dllexport)
#else
#define WINPTY_API __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct winpty_s winpty_t;

/*
 * winpty API.
 */

/*
 * Starts a new winpty instance with the given size.
 *
 * This function creates a new agent process and connects to it.
 */
WINPTY_API winpty_t *winpty_open(int cols, int rows);

/**
* Starts a new winpty instance with the given size.
* 
* This function creates a new agent process and connects to it. 
* By using this method you are responsible for creating your own named
* pipe server for communicating with the child process.
* 
*/
WINPTY_API winpty_t *winpty_open_use_own_datapipe(const wchar_t *dataPipe, int cols, int rows);

/*
 * Start a child process.  Either (but not both) of appname and cmdline may
 * be NULL.  cwd and env may be NULL.  env is a pointer to an environment
 * block like that passed to CreateProcess.
 *
 * This function never modifies the cmdline, unlike CreateProcess.
 *
 * Only one child process may be started.  After the child process exits, the
 * agent will scrape the console output one last time, then close the data pipe
 * once all remaining data has been sent.
 *
 * Returns 0 on success or a Win32 error code on failure.
 */
WINPTY_API int winpty_start_process(winpty_t *pc,
                                    const wchar_t *appname,
                                    const wchar_t *cmdline,
                                    const wchar_t *cwd,
                                    const wchar_t *env);

/*
 * Returns the exit code of the process started with winpty_start_process,
 * or -1 none is available.
 */
WINPTY_API int winpty_get_exit_code(winpty_t *pc);

/*
 * Returns an overlapped-mode pipe handle that can be read and written
 * like a Unix terminal.
 */
WINPTY_API HANDLE winpty_get_data_pipe(winpty_t *pc);

/*
 * Change the size of the Windows console.
 */
WINPTY_API int winpty_set_size(winpty_t *pc, int cols, int rows);

/*
 * Closes the winpty.
 */
WINPTY_API void winpty_exit(winpty_t *pc);

#ifdef __cplusplus
}
#endif

#endif /* WINPTY_H */
