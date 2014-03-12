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

#include "Agent.h"
#include "AgentAssert.h"
#include <stdio.h>
#include <stdlib.h>

static wchar_t *heapMbsToWcs(const char *text)
{
    size_t len = mbstowcs(NULL, text, 0);
    ASSERT(len != (size_t)-1);
    wchar_t *ret = new wchar_t[len + 1];
    size_t len2 = mbstowcs(ret, text, len + 1);
    ASSERT(len == len2);
    return ret;
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr,
            "Usage: %s controlPipeName dataPipeName cols rows\n"
            "(Note: This program is intended to be run by libwinpty.dll.)\n",
            argv[0]);
        return 1;
    }

    Agent agent(heapMbsToWcs(argv[1]),
                heapMbsToWcs(argv[2]),
                atoi(argv[3]),
                atoi(argv[4]));
    agent.run();
}
