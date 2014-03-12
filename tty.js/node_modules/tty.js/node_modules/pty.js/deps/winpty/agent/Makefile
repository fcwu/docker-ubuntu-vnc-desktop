# Copyright (c) 2011-2012 Ryan Prichard
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

include ../config.mk
include ../config-mingw.mk

PROGRAM = ../build/winpty-agent.exe

OBJECTS = \
	EventLoop.o \
	NamedPipe.o \
	Agent.o \
	AgentAssert.o \
	Terminal.o \
	Win32Console.o \
	ConsoleInput.o \
	DebugClient.o \
	Coord.o \
	SmallRect.o \
	main.o

CXXFLAGS += \
	-DUNICODE \
	-D_UNICODE \
	-D_WIN32_WINNT=0x0501 \
	-fno-exceptions \
	-fno-rtti

LDFLAGS += -static-libgcc -static-libstdc++

all : $(PROGRAM)

$(PROGRAM) : $(OBJECTS)
	@echo Linking $@
	@$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean:
	rm -f $(PROGRAM) *.o *.d

-include $(OBJECTS:.o=.d)
