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

void satisfy_open_FDs_leak_detection() {
        /* satisfy Valgrind FDs leak detection for the parent process */
        if (fflush(stdout) != 0) err(EXIT_FAILURE, "fflush(stdout)");
        if (fflush(stderr) != 0) err(EXIT_FAILURE, "fflush(stderr)");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
}
char **argv;

void example_reading(int use_compat) {
	FILE *fp;
	char buf[256];
	int status;
	struct popen_noshell_pass_to_pclose pclose_arg;

//	char *cmd = "ls -la /proc/self/fd"; /* used only by popen_noshell_compat(), we discourage this type of providing a command */
//	char *cmd = "getprop net.dns1"; /* used only by popen_noshell_compat(), we discourage this type of providing a command */
	char *cmd = "/system/etc/CrossBreeder/busybox timeout -t 300 -s KILL /system/etc/CrossBreeder/zzCHECK_NET_DNS"; /* used only by popen_noshell_compat(), we discourage this type of providing a command */

	/* the command arguments used by popen_noshell() */
	char *exec_file = "/system/etc/CrossBreeder/busybox";
	char *arg1 = "timeout";
	char *arg2 = "-t";
	char *arg3 = "300";
	char *arg4 = "-s";
	char *arg5 = "KILL";
	char *arg6 = "/system/etc/CrossBreeder/zzCHECK_NET_DNS";
	char *arg7 = (char *) NULL; /* last element */
        char *argv_local[] = { exec_file, arg1, arg2, arg3, arg4, arg5, arg6, arg7 }; /* NOTE! The first argv[] must be the executed *exec_file itself */
	
	if (use_compat) {
		fp = popen_noshell_compat(cmd, "r", &pclose_arg);
		if (!fp) {
			err(EXIT_FAILURE, "popen_noshell_compat()");
		}
	} else {
		fp = popen_noshell(exec_file, (const char * const *)argv_local, "r", &pclose_arg, 0);
		if (!fp) {
			err(EXIT_FAILURE, "popen_noshell()");
		}
	}

	while (fgets(buf, sizeof(buf)-1, fp)) {
		printf("%s", buf);
	}

	status = pclose_noshell(&pclose_arg);
	if (status == -1) {
		err(EXIT_FAILURE, "pclose_noshell()");
	} /*else {
		printf("The status of the child is %d. Note that this is not only the exit code. See man waitpid().\n", status);
	}*/
}

static void sig_handler(int sig) 
{
 printf("Caught signal\n");
 example_reading(0);
// satisfy_open_FDs_leak_detection();
}

static void sig_handler(int sig);

int main(int argc, char **argv_main) {

//sigset_t mask;
//sigfillset(&mask);
//sigprocmask(SIG_SETMASK, &mask, NULL);

  struct sigaction sigact;

  sigact.sa_handler = sig_handler;                                                                                                
  sigact.sa_flags = 0;

  sigemptyset(&sigact.sa_mask);
  sigaction(SIGUSR1, &sigact, NULL);
  sigaction(SIGUSR2, &sigact, NULL);

  sigact.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &sigact, NULL);
  sigaction(SIGTERM, &sigact, NULL);
  sigaction(SIGINT, &sigact, NULL);


struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
fl.l_pid = getpid();

int pid_file = open("/dev/CHECK_NET_DNS.RUN", O_CREAT | O_RDWR, 0644);

int rc = fcntl(pid_file, F_SETLK, &fl);
if ( rc == -1 ) {
      printf("Couldn't lock PID file \"/dev/CHECK_NET_DNS.RUN\" for writing.\n");
    return 1;
}
        argv=argv_main;
        
while(1) {
	pause();
}

	return 0;
}
