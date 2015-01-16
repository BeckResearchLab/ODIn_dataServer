/*******************************************************************************
** dataServer.c
** UDP packet logger as data logger for OD device
** David Andrew Crawford Beck
** dacb@uw.edu
** Original:
**	Mon Apr 1 10:30:16 PDT 2013
** Modified:
*******************************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "packet.h"

int main(int argc, char *argv[]) {
	char *server_ip;
	unsigned int port;
	packet_t p;

	// UDP socket client variables
	int sock_fd;
	struct sockaddr_in servaddr;

	// argument checking & handling
	if (argc != 3) {
		fprintf(stderr, "usage: %s <server IP> <UDP port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	server_ip = argv[1];
	port = atoi(argv[2]);

	// initial setup of UDP client
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(server_ip);
	servaddr.sin_port = htons(port);

	bzero(&p, sizeof(packet_t));
	while (1) {
		int i;
		p.sequenceID++;
		for (i = 0; i < ADCs; ++i)
			p.adc[i] = p.sequenceID % (i + 1);
		sendto(sock_fd, &p, sizeof(packet_t), 0, (struct sockaddr *)&servaddr,sizeof(servaddr));
		sleep(1);
	}

	exit(EXIT_SUCCESS);
}
