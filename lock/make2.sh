export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib; export PATH=$PATH:/data/local/gcc/bin

arm-elf-linux-androideabi-gcc -std=gnu99 -Wall -I /data/local/gcc/include -O3 -Wl,-rpath -Wl,/data/local/gcc/lib CB_Check_Net_DNS.c -o CB_Check_Net_DNS
strip CB_Check_Net_DNS
