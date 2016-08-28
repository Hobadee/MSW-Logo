del *.lob
rem path=C:\LINT;%PATH%
lint opt.lnt -os(lint.out) files.lnt
rem lint opt.lnt -u -os(lint.out) unix.cpp -oo(unix.lob)
rem lint opt.lnt -u +os(lint.out) lint.cpp -oo(lint.lob)
rem lint opt.lnt +os(lint.out) unix.lob
