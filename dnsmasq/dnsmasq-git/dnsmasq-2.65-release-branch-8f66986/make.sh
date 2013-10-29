#!/system/bin/sh
busybox mount -o rw,remount /; ln -s /system/xbin /bin; mkdir /tmp; export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib:/botbrew/lib ; export PATH=/system/xbin:$PATH:/data/local/gcc/bin:/data/local/perl/:/botbrew/bin/
make CC=arm-elf-linux-androideabi-gcc LDFLAGS="-static-libgcc -llog" CFLAGS="-DNO_IPV6 -O3 -static -D__ANDROID__ "
