export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib; export PATH=$PATH:/data/local/gcc/bin

arm-elf-linux-androideabi-gcc -std=gnu99 -Wall -I /data/local/gcc/include -O3 -Wl,-rpath -Wl,/usr/local/lib -o CB_Check_Net_DNS CB_Check_Net_DNS.c popen_noshell.c ; strip CB_Check_Net_DNS
