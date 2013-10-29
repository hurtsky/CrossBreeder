export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib; export PATH=$PATH:/data/local/gcc/bin

arm-elf-linux-androideabi-gcc -Wall -I /data/local/gcc/include -O3 -Wl,-rpath -Wl,/usr/local/lib -o CB_FileLock CB_FileLock.c ; strip CB_FileLock
