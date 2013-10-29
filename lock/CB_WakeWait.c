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

//char pid[8];
char buf[1024],command[1024];
//sprintf(pid,"%i",getpid());
//write(pid_file, pid, strlen(pid));

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
FILE *fp; char path[1035],path2[1035];
FILE *fptr_WRITE_WAKEUP_THRESHOLD;
int repeat; repeat=5;
int count; count=10;
int startup; startup=0;
int i=0,iReturn;

for (;;) {
repeat=5;

sprintf(path,"");
iReturn = __system_property_get("net.dns1",path);
if ( ( ! iReturn ) || ( strchr(path,'.') == NULL ) ) {
  fp = popen("getprop net.dns1 2>/dev/null", "r"); 
  fgets(path, sizeof(path)-1, fp);
  pclose(fp);
  repeat=30;
}

//path[7]='\0';

if ( strncmp(path,"0.0.0.0",7) != 0 ) { 
  sprintf(path2,"");
iReturn = __system_property_get("net.dns2",path2);
  if ( ( ! iReturn ) || ( strchr(path2,'.') == NULL ) ) {
    fp = popen("getprop net.dns2 2>/dev/null", "r");
    fgets(path2, sizeof(path2)-1, fp);
    pclose(fp);
  }
//  path2[7]='\0';

  if ( ( strchr(path2,'.') != NULL ) && ( strchr(path,'.') != NULL ) ) { 
//    sprintf(command,"%s/busybox echo -e \"nameserver %s\\nnameserver %s\" > /dev/resolv-local.conf; %s/busybox sort -u /dev/resolv-local.conf;%s/busybox echo -e \"nameserver %s\\nnameserver %s\" >> /dev/resolv-local.conf.tmp",buf,path,path2,buf);
    sprintf(command,"%s/busybox echo -e \"nameserver %s\\nnameserver %s\" > /dev/resolv-local.conf",buf,path,path2);
    system(command);
  }
  sprintf(command,"%s/busybox timeout -t 60 -s KILL %s/zzCHECK_NET_DNS SET %s",buf,buf,path);
  system(command);
}

sleep(repeat); 
count+=repeat;

if ( startup != 1 ) {

iReturn = __system_property_get("sys.boot_completed",path);

if ( ! iReturn ) {
  fp = popen("getprop sys.boot_completed 2>/dev/null", "r");
  fgets(path, sizeof(path)-1, fp);
  pclose(fp);
}

if ( ( strncmp(path,"1",1) == 0 ) && ( count > 600 ) ) {
  startup = 1;
  char append_command[256]="";
  sprintf(command,"%s/busybox timeout -t 120 -s KILL %s/zzCrossBreeder FORCE & %s",buf,buf,append_command);
  system(command);
}

}
/*
int WWT=16;

if(sscanf(argv[1], "%d", &WWT) == EOF ) WWT=16;

int IRQ_FLUSH_FREQUENCY=60;

if(sscanf(argv[2], "%d", &IRQ_FLUSH_FREQUENCY) == EOF ) IRQ_FLUSH_FREQUENCY=60;

if ( ( count % (IRQ_FLUSH_FREQUENCY * 60) ) == 0 ) {
  fptr_WRITE_WAKEUP_THRESHOLD=fopen("/proc/sys/kernel/random/write_wakeup_threshold","w");
  if ( fptr_WRITE_WAKEUP_THRESHOLD != NULL ) { 
    fprintf(fptr_WRITE_WAKEUP_THRESHOLD,WWT);
    fclose(fptr_WRITE_WAKEUP_THRESHOLD); 
  }
}
*/
/*
char one;

if ( ( count % 300 ) == 0 ) { 
  fptr_WRITE_WAKEUP_THRESHOLD=fopen("/sys/power/wait_for_fb_wake","r");
  if ( fptr_WRITE_WAKEUP_THRESHOLD != NULL ) {
    fscanf(fptr_WRITE_WAKEUP_THRESHOLD,"%c",&one);
    fclose(fptr_WRITE_WAKEUP_THRESHOLD);
  }
}
*/
if ( count > 86400 ) { startup = 0; count = 600 + repeat; }

}

return 0;
}
