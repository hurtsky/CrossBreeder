#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h> /* idcrisis */

#include <unistd.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/random.h>

#include <errno.h>

#include <sys/system_properties.h>

void sig_handler(int signo) { 

for(int i=0; i<=15; i++) {
if (signo == i) 
  printf("received %d\n",i); 
}
}

char buf[1024],command[1024];

int main(int argc, char *argv[]) {

//sigset_t mask; 
//sigfillset(&mask); 
//sigprocmask(SIG_SETMASK, &mask, NULL);
close(STDIN_FILENO);
close(STDOUT_FILENO);

struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
fl.l_pid = getpid();

int pid_file = open("/dev/CHECK_NET_DNS.RUN", O_CREAT | O_RDWR, 0644);

int rc = fcntl(pid_file, F_SETLK, &fl);
if ( rc == -1 ) {
      printf("Couldn't lock PID file \"/dev/CHECK_NET_DNS.RUN\" for writing.\n");
    return 1;
}

if ( ! readlink("/proc/self/exe", buf, sizeof(buf)-1)) 
{
  sprintf(buf,"/system/etc/CrossBreeder/CB_WakeWait");
}
char *index=strrchr(buf,'/');
if ( index != NULL ) index[0]=0;

if ( argc == 2 ) {
  printf("%s/busybox timeout -t 300 -s KILL %s/zzCHECK_NET_DNS %s",buf,buf,argv[1]);
  sprintf(command,"%s/busybox timeout -t 300 -s KILL %s/zzCHECK_NET_DNS %s",buf,buf,argv[1]);
  system(command);
} else
{
  printf("%s/busybox timeout -t 300 -s KILL %s/zzCHECK_NET_DNS SET",buf,buf);
  sprintf(command,"%s/busybox timeout -t 300 -s KILL %s/zzCHECK_NET_DNS SET",buf,buf);
  system(command);
} 
return 0;
}
