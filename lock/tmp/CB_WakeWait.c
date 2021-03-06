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

int main(int argc, char *argv[]) {

sigset_t mask; 
sigfillset(&mask); 
sigprocmask(SIG_SETMASK, &mask, NULL);

struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
fl.l_pid = getpid();

int pid_file = open("/dev/CHECK_NET_DNS.RUN", O_CREAT | O_RDWR, 0644);

int rc = fcntl(pid_file, F_SETLK, &fl);
if ( rc == -1 ) {
      printf("Couldn't lock PID file \"/dev/CHECK_NET_DNS.RUN\" for writing.\n");
    return 1;
}

char pid[8],buf[1024],command[1024];
sprintf(pid,"%i",getpid());
write(pid_file, pid, strlen(pid));

if ( ! readlink("/proc/self/exe", buf, sizeof(buf)-1)) 
{
  sprintf(buf,"/system/etc/CrossBreeder/CB_WakeWait");
}
char *index=strrchr(buf,'/');
if ( index != NULL ) index[0]=0;

//for(int i=0; i<=15; i++) {
//if (signal(i, SIG_IGN) == SIG_ERR)
//  printf("\ncan't catch %d\n",i); 
//}

int sleep_fd = -1 ; int len = -1;
char sleep_read_buffer;
FILE *fp; char path[1035];
int repeat=5;
int count=0;
int startup=0;
for (;;) {
repeat=5;

int iReturn = __system_property_get("net.dns1",path);

if ( ! iReturn ) {
  fp = popen("getprop net.dns1 2>/dev/null", "r"); 
  fgets(path, sizeof(path)-1, fp);
  pclose(fp);
  repeat=10;
}

if ( strncmp(path,"0.0.0.0",7) ) { 
  sprintf(command,"%s/busybox timeout -t 60 -s 9 %s/zzCHECK_NET_DNS",buf,buf);
  system(command);
}

sleep(repeat); 
count+=repeat;

if ( startup == 0 ) {

iReturn = __system_property_get("sys.boot_completed",path);

if ( ! iReturn ) {
  fp = popen("getprop sys.boot_completed 2>/dev/null", "r");
  fgets(path, sizeof(path)-1, fp);
  pclose(fp);
}

if ( strncmp(path,"1",1) || ( count > 60 ) ) {
  startup = 1;
  sprintf(command,"%s/busybox timeout -t 120 -s 9 %s/zzCrossBreeder FORCE &",buf,buf);
  system(command);
}

}

if ( count > 60 ) { 
count=0;
sleep_fd = open ( "/sys/power/wait_for_fb_wake" , O_RDONLY, 0);
        if (sleep_fd != -1 )
        {
         do {
             len = read (sleep_fd, &sleep_read_buffer, 1);
            } while ( len < 0 && errno == EINTR );
        close(sleep_fd);
        }
}
}

return 0;
}
