busybox mount -o rw,remount /
ln -s /system/xbin /bin; 
mkdir /tmp; 

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib; 
export PATH=/system/xbin:$PATH:/data/local/gcc/bin:/data/local/perl/:/botbrew/bin/

make
#arm-elf-linux-androideabi-gcc -Wall -I.. -g -O3 -o .libs/haveged.static haveged.o .libs/libhavege.a -Wl,-rpath -Wl,/usr/local/lib; strip .libs/haveged.static
arm-elf-linux-androideabi-gcc -Wall -I.. -O3 -static-libgcc -o .libs/haveged.static haveged.o .libs/libhavege.a -Wl,-rpath -Wl,/usr/local/lib -lm; strip .libs/haveged.static
arm-elf-linux-androideabi-gcc -Wall -I.. -O3 -static-libgcc -o .libs/haveged haveged.o ../src/.libs/libhavege.a -lm; strip .libs/haveged
