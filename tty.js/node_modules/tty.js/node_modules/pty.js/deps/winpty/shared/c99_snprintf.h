// Copyright (c) 2012 Ryan Prichard
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

#ifndef C99_SNPRINTF_H
#define C99_SNPRINTF_H

#include <stdarg.h>
#include <stdio.h>

#ifdef _MSC_VER

/* MSVC does not define C99's va_copy, so define one for it.  It appears that
 * with MSVC, a va_list is a char*, not an array type, so va_copy is a simple
 * assignment.  On MSVC 2012, there is a VC/include/vadefs.h file defining
 * va_list and the va_XXX routines, which has code for x86, x86-64, and ARM. */
#define c99_va_copy(dest, src) ((dest) = (src))

static inline int c99_vsnprintf(
        char* str,
        size_t size,
        const char* format,
        va_list ap)
{
    va_list apcopy;
    int count = -1;
    if (size != 0) {
        c99_va_copy(apcopy, ap);
        count = _vsnprintf_s(str, size, _TRUNCATE, format, apcopy);
        va_end(apcopy);
    }
    if (count == -1) {
        c99_va_copy(apcopy, ap);
        count = _vscprintf(format, apcopy);
        va_end(apcopy);
    }
    return count;
}

#else

#define c99_va_copy(dest, src) (va_copy(dest, src))

static inline int c99_vsnprintf(
        char* str,
        size_t size,
        const char* format,
        va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

#endif /* _MSC_VER */

static inline int c99_snprintf(
        char* str,
        size_t size,
        const char* format,
        ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}

#endif /* C99_SNPRINTF_H */
