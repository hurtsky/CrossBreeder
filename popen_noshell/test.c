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

int main()
{
FILE *fd;
char one;
int err;

fd = open("/sys/power/wait_for_fb_wake", O_RDONLY | O_NONBLOCK, 0);
//do { 
err = read(fd, &one, 1); 
printf("%c\n",one);
//}                                                                                                 
//while (err < 0 && errno == EINTR); 

close(fd);
return err;
}
