mount -o rw,remount /
ln -s /system/xbin /bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/gcc/lib; 
export PATH=/system/xbin:$PATH:/data/local/gcc/bin
