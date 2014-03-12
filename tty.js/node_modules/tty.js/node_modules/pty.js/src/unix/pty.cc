/**
 * pty.js
 * Copyright (c) 2012, Christopher Jeffrey (MIT License)
 *
 * pty.cc:
 *   This file is responsible for starting processes
 *   with pseudo-terminal file descriptors.
 *
 * See:
 *   man pty
 *   man tty_ioctl
 *   man termios
 *   man forkpty
 */

#include <v8.h>
#include <node.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* forkpty */
/* http://www.gnu.org/software/gnulib/manual/html_node/forkpty.html */
#if defined(__GLIBC__) || defined(__CYGWIN__)
#include <pty.h>
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <util.h>
#elif defined(__FreeBSD__)
#include <libutil.h>
#elif defined(__sun)
#include <stropts.h> /* for I_PUSH */
#else
#include <pty.h>
#endif

#include <termios.h> /* tcgetattr, tty_ioctl */

/* environ for execvpe */
/* node/src/node_child_process.cc */
#if defined(__APPLE__) && !TARGET_OS_IPHONE
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

/* for pty_getproc */
#if defined(__linux__)
#include <stdio.h>
#include <stdint.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <libproc.h>
#endif

using namespace node;
using namespace v8;

static Handle<Value>
PtyFork(const Arguments&);

static Handle<Value>
PtyOpen(const Arguments&);

static Handle<Value>
PtyResize(const Arguments&);

static Handle<Value>
PtyGetProc(const Arguments&);

static int
pty_execvpe(const char *, char **, char **);

static int
pty_nonblock(int);

static char *
pty_getproc(int, char *);

static int
pty_openpty(int *, int *, char *,
            const struct termios *,
            const struct winsize *);

static pid_t
pty_forkpty(int *, char *,
            const struct termios *,
            const struct winsize *);

extern "C" void
init(Handle<Object>);

/**
 * PtyFork
 * pty.fork(file, args, env, cwd, cols, rows)
 */

static Handle<Value>
PtyFork(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 6
      || !args[0]->IsString()
      || !args[1]->IsArray()
      || !args[2]->IsArray()
      || !args[3]->IsString()
      || !args[4]->IsNumber()
      || !args[5]->IsNumber()
      || (args.Length() >= 8 && !args[6]->IsNumber())
      || (args.Length() >= 8 && !args[7]->IsNumber())) {
    return ThrowException(Exception::Error(
      String::New("Usage: pty.fork(file, args, env, cwd, cols, rows[, uid, gid])")));
  }

  // node/src/node_child_process.cc

  // file
  String::Utf8Value file(args[0]->ToString());

  // args
  int i = 0;
  Local<Array> argv_ = Local<Array>::Cast(args[1]);
  int argc = argv_->Length();
  int argl = argc + 1 + 1;
  char **argv = new char*[argl];
  argv[0] = strdup(*file);
  argv[argl-1] = NULL;
  for (; i < argc; i++) {
    String::Utf8Value arg(argv_->Get(Integer::New(i))->ToString());
    argv[i+1] = strdup(*arg);
  }

  // env
  i = 0;
  Local<Array> env_ = Local<Array>::Cast(args[2]);
  int envc = env_->Length();
  char **env = new char*[envc+1];
  env[envc] = NULL;
  for (; i < envc; i++) {
    String::Utf8Value pair(env_->Get(Integer::New(i))->ToString());
    env[i] = strdup(*pair);
  }

  // cwd
  String::Utf8Value cwd_(args[3]->ToString());
  char *cwd = strdup(*cwd_);

  // size
  struct winsize winp;
  winp.ws_col = args[4]->IntegerValue();
  winp.ws_row = args[5]->IntegerValue();
  winp.ws_xpixel = 0;
  winp.ws_ypixel = 0;

  // uid / gid
  int uid = -1;
  int gid = -1;
  if (args.Length() >= 8) {
    uid = args[6]->IntegerValue();
    gid = args[7]->IntegerValue();
  }

  // fork the pty
  int master = -1;
  char name[40];
  pid_t pid = pty_forkpty(&master, name, NULL, &winp);

  if (pid) {
    for (i = 0; i < argl; i++) free(argv[i]);
    delete[] argv;
    for (i = 0; i < envc; i++) free(env[i]);
    delete[] env;
    free(cwd);
  }

  switch (pid) {
    case -1:
      return ThrowException(Exception::Error(
        String::New("forkpty(3) failed.")));
    case 0:
      if (strlen(cwd)) chdir(cwd);

      if (uid != -1 && gid != -1) {
        if (setgid(gid) == -1) {
          perror("setgid(2) failed.");
          _exit(1);
        }
        if (setuid(uid) == -1) {
          perror("setuid(2) failed.");
          _exit(1);
        }
      }

      pty_execvpe(argv[0], argv, env);

      perror("execvp(3) failed.");
      _exit(1);
    default:
      if (pty_nonblock(master) == -1) {
        return ThrowException(Exception::Error(
          String::New("Could not set master fd to nonblocking.")));
      }

      Local<Object> obj = Object::New();
      obj->Set(String::New("fd"), Number::New(master));
      obj->Set(String::New("pid"), Number::New(pid));
      obj->Set(String::New("pty"), String::New(name));

      return scope.Close(obj);
  }

  return Undefined();
}

