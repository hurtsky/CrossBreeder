#!/bin/sh
SRC=pixelserv30.c

CC="arm-elf-linux-androideabi-gcc" 
CFLAGS="-Os -s -Wall -ffunction-sections -fdata-sections -fno-strict-aliasing"
LDFLAGS="-Wl,--gc-sections"
STRIP="strip -s -R .note -R .comment -R .gnu.version -R .gnu.version_r"
OPTS="-O3 -DTEXT_REPLY -DDROP_ROOT -DREAD_FILE -DREAD_GIF"
BIN=pixelserv.host
$CC $CFLAGS $OPTS $SRC -o $BIN
$STRIP $BIN
ls -laF $BIN

OPTS="-O3 -DTINY"
BIN=pixelserv.tiny
$CC $CFLAGS $LDFLAGS $OPTS $SRC -o $BIN
$STRIP $BIN
ls -laF $BIN

