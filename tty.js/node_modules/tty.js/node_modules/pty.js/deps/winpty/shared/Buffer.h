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

#ifndef BUFFER_H
#define BUFFER_H

#include <sstream>
#include <iostream>

class WriteBuffer
{
private:
    std::stringstream ss;
public:
    void putInt(int i);
    void putWString(const std::wstring &str);
    void putWString(const wchar_t *str);
    std::string str() const;
};

inline void WriteBuffer::putInt(int i)
{
    ss.write((const char*)&i, sizeof(i));
}

inline void WriteBuffer::putWString(const std::wstring &str)
{
    putInt(str.size());
    ss.write((const char*)str.c_str(), sizeof(wchar_t) * str.size());
}

inline void WriteBuffer::putWString(const wchar_t *str)
{
    int len = wcslen(str);
    putInt(len);
    ss.write((const char*)str, sizeof(wchar_t) * len);
}

inline std::string WriteBuffer::str() const
{
    return ss.str();
}

class ReadBuffer
{
private:
    std::stringstream ss;
public:
    ReadBuffer(const std::string &packet);
    int getInt();
    std::wstring getWString();
    bool eof();
};

inline ReadBuffer::ReadBuffer(const std::string &packet) : ss(packet)
{
}

inline int ReadBuffer::getInt()
{
    int i;
    ss.read((char*)&i, sizeof(i));
    return i;
}

inline std::wstring ReadBuffer::getWString()
{
    int len = getInt();
    wchar_t *tmp = new wchar_t[len];
    ss.read((char*)tmp, sizeof(wchar_t) * len);
    std::wstring ret(tmp, len);
    delete [] tmp;
    return ret;
}

inline bool ReadBuffer::eof()
{
    ss.peek();
    return ss.eof();
}

#endif /* BUFFER_H */
