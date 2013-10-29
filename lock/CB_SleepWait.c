#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h> /* idcrisis */

#ifndef NO_DAEMON
#include <unistd.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/random.h>
#endif

#include <errno.h>

int sleep_fd;
char sleep_read_buffer;

main() {

sleep_fd = open ( "/sys/power/wait_for_fb_sleep" , O_RDONLY);
        if (sleep_fd != -1 )
        {
           read (sleep_fd, &sleep_read_buffer, 0);
           close (sleep_fd);
        } 
}
