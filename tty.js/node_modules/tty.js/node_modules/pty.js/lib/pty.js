/**
 * pty.js
 * Copyright (c) 2012, Christopher Jeffrey (MIT License)
 * Binding to the pseudo terminals.
 */

var net = require('net');
var tty = require('tty');
var pty = require('../build/Release/pty.node');

/**
 * Terminal
 */

// Example:
//  var term = new Terminal('bash', [], {
//    name: 'xterm-color',
//    cols: 80,
//    rows: 24,
//    cwd: process.env.HOME,
//    env: process.env
//  });

function Terminal(file, args, opt) {
  if (!(this instanceof Terminal)) {
    return new Terminal(file, args, opt);
  }

  var self = this
    , env
    , cwd
    , name
    , cols
    , rows
    , term;

  // backward compatibility
  if (typeof args === 'string') {
    opt = {
      name: arguments[1],
      cols: arguments[2],
      rows: arguments[3],
      cwd: process.env.HOME
    };
    args = [];
  }

  // arguments
  args = args || [];
  file = file || 'sh';
  opt = opt || {};

  cols = opt.cols || 80;
  rows = opt.rows || 24;
  env = clone(opt.env || process.env);
  cwd = opt.cwd || process.cwd();
  name = opt.name || env.TERM || 'xterm';

  env.TERM = name;
  env = environ(env);

  // fork
  term = opt.uid && opt.gid
    ? pty.fork(file, args, env, cwd, cols, rows, opt.uid, opt.gid)
    : pty.fork(file, args, env, cwd, cols, rows);

  this.socket = new tty.ReadStream(term.fd);
  this.socket.setEncoding('utf8');
  this.socket.resume();

  // setup
  this.socket.on('error', function(err) {
    // close
    self._close();

    // EIO, happens when someone closes our child
    // process: the only process in the terminal.
    // node < 0.6.14: errno 5
    // node >= 0.6.14: read EIO
    if (err.code) {
      if (~err.code.indexOf('errno 5')
          || ~err.code.indexOf('EIO')) return;
    }

    // throw anything else
    if (self.listeners('error').length < 2) {
      throw err;
    }
  });

  this.pid = term.pid;
  this.fd = term.fd;
  this.pty = term.pty;

  this.file = file;
  this.name = name;
  this.cols = cols;
  this.rows = rows;

  this.readable = true;
  this.writable = true;

  Terminal.total++;
  this.socket.on('close', function() {
    Terminal.total--;
    self._close();
    self.emit('exit', null);
  });

  env = null;
}

Terminal.fork =
Terminal.spawn =
Terminal.createTerminal = function(file, args, opt) {
  return new Terminal(file, args, opt);
};

/**
 * openpty
 */

Terminal.open = function(opt) {
  var self = Object.create(Terminal.prototype)
    , opt = opt || {};

  if (arguments.length > 1) {
    opt = {
      cols: arguments[1],
      rows: arguments[2]
    };
  }

  var cols = opt.cols || 80
    , rows = opt.rows || 24
    , term;

  // open
  term = pty.open(cols, rows);

  self.master = new net.Socket(term.master);
  self.master.setEncoding('utf8');
  self.master.resume();

  self.slave = new net.Socket(term.slave);
  self.slave.setEncoding('utf8');
  self.slave.resume();

  self.socket = self.master;
  self.pid = null;
  self.fd = term.master;
  self.pty = term.pty;

  self.file = process.argv[0] || 'node';
  self.name = process.env.TERM || '';
  self.cols = cols;
  self.rows = rows;

  self.readable = true;
  self.writable = true;

  self.socket.on('error', function(err) {
    self._close();
    if (self.listeners('error').length < 2) {
      throw err;
    }
  });

  Terminal.total++;
  self.socket.on('close', function() {
    Terminal.total--;
    self._close();
  });

  return self;
};

/**
 * Total
 */

// Keep track of the total
// number of terminals for
// the process.
Terminal.total = 0;

/**
 * Events
 */

// Don't inherit from net.Socket in
// order to avoid collisions.

Terminal.prototype.write = function(data) {
  return this.socket.write(data);
};

Terminal.prototype.end = function(data) {
  return this.socket.end(data);
};

Terminal.prototype.pipe = function(dest, options) {
  return this.socket.pipe(dest, options);
};

Terminal.prototype.pause = function() {
  this.socket.pause();
};

Terminal.prototype.resume = function() {
  this.socket.resume();
};

Terminal.prototype.setEncoding = function(enc) {
  if (this.socket._decoder) {
    delete this.socket._decoder;
  }
  if (enc) {
    this.socket.setEncoding(enc);
  }
};

Terminal.prototype.addListener =
Terminal.prototype.on = function(type, func) {
  this.socket.on(type, func);
  return this;
};

Terminal.prototype.emit = function() {
  return this.socket.emit.apply(this.socket, arguments);
};

Terminal.prototype.listeners = function(type) {
  return this.socket.listeners(type);
};

Terminal.prototype.removeListener = function(type, func) {
  this.socket.removeListener(type, func);
  return this;
};

Terminal.prototype.removeAllListeners = function(type) {
  this.socket.removeAllListeners(type);
  return this;
};

Terminal.prototype.once = function(type, func) {
  this.socket.once(type, func);
  return this;
};

Terminal.prototype.__defineGetter__('stdin', function() {
  return this;
});

Terminal.prototype.__defineGetter__('stdout', function() {
  return this;
});

Terminal.prototype.__defineGetter__('stderr', function() {
  throw new Error('No stderr.');
});

/**
 * TTY
 */

Terminal.prototype.resize = function(cols, rows) {
  cols = cols || 80;
  rows = rows || 24;

  this.cols = cols;
  this.rows = rows;

  pty.resize(this.fd, cols, rows);
};

Terminal.prototype.destroy = function() {
  var self = this;

  // close
  this._close();

  // Need to close the read stream so
  // node stops reading a dead file descriptor.
  // Then we can safely SIGTERM the shell.
  this.socket.once('close', function() {
    self.kill('SIGTERM');
  });

  this.socket.destroy();
};

Terminal.prototype.kill = function(sig) {
  try {
    process.kill(this.pid, sig || 'SIGTERM');
  } catch(e) {
    ;
  }
};

Terminal.prototype.__defineGetter__('process', function() {
  return pty.process(this.fd, this.pty) || this.file;
});

Terminal.prototype._close = function() {
  this.socket.writable = false;
  this.socket.readable = false;
  this.write = function() {};
  this.end = function() {};
  this.writable = false;
  this.readable = false;
};

/**
 * Helpers
 */

function clone(a) {
  var keys = Object.keys(a || {})
    , l = keys.length
    , i = 0
    , b = {};

  for (; i < l; i++) {
    b[keys[i]] = a[keys[i]];
  }

  return b;
}

function environ(env) {
  var keys = Object.keys(env || {})
    , l = keys.length
    , i = 0
    , pairs = [];

  for (; i < l; i++) {
    pairs.push(keys[i] + '=' + env[keys[i]]);
  }

  return pairs;
}

/**
 * Expose
 */

exports = Terminal;
exports.Terminal = Terminal;
exports.native = pty;
module.exports = exports;
