/*
* pixelserv.c a small mod to public domain server.c -- a stream socket server demo
* from http://beej.us/guide/bgnet/
* single pixel http string from http://proxytunnel.sourceforge.net/pixelserv.php
*/

#define VERSION "V30"

#define BACKLOG 30 // how many pending connections queue will hold
#define CHAR_BUF_SIZE 1023 //surprising how big requests can be with cookies etc

#define DEFAULT_IP "127.0.0.1" // default IP address = all
#define DEFAULT_PORT "80"  // the default port users will be connecting to

#ifdef IF_MODE
# define DEFAULT_IF "" // default interface was br0, blank all
#endif

#ifdef DROP_ROOT
# define DEFAULT_USER "nobody" // nobody used by dnsmasq
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> /* for TCP_NODELAY */
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <net/if.h> /* for IFNAMSIZ */
#include <pwd.h> /* for getpwnam */

#ifdef READ_FILE
#include <sys/stat.h>
#endif

#ifdef TEST
# define TEXT_REPLY 1
# define VERBOSE 1
# define TESTPRINT printf
#else 
# define TESTPRINT(x,y...)
#endif

#ifdef VERBOSE
# define MYLOG syslog
#else // rely on optimiser to remove redundant code
# define MYLOG(x,y...)
#endif

#ifdef TINY
/* redefine log functions to NULL */
#	define openlog(x,y...)
#	define syslog(x,y...)
#endif

#define OK (0)
#define ERROR (-1)
#define EXIT_TXT 10
#define EXIT_BAD 20

#ifdef DO_COUNT
volatile sig_atomic_t count = 0;
volatile sig_atomic_t gif = 0;
volatile sig_atomic_t err = 0;
# ifdef TEXT_REPLY
volatile sig_atomic_t txt = 0;
volatile sig_atomic_t bad = 0;
# endif /* TEXT_REPLY */
#endif /* DO_COUNT */

/** common signal handler */
void signal_handler (int sig)
{
	int status;
	switch (sig)
	{
	/* handler to ensure no zombie sub processes left */
	case SIGCHLD :
		while ( waitpid (-1, &status, WNOHANG) > 0 ) {
#ifdef DO_COUNT
			if ( WIFEXITED (status) ) {
				switch ( WEXITSTATUS (status) )
				{
					case EXIT_FAILURE :
						err++;
						break;

					case EXIT_SUCCESS :
						gif++;
						break;
#ifdef TEXT_REPLY
					case EXIT_BAD :
						bad++;
						break;

					case EXIT_TXT :
						txt++;
						break;
#endif /* TEXT_REPLY */
				}
			}
#endif /* DO_COUNT */
		};
		return;

#ifndef TINY
	/* Handler for the SIGTERM signal (kill) */
	case SIGTERM :
		signal (sig, SIG_IGN);	/* Ignore this signal while we are quitting */
#ifdef DO_COUNT
	case SIGUSR1 :
#ifdef TEXT_REPLY
		syslog (LOG_INFO, "%d requests, %d errors, %d bad, %d gif, %d txt replies", count, err, bad, gif, txt);
#else
		syslog (LOG_INFO, "%d requests, %d errors, %d gif", count, err, gif);
#endif
		if (sig == SIGUSR1) {
			return;
		}
#endif /* DO_COUNT */
		syslog (LOG_NOTICE, "exit on SIGTERM");
		exit (EXIT_SUCCESS);
#endif /* TINY */
	}
}

#ifdef TEST
/** get sockaddr, IPv4 or IPv6: */
void *get_in_addr (struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(( (struct sockaddr_in*) sa )->sin_addr);
	}

	return &(( (struct sockaddr_in6*) sa )->sin6_addr);
}
#endif

