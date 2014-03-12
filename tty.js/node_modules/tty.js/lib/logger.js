/**
 * tty.js: logger.js
 * Copyright (c) 2012-2013, Christopher Jeffrey (MIT License)
 */

var slice = Array.prototype.slice
  , isatty = require('tty').isatty;

/**
 * Logger
 */

function logger(level) {
  var args = slice.call(arguments, 1);

  if (typeof args[0] !== 'string') args.unshift('');

  level = logger.levels[level];

  args[0] = '\x1b['
    + level[0]
    + 'm['
    + logger.prefix
    + ']\x1b[m '
    + args[0];

  if ((level[1] === 'log' && !logger.isatty[1])
      || (level[1] === 'error' && !logger.isatty[2])) {
    args[0] = args[0].replace(/\x1b\[(?:\d+(?:;\d+)*)?m/g, '');
  }

  return console[level[1]].apply(console, args);
}

logger.isatty = [isatty(0), isatty(1), isatty(2)];

logger.levels = {
  'log': [34, 'log'],
  'error': [41, 'error'],
  'warning': [31, 'error']
};

logger.prefix = 'tty.js';

logger.log = function() {
  return logger.apply(null, ['log'].concat(slice.call(arguments)));
};

logger.warning = function() {
  return logger.apply(null, ['warning'].concat(slice.call(arguments)));
};

logger.error = function() {
  return logger.apply(null, ['error'].concat(slice.call(arguments)));
};

/**
 * Expose
 */

module.exports = logger;
