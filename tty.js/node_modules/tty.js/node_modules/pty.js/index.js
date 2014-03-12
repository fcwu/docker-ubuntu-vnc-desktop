var os = process.platform === 'win32' ? '_win' : '';
module.exports = require('./lib/pty'+ os +'.js');
