#ifndef _DATASERVER_H
#define _DATASERVER_H

#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <limits.h>
#include <float.h>

#include "sndup.h"
#include "error.h"
#include "file.h"
#include "socket.h"
#include "net.h"
#include "packet.h"
#include "httpd.h"

enum STATE { BOOT, IDLE, RUN, HALT };

/* mcp root data structure */
struct dataServer {
	pthread_mutex_t	file_mutex;
	char		*log_file;
	FILE		*fp;
	unsigned int	total_packets;
	unsigned int	udp_port;
	unsigned int	httpd_port;
	int		sock_fd;
	httpd_t		httpd;	/* mini-web server for interacting w/ dataServer */
};

#define TIME_SIZE 128

#endif /* _DATASERVER_H */
