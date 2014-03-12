{
  'targets': [{
    'target_name': 'pty',
    'conditions': [
      ['OS=="win"', {
        'include_dirs' : [
          'deps/winpty/include',
        ],
        'dependencies' : [
          'deps/winpty/winpty.gyp:winpty-agent',
          'deps/winpty/winpty.gyp:winpty',
        ],
        'sources' : [
          'src/win/pty.cc'
        ]
      }, { # OS!="win"
        'sources': [
          'src/unix/pty.cc'
        ],
        'libraries': [
          '-lutil'
        ],
      }],
      # http://www.gnu.org/software/gnulib/manual/html_node/forkpty.html
      #   One some systems (at least including Cygwin, Interix,
      #   OSF/1 4 and 5, and Mac OS X) linking with -lutil is not required.
      ['OS=="mac" or OS=="solaris"', {
        'libraries!': [
          '-lutil'
        ]
      }],
    ]
  }]
}
