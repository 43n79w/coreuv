SHELL=/bin/sh
#CC=clang `gnustep-config --objc-flags` `gnustep-config --objc-libs`
CC=clang
#CC=clang `gnustep-config --base-libs`

all: coreuv-utils.o coreuv.o
	$(CC) -o main test/main.m coreuv.o http-parser/http_parser.o coreuv-utils.o -x c -fblocks -l:libgnustep-corebase.so.0.1.1 -lgnustep-base -lgnustep-gui -ldispatch -lcurl -ltidy -lxml2 -luv -I/usr/local/include -L/usr/local/lib -I/usr/include/libxml2 -L/usr/lib -I/srv/coreuv/http-parser -L/srv/coreuv/http-parser

coreuv.o:
	$(CC) -c -o coreuv.o -x c coreuv.c -fblocks -l:libgnustep-corebase.so.0.1 -lgnustep-base -lgnustep-corebase -lgnustep-gui -ldispatch -luv -I/usr/local/include -L/usr/local/lib -I/usr/include/libxml2 -L/usr/lib -I/srv/coreuv/http-parser -L/srv/coreuv/http-parser

coreuv-utils.o:
	$(CC) -c -x c coreuv-utils.c -fblocks -I/usr/local/include -L/usr/local/lib

#CoreJSON.o:
#	$(CC) -c -x objective-c CoreJSON.c -fblocks -I/usr/local/include -L/usr/local/lib

#http_parser:
#	$(CC) -o http_parser -c http_parser.c -x c -fblocks -I/usr/local/include -L/usr/local/lib
