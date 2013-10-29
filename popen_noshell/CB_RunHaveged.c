/*
 * popen_noshell: A faster implementation of popen() and system() for Linux.
 * Copyright (c) 2009 Ivan Zahariev (famzah)
 * Version: 1.0
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 */

#include "popen_noshell.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <fcntl.h>
#include <errno.h>

/***********************************
 * popen_noshell use-case examples *
 ***********************************
 *
 * Compile and test via:
 * 	gcc -Wall popen_noshell.c popen_noshell_examples.c -o popen_noshell_examples && ./popen_noshell_examples
 *
 * If you want to profile using Valgrind, then compile and run via:
 *	gcc -Wall -g -DPOPEN_NOSHELL_VALGRIND_DEBUG popen_noshell.c popen_noshell_examples.c -o popen_noshell_examples && \
 *        valgrind -q --tool=memcheck --leak-check=yes --show-reachable=yes --track-fds=yes ./popen_noshell_examples
 */

void satisfy_open_FDs_leak_detection_and_exit() {
	/* satisfy Valgrind FDs leak detection for the parent process */
	if (fflush(stdout) != 0) err(EXIT_FAILURE, "fflush(stdout)");
	if (fflush(stderr) != 0) err(EXIT_FAILURE, "fflush(stderr)");
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	exit(0);
}

char **argv;

int main(int argc, char **argv_main) {
sigset_t mask;
sigfillset(&mask);
sigprocmask(SIG_SETMASK, &mask, NULL);
int i=0,secs=0,startup=0;
char buf[1024]="/system/etc/CrossBreeder",command[1024];
char path[1024]="",path2[1024],path3[1024];


struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
fl.l_pid = getpid();

int pid_file = open("/dev/CB_RunHaveged.RUN", O_CREAT | O_RDWR, 0644);

int rc = fcntl(pid_file, F_SETLK, &fl);
if ( rc == -1 ) {
      printf("Couldn't lock PID file \"/dev/CB_RunHaveged.RUN\" for writing.\n");
    return 1;
}
        argv=argv_main;
		
	int try_compat=0;
	
	FILE *fp;
	int fd,err;
	int status,iReturn=-1;
	char one;

	struct popen_noshell_pass_to_pclose pclose_arg;

	/* the command arguments used by popen_noshell() */
	char *exec_file = "/system/xbin/haveged";
	char *arg1 = "-F";
	char *arg2 = "-s";
	char *arg3 = argv[1];
	char *arg4 = "-w";
	char *arg5 = argv[2];
	char *arg6 = (char *) NULL; /* last element */
    char *argv_local[] = { exec_file, arg1, arg2, arg3, arg4, arg5, arg6 }; /* NOTE! The first argv[] must be the executed *exec_file itself */
	time_t start;
	
	fp = popen_noshell(exec_file, (const char * const *)argv_local, "r", &pclose_arg, 0);
    
	if (!fp)
	  printf("CB_RunHaveged: Haveged startup failed.\n");
	else 
	  printf("CB_RunHaveged: Haveged startup succeeded.\n");
	  
//	if (!fp) {
//		err(EXIT_FAILURE, "popen_noshell()");
//	}

//	while (fgets(buf, sizeof(buf)-1, fp)) {
//		printf("%s", buf);
//	}

	time_t p_start = time(0);
	
loop:

       start = time(0);
       fd = open("/sys/power/wait_for_fb_wake", O_RDONLY|O_NONBLOCK, 0);
       do { err = read(fd, &one, 1); }
                while (err < 0 && errno == EINTR); 
       close(fd);
       time_t stop = time(0);
       
       if ( ( stop - start ) > 5 ) {
                  printf("Wakeup to reset governors\n");
            system("( sleep 5; /system/etc/CrossBreeder/CB_Governor_Tweaks.sh 2>/dev/null ) &");
       }
        
	if ( secs > 310 ) {
	secs = 0;

	iReturn = __system_property_get("net.dns1",path);

	if ( ( ! iReturn ) || ( strchr(path,'.') == NULL ) ) {
	  fp = popen("getprop net.dns1 2>/dev/null", "r");
	  fgets(path, sizeof(path)-1, fp);
	  pclose(fp);
	} 

	if ( strncmp(path,"0.0.0.0",7) != 0 ) {
	  printf("CB_RunHaveged: DNS change with net.dns1 = %s\n",path);
	  sprintf(command,"touch /dev/resolv-local.conf.TOUCH; %s/busybox killall -SIGUSR1 CB_Check_Net_DNS &",buf);
	  system(command);
	}
    
	if ( startup < 2 ) { 

		iReturn = __system_property_get("sys.boot_completed",path3);

		if ( ! iReturn ) {
		  fp = popen("getprop sys.boot_completed 2>/dev/null", "r");
		  fgets(path3, sizeof(path3)-1, fp);
		  pclose(fp);
		}

		if ( startup < 1 ) {
		if ( ( strncmp(path,"1",1) == 0 ) && ( ( stop - p_start ) > 300 ) ) {
		  startup = 1;
		  sprintf(command,"%s/busybox timeout -t 120 -s KILL %s/zzCrossBreeder FORCE &",buf,buf);
		  system(command);
		}
		}

		if ( ( stop - p_start ) > 600 ) {
		  startup = 2;
		  sprintf(command,"%s/busybox timeout -t 120 -s KILL %s/zzCrossBreeder FORCE &",buf,buf);
		  system(command);
		}

	}

	}
	
    fd = open("/sys/power/wait_for_fb_sleep", O_RDONLY|O_NONBLOCK, 0);
    do { err = read(fd, &one, 1); }
        while (err < 0 && errno == EINTR); 
    close(fd);
       
    sleep(30);   
	secs += 30;
goto loop;

	status = pclose_noshell(&pclose_arg);
	//if (status == -1) {
		//err(EXIT_FAILURE, "pclose_noshell()");
	//} /*else {
		printf("The status of the child is %d. Note that this is not only the exit code. See man waitpid().\n", status);
	//}*/

	satisfy_open_FDs_leak_detection_and_exit();

	return 0;
}
