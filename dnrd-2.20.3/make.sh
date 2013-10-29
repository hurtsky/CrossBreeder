#!/system/bin/sh
busybox mount -o rw,remount /; ln -s /system/xbin /bin; mkdir /tmp;

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib:/botbrew/lib ; export PATH=/system/xbin:$PATH:/data/local/gcc/bin:/data/local/perl/:/botbrew/bin/

make clean
./configure CC=arm-elf-linux-androideabi-gcc CPPFLAGS="-I /data/local/gcc/include" --enable-tcp=no
make CC=arm-elf-linux-androideabi-gcc
cd src
arm-elf-linux-androideabi-gcc -g -O3 -o dnrd args.o cache.o common.o dns.o lib.o main.o master.o query.o relay.o sig.o tcp.o udp.o srvnode.o domnode.o rand.o qid.o check.o
strip dnrd
