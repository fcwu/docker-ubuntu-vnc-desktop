#!/usr/bin/env node

process.title = 'tty.js';

var tty = require('../');

var conf = tty.config.readConfig()
  , app = tty.createServer(conf);

app.listen();