/** program start */
int main (int argc, char *argv[])
{
	int sockfd, new_fd;	/* listen on sock_fd, new connection on new_fd */
	struct sockaddr_storage their_addr;	/* connector's address information */
	socklen_t sin_size;
	int yes = 1;
	int n = -1;  // to turn off linger2
#ifdef TEST
	char s[INET6_ADDRSTRLEN];
#endif
	int rv;
	char ip_addr[INET_ADDRSTRLEN] = DEFAULT_IP;
	int use_ip = 0;
	char buf[CHAR_BUF_SIZE + 1];

#ifdef PORT_MODE
	char port[6] = DEFAULT_PORT;/* not sure how long this can be, use number if name too long */
#else
# define port DEFAULT_PORT
#endif

	int i;
#ifdef IF_MODE
	char ifname[IFNAMSIZ] = DEFAULT_IF;
#endif

#ifdef DROP_ROOT
	char user[8] = DEFAULT_USER; /* used to be long enough */
	struct passwd *pw;
#endif

#ifdef READ_FILE
	char *fname = NULL;
	int fsize;
  #ifdef READ_GIF
	int do_gif = 0; 
  #endif
	int hsize = 0;   	
    struct stat file_stat;
    FILE *fp;
#endif

	static unsigned char httpnullpixel[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-type: image/gif\r\n"
	"Content-length: 43\r\n"
	"Connection: close\r\n"
	"\r\n"
	"GIF89a\1\0\1\0\200\0\0\377\377\377\0\0\0\41\371\4\1\0\0\0\0\54\0\0\0\0\1\0\1\0\0\2\2\104\1\0;" ;

#ifdef TEXT_REPLY
	static unsigned char httpnulltext[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-type: text/javascript\r\n"
	"Content-length: 0\r\n"
	"Connection: close\r\n"
	"\r\n";

	static unsigned char http501[] =
	"HTTP/1.1 501 Method Not Implemented\r\n"
	"Connection: close\r\n"
	"\r\n";
#endif

        static unsigned char httptext[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Last-Modified: Sat, 08 Jan 1492 01:12:12 GMT\r\n"
        "Content-Length: 42\r\n\r\n"
//        "<html>[AD]</html>\r\n";
        "<HTML><TITLE></TITLE><BODY></BODY></HTML>\r\n";

	unsigned char *httpresponse = httpnullpixel;
	int rsize = (sizeof httpnullpixel) - 1;

	struct addrinfo hints, *servinfo;
	int error = 0;
	
	fd_set set;
	struct timeval timeout;
	int select_rv;
	int status = EXIT_FAILURE; /* default return from child */

	/* command line arguments processing */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-') {
#ifdef INETD_MODE
			if (argv[i][1] == 'i') {
				/* first check if "-i" option is used */
				fwrite (httpnullpixel, 1, (sizeof httpnullpixel) - 1, stdout);  //not updated for TEXT_REPLY
				/* flush read buffer ? */
				exit (EXIT_SUCCESS);
			}
#endif
			if ( (i + 1) < argc ) {
				switch (argv[i][1])
				{
#ifdef IF_MODE
				case 'n' :
					strncpy (ifname, argv[++i], IFNAMSIZ);
					ifname[IFNAMSIZ - 1] = '\0';
					break;
#endif
#ifdef PORT_MODE
				case 'p' :
					strncpy (port, argv[++i], sizeof port);
					port[(sizeof port) - 1] = '\0';
					break;
#endif
#ifdef DROP_ROOT
				case 'u' :
					strncpy (user, argv[++i], sizeof user);
					user[(sizeof user) - 1] = '\0';
					break;
#endif
#ifdef READ_FILE
  #ifdef READ_GIF
				case 'g' :
					do_gif = 1; /* and fall through */
  #endif					
				case 'f' :
					fname = argv[++i];
					break;				
#endif
				default :
					error = 1;
				}
			} else {
				error = 1;
			}
		} else if (use_ip == 0) {
			/* assume its a listening IP address */
			strncpy (ip_addr, argv[i], INET_ADDRSTRLEN);
			ip_addr[INET_ADDRSTRLEN - 1] = '\0';
			use_ip = 1;
		} else {
			error = 1; // fix bug with 2 IP like args
		}
	}

	if (error) {
#ifndef TINY
		printf ("Usage:%s"
#ifdef INETD_MODE
		" [-i inetd mode, no other params]"
#endif
		" [IP No/hostname (all)]"
#ifdef PORT_MODE
		" [-p port (80)]"
#endif
#ifdef IF_MODE
		" [-n i/f (all)]"
#endif
#ifdef DROP_ROOT
		" [-u user (\"nobody\")]"
#endif
#ifdef READ_FILE
		" [-f response.bin]"
  #ifdef READ_GIF
		" [-g name.gif]"
  #endif		
#endif
		"\n", argv[0]);
#endif /* TINY */
		exit (EXIT_FAILURE);
	}

	openlog ("pixelserv", LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
	syslog ( LOG_INFO, "%s %s compiled: %s from %s", argv[0], VERSION, __DATE__ " " __TIME__ , __FILE__ );

#ifdef READ_FILE
    if (fname)
    {
		if ( stat (fname, &file_stat) < 0 ) {
			syslog (LOG_ERR, "stat: '%s': %m", fname);
			exit (EXIT_FAILURE);		
		}
		
		fsize = (int) file_stat.st_size;
		TESTPRINT ("fsize:%d\n", fsize);		

		if (fsize < 43)
		{
			syslog (LOG_ERR, "%s: size only %d", fname, fsize);
			exit (EXIT_FAILURE);		
		}
		
		if ( (fp = fopen (fname, "rb")) == NULL ) {
			syslog (LOG_ERR, "fopen: '%s': %m", fname);
			exit (EXIT_FAILURE);		
		}

  #ifdef READ_GIF			
		if (do_gif) {
			snprintf (buf, CHAR_BUF_SIZE,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: image/gif\r\n"
				"Content-length: %d\r\n"
				"Connection: close\r\n"
				"\r\n", fsize);
			
			hsize = strlen (buf);
			TESTPRINT ("hsize:%d\n", hsize);		
		}
  #endif		
		
		rsize = hsize + fsize;
		TESTPRINT ("rsize:%d\n", rsize);		
		if ( (httpresponse = malloc (rsize)) == NULL )
		{
			syslog (LOG_ERR, "malloc: %m");
			exit (EXIT_FAILURE);		
		}

  #ifdef READ_GIF	
		if (do_gif) {		
			strcpy ((char *) httpresponse, buf);
		}
  #endif
		
		if ( fread ( &httpresponse[hsize], sizeof(char), fsize, fp) < fsize )
		{
			syslog (LOG_ERR, "fread: '%s': %m", fname);
			exit (EXIT_FAILURE);	
		}
		
		fclose (fp);
		} 
  #ifdef SAVE_RESP		
	fp = fopen ("test.tmp", "wb");
	fwrite (httpresponse, sizeof(char), rsize, fp );
	fclose (fp);
  #endif				
#endif   	
   	
#ifndef TEST
	if ( daemon (0, 0) != OK ) {
		syslog (LOG_ERR, "failed to daemonize, exit: %m");
		exit (EXIT_FAILURE);
	}
#endif /* TEST */

	memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; /* AF_UNSPEC - AF_INET restricts to IPV4 */
	hints.ai_socktype = SOCK_STREAM;
	if (use_ip == 0) {
		hints.ai_flags = AI_PASSIVE; /* use my IP */
	}
	
	rv = getaddrinfo (use_ip ? ip_addr : NULL, port, &hints, &servinfo);
	if (rv != OK) {
		syslog ( LOG_ERR, "getaddrinfo: %s", gai_strerror (rv) );
		exit (EXIT_FAILURE);
	}

	if ( (( sockfd = socket (servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) ) < 1)
		|| ( setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) != OK )
#ifdef IF_MODE
		|| ( setsockopt (sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifname, IFNAMSIZ)	!= OK )	/* only use selected i/f */
#endif
		|| ( setsockopt (sockfd, SOL_TCP, TCP_NODELAY, &yes, sizeof (int)) != OK )		/* send short packets straight away */
		|| ( setsockopt (sockfd, SOL_TCP, TCP_LINGER2, (void *) &n, sizeof (n)) != OK )	/* try to prevent hanging processes in FIN_WAIT2 */
		|| ( bind (sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != OK )
		|| ( listen (sockfd, BACKLOG) != OK ) ) {
		syslog (LOG_ERR, "Abort: %m");
		exit (EXIT_FAILURE);
	}

	freeaddrinfo (servinfo); /* all done with this structure */

	{
		struct sigaction sa;
		sa.sa_handler = signal_handler;
		sigemptyset (&sa.sa_mask);

#ifndef TINY
		/* set signal handler for termination */
		if ( sigaction (SIGTERM, &sa, NULL) != OK ) {
			syslog (LOG_ERR, "SIGTERM %m");
			exit (EXIT_FAILURE);
		}
#endif
		/* reap all dead processes */
		sa.sa_flags = SA_RESTART;
		if ( sigaction (SIGCHLD, &sa, NULL) != OK ) {
			syslog (LOG_ERR, "SIGCHLD %m");
			exit (EXIT_FAILURE);
		}
#ifdef DO_COUNT
		/* set signal handler for info */
		if ( sigaction (SIGUSR1, &sa, NULL) != OK ) {
			syslog (LOG_ERR, "SIGUSR1 %m");
			exit (EXIT_FAILURE);
		}
#endif
	}

#ifdef DROP_ROOT
	if ( (pw = getpwnam (user)) == NULL ) {
		syslog (LOG_ERR, "Unknown user \"%s\"", user);
		exit (EXIT_FAILURE);
	}

	if ( setuid (pw->pw_uid) ) {
		syslog ( LOG_ERR, "setuid %d: %s\n", pw->pw_uid, strerror (errno) );
		exit (EXIT_FAILURE);
	}
#endif /* DROP_ROOT */

#ifdef IF_MODE
	syslog (LOG_NOTICE, "Listening on %s %s:%s", ifname, ip_addr, port);
#else
	syslog (LOG_NOTICE, "Listening on %s:%s", ip_addr, port);
#endif

	while(1) {  /* main accept() loop */
		sin_size = sizeof their_addr;
		new_fd = accept ( sockfd, (struct sockaddr *) &their_addr, &sin_size );
		if (new_fd < 1) {
			MYLOG (LOG_WARNING, "accept: %m");
			continue;
		}

#ifdef DO_COUNT
		count++;
#endif

		if ( fork () == 0 ) {
			/* this is the child process */
			close (sockfd); /* child doesn't need the listener */
#ifndef TINY
			signal(SIGTERM, SIG_DFL);
#endif
			signal(SIGCHLD, SIG_DFL);
#ifdef DO_COUNT
			signal(SIGUSR1, SIG_IGN);
#endif

#ifdef TEST
			inet_ntop (their_addr.ss_family, get_in_addr( (struct sockaddr *) &their_addr ), s, sizeof s);
			printf ("server: got connection from %s\n", s);
#endif

#ifdef TEXT_REPLY
			/* read a line from the request */
			FD_ZERO (&set);
			FD_SET(new_fd, &set);
			/* Initialize the timeout data structure */
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;

			/* select returns 0 if timeout, 1 if input available, -1 if error */
			select_rv = select (new_fd + 1, &set, NULL, NULL, &timeout);
			if (select_rv < 0) {
				MYLOG (LOG_ERR, "select: %m");			
			} else if (select_rv == 0) {
				MYLOG (LOG_ERR, "timeout on select");
			} else {
				rv = recv (new_fd, buf, CHAR_BUF_SIZE, 0);
				if (rv < 0) {
					MYLOG (LOG_ERR, "recv: %m");
				} else if (rv == 0) {
					MYLOG (LOG_ERR, "recv: No data");
				} else {
					buf[rv] = '\0';
					TESTPRINT ("\nreceived %d bytes\n'%s'\n", rv, buf);
					char *method = strtok (buf, " ");
					if (method == NULL) {
						MYLOG (LOG_ERR, "null method");						
					} else {
						TESTPRINT ("method: '%s'\n", method);
						if ( strcasecmp (method, "GET") != 0 ) {
							MYLOG (LOG_ERR, "unknown method: %s", method);
							status = EXIT_BAD;
							TESTPRINT ("Sending 501 response\n");
							httpresponse = http501;
							rsize = (sizeof http501) - 1;							
						} else {													
							status = EXIT_SUCCESS;	// send a gif by default from here
							/* trim up to non path chars */
							char *path = strtok (NULL, " ?#;="); // "?;#:*<>[]='\"\\,|!~()"
							if (path == NULL) {
							    MYLOG (LOG_ERR, "null path");
							} else {
								TESTPRINT ("path: '%s'\n",path);
								char *file = strrchr (path, '/');
								if (file == NULL) {
									MYLOG (LOG_ERR, "invalid file path %s", path);								
								} else {
									TESTPRINT ("file: '%s'\n", file);
									char *ext = strrchr (file, '.');
									if (ext == NULL) {
										MYLOG (LOG_ERR, "No file extension %s", file);
											TESTPRINT ("Sending 501 response\n");
											httpresponse = http501;
											rsize = (sizeof http501) - 1;												
									} else {							
										TESTPRINT ("ext: '%s'\n", ext);
										if ( strncasecmp (ext, ".js", 3) == 0 ) {	/* .jsx ?*/
											status = EXIT_TXT;
											TESTPRINT ("Sending JS response\n");
											httpresponse = httpnulltext;
											rsize = (sizeof httpnulltext) - 1;												
										}
										/* add other response types here */
                                                                                if ( strncasecmp (ext, ".htm", 4) == 0 ) {
                                                                                        status = EXIT_TXT;
                                                                                        TESTPRINT ("Sending httptext response\n");
                                                                                        httpresponse = httptext;
                                                                                        rsize = (sizeof httptext) - 1;
                                                                                }
									}
								}
							}
						}
					}
				}
			}

			if (status != EXIT_FAILURE) {
				if (status == EXIT_SUCCESS)
#else
			{
				status = EXIT_SUCCESS;			
#endif /* TEXT_REPLY */
				{
					TESTPRINT ("Sending GIF response\n");				
				}
				
				rv = send (new_fd, httpresponse, rsize, 0);
					
				/* check for error message, but don't bother checking that all bytes sent */
				if (rv < 0) {
					MYLOG (LOG_WARNING, "send: %m");
					status = EXIT_FAILURE;
				}
			}

			/* clean way to flush read buffers and close connection */
			if (shutdown (new_fd, SHUT_WR) == OK) {
				do {
					/* Initialize the file descriptor set */
					FD_ZERO (&set);
					FD_SET(new_fd, &set);
					/* Initialize the timeout data structure */
					timeout.tv_sec = 2;
					timeout.tv_usec = 0;
					/* select returns 0 if timeout, 1 if input available, -1 if error */
					select_rv = select (new_fd + 1, &set, NULL, NULL, &timeout);
				} while ( (select_rv > 0) && ( recv (new_fd, buf, CHAR_BUF_SIZE, 0) > 0 ) );
			}

			shutdown (new_fd, SHUT_RD);
			close (new_fd);
			exit (status);
		}

		close (new_fd);	/* parent doesn't need this */
	}

	return (EXIT_SUCCESS);
}

/*
* V1 Proof of concept mstombs www.linkysinfo.org 06/09/09
* V2 usleep after send to delay socket close 08/09/09
* V3 TCP_NODELAY not usleep 09/09/09
* V4 daemonize with syslog 10/09/09
* V5 usleep back in 10/09/09
* V6 only use IPV4, add linger and shutdown to avoid need for sleep 11/09/09
* Consistent exit codes and version stamp
* V7 use shutdown/read/shutdown to cleanly flush and close connection
* V8 add inetd and listening IP option
* V9 minimalize
* V10 make inetd mode compiler option -DINETD_MODE
* V11 debug TCP_NODELAY back and MSG_DONTWAIT flag on send
* V12 Change read to recv with MSG_DONTWAIT and add MSG_NOSIGNAL on send
* V13 DONTWAIT's just trigger RST connection closing so remove
* V14 Back to V8 fork(), add header "connection: close"" and reformat pixel def
* V15 add command line options for variable port 2nd March 2010
* V16 add command line option for ifname, add SO_LINGER2 to not hang in FIN_WAIT2
* V17 only send null pixel if image requested, make most options compiler options to make small version
* V18 move image file test back into TEST
* V19 add TINY build which has no output.
* V20 Remove default interface "br0" assignment,
	amend http header to not encourage server like byte requests,
	use CHAR_BUF_SIZE rather than sizeof
	try again to turn off FIN_WAIT2
* V21 run as user nobody by default
* V22 Use apache style close using select to timeout connection
	and not leave dormant processes lying around if browser doeesn't close connection cleanly
	use SIGURS1 for report count, not system signal SIGHUP - thanks Rodney
* V23 be more selective about replies
* V24 common signal_handler and minor mods to minimize size
	Fix V23 bugs and use null string and javascript detection by ~nephelim~
  V25 test version for robust parsing of path
  V26 timeout on recv, block signals in child, enhance stats collection, fix bug in "-u user"
  V27 add error reply messages
  V28 move log, add option to read nullpixel from file.
  V29 add option to read gif from file
  V30 tidy up
*/
