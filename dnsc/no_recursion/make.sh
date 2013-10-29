export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib:/botbrew/lib ; export PATH=/system/xbin:$PATH:/data/local/gcc/bin:/data/local/perl/:/botbrew/bin/

arm-elf-linux-androideabi-gcc -Wall -I/usr/local/gcc/include -g -O -o dnsc dnsc.c
strip dnsc
