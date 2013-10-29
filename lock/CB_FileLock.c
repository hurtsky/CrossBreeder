/*
** lockdemo.c -- shows off your system's file locking.  Rated R.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	                /* l_type   l_whence  l_start  l_len  l_pid   */
	struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
	int fd;

	fl.l_pid = getpid();

	/*if (argc > 1)*/ 
	/*fl.l_type = F_RDLCK;*/

	if ((fd = open(argv[1], O_WRONLY|O_CREAT|O_EXCL)) == -1) {
		perror("open");
		exit(99);
	}

/*	printf("Press <RETURN> to try to get lock: ");
	getchar();
	printf("Trying to get lock...");
*/
/*	if (fcntl(fd, F_SETLKW, &fl) == -1) {*/
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		exit(99);
	}

	printf("got lock\n");
/*	printf("Press <RETURN> to release lock: ");
	getchar();*/

	fl.l_type = F_UNLCK;  /* set to unlock same region */

	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		exit(99);
	}

	printf("Unlocked.\n");

	close(fd);

	return 0;
}