/**
 * PtyOpen
 * pty.open(cols, rows)
 */

static Handle<Value>
PtyOpen(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 2
      || !args[0]->IsNumber()
      || !args[1]->IsNumber()) {
    return ThrowException(Exception::Error(
      String::New("Usage: pty.open(cols, rows)")));
  }

  // size
  struct winsize winp;
  winp.ws_col = args[0]->IntegerValue();
  winp.ws_row = args[1]->IntegerValue();
  winp.ws_xpixel = 0;
  winp.ws_ypixel = 0;

  // pty
  int master, slave;
  char name[40];
  int ret = pty_openpty(&master, &slave, name, NULL, &winp);

  if (ret == -1) {
    return ThrowException(Exception::Error(
      String::New("openpty(3) failed.")));
  }

  if (pty_nonblock(master) == -1) {
    return ThrowException(Exception::Error(
      String::New("Could not set master fd to nonblocking.")));
  }

  if (pty_nonblock(slave) == -1) {
    return ThrowException(Exception::Error(
      String::New("Could not set slave fd to nonblocking.")));
  }

  Local<Object> obj = Object::New();
  obj->Set(String::New("master"), Number::New(master));
  obj->Set(String::New("slave"), Number::New(slave));
  obj->Set(String::New("pty"), String::New(name));

  return scope.Close(obj);
}

/**
 * Resize Functionality
 * pty.resize(fd, cols, rows)
 */

static Handle<Value>
PtyResize(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 3
      || !args[0]->IsNumber()
      || !args[1]->IsNumber()
      || !args[2]->IsNumber()) {
    return ThrowException(Exception::Error(
      String::New("Usage: pty.resize(fd, cols, rows)")));
  }

  int fd = args[0]->IntegerValue();

  struct winsize winp;
  winp.ws_col = args[1]->IntegerValue();
  winp.ws_row = args[2]->IntegerValue();
  winp.ws_xpixel = 0;
  winp.ws_ypixel = 0;

  if (ioctl(fd, TIOCSWINSZ, &winp) == -1) {
    return ThrowException(Exception::Error(
      String::New("ioctl(2) failed.")));
  }

  return Undefined();
}

/**
 * PtyGetProc
 * Foreground Process Name
 * pty.process(fd, tty)
 */

static Handle<Value>
PtyGetProc(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 2
      || !args[0]->IsNumber()
      || !args[1]->IsString()) {
    return ThrowException(Exception::Error(
      String::New("Usage: pty.process(fd, tty)")));
  }

  int fd = args[0]->IntegerValue();

  String::Utf8Value tty_(args[1]->ToString());
  char *tty = strdup(*tty_);
  char *name = pty_getproc(fd, tty);
  free(tty);

  if (name == NULL) {
    return Undefined();
  }

  Local<String> name_ = String::New(name);
  free(name);
  return scope.Close(name_);
}

/**
 * execvpe
 */

// execvpe(3) is not portable.
// http://www.gnu.org/software/gnulib/manual/html_node/execvpe.html
static int
pty_execvpe(const char *file, char **argv, char **envp) {
  char **old = environ;
  environ = envp;
  int ret = execvp(file, argv);
  environ = old;
  return ret;
}

/**
 * Nonblocking FD
 */

static int
pty_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * pty_getproc
 * Taken from tmux.
 */

// Taken from: tmux (http://tmux.sourceforge.net/)
// Copyright (c) 2009 Nicholas Marriott <nicm@users.sourceforge.net>
// Copyright (c) 2009 Joshua Elsasser <josh@elsasser.org>
// Copyright (c) 2009 Todd Carson <toc@daybefore.net>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
// IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
// OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#if defined(__linux__)

static char *
pty_getproc(int fd, char *tty) {
  FILE *f;
  char *path, *buf;
  size_t len;
  int ch;
  pid_t pgrp;
  int r;

  if ((pgrp = tcgetpgrp(fd)) == -1) {
    return NULL;
  }

  r = asprintf(&path, "/proc/%lld/cmdline", (long long)pgrp);
  if (r == -1 || path == NULL) return NULL;

  if ((f = fopen(path, "r")) == NULL) {
    free(path);
    return NULL;
  }

  free(path);

  len = 0;
  buf = NULL;
  while ((ch = fgetc(f)) != EOF) {
    if (ch == '\0') break;
    buf = (char *)realloc(buf, len + 2);
    if (buf == NULL) return NULL;
    buf[len++] = ch;
  }

  if (buf != NULL) {
    buf[len] = '\0';
  }

  fclose(f);
  return buf;
}

#elif defined(__APPLE__)

static char *
pty_getproc(int fd, char *tty) {
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, 0 };
  size_t size;
  struct kinfo_proc kp;

  if ((mib[3] = tcgetpgrp(fd)) == -1) {
    return NULL;
  }

  size = sizeof kp;
  if (sysctl(mib, 4, &kp, &size, NULL, 0) == -1) {
    return NULL;
  }

  if (*kp.kp_proc.p_comm == '\0') {
    return NULL;
  }

  return strdup(kp.kp_proc.p_comm);
}

#else

static char *
pty_getproc(int fd, char *tty) {
  return NULL;
}

#endif

/**
 * openpty(3) / forkpty(3)
 */

static int
pty_openpty(int *amaster, int *aslave, char *name,
            const struct termios *termp,
            const struct winsize *winp) {
#if defined(__sun)
  char *slave_name;
  int slave;
  int master = open("/dev/ptmx", O_RDWR | O_NOCTTY);
  if (master == -1) return -1;
  if (amaster) *amaster = master;

  if (grantpt(master) == -1) goto err;
  if (unlockpt(master) == -1) goto err;

  slave_name = ptsname(master);
  if (slave_name == NULL) goto err;
  if (name) strcpy(name, slave_name);

  slave = open(slave_name, O_RDWR | O_NOCTTY);
  if (slave == -1) goto err;
  if (aslave) *aslave = slave;

  ioctl(slave, I_PUSH, "ptem");
  ioctl(slave, I_PUSH, "ldterm");
  ioctl(slave, I_PUSH, "ttcompat");

  if (termp) tcsetattr(slave, TCSAFLUSH, termp);
  if (winp) ioctl(slave, TIOCSWINSZ, winp);

  return 0;

err:
  close(master);
  return -1;
#else
  return openpty(amaster, aslave, name, (termios *)termp, (winsize *)winp);
#endif
}

static pid_t
pty_forkpty(int *amaster, char *name,
            const struct termios *termp,
            const struct winsize *winp) {
#if defined(__sun)
  int master, slave;

  int ret = pty_openpty(&master, &slave, name, termp, winp);
  if (ret == -1) return -1;
  if (amaster) *amaster = master;

  pid_t pid = fork();

  switch (pid) {
    case -1:
      close(master);
      close(slave);
      return -1;
    case 0:
      close(master);

      setsid();

#if defined(TIOCSCTTY)
      // glibc does this
      if (ioctl(slave, TIOCSCTTY, NULL) == -1) _exit(1);
#endif

      dup2(slave, 0);
      dup2(slave, 1);
      dup2(slave, 2);

      if (slave > 2) close(slave);

      return 0;
    default:
      close(slave);
      return pid;
  }

  return -1;
#else
  return forkpty(amaster, name, (termios *)termp, (winsize *)winp);
#endif
}

/**
 * Init
 */

extern "C" void
init(Handle<Object> target) {
  HandleScope scope;
  NODE_SET_METHOD(target, "fork", PtyFork);
  NODE_SET_METHOD(target, "open", PtyOpen);
  NODE_SET_METHOD(target, "resize", PtyResize);
  NODE_SET_METHOD(target, "process", PtyGetProc);
}

NODE_MODULE(pty, init)
