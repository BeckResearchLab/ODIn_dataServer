/*******************************************************************************
** dataServer.c
** UDP packet logger as data logger for OD device
** David Andrew Crawford Beck
** dacb@uw.edu
** Original:
**	Mon Apr 1 10:30:16 PDT 2013
** Modified:
*******************************************************************************/

#include "dataServer.h"

struct dataServer dataServer;

int main(int argc, char *argv[]) {
	// logging variables
	time_t now;
	struct tm* nowlocal;
	unsigned int i;
	char time_text[TIME_SIZE];
	packet_t p;

	// UDP socket server variables
	int sock_fd, rlen;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len;

	memset(&dataServer, 0, sizeof(struct dataServer));
	pthread_mutex_init(&dataServer.file_mutex, NULL);

	// argument checking & handling
	if (argc != 4) {
		fatal_error("usage: %s <UDP port #> <output filename> <HTTPd port #>\n", argv[0]);
	}
	dataServer.udp_port = atoi(argv[1]);
	file_open_log(argv[2]);
	dataServer.httpd.config.port = atoi(argv[3]);

	// start the web server
	httpd_start();

	// initial setup of UDP server
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(dataServer.udp_port);
	bind(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	len = sizeof(cliaddr);

	printf("UDP server listening on port %d\n", dataServer.udp_port);

	// main capture loop
	while (1) {
		uint8_t error = 0;
		rlen = recvfrom(sock_fd, &p, sizeof(packet_t), 0, (struct sockaddr *)&cliaddr, &len);
		now = time(NULL);
		nowlocal = localtime(&now);
		++dataServer.total_packets;
		if (rlen != sizeof(packet_t)) {
			warning("%s: invalid packet received of size %d\n", argv[0], rlen);
			memset(&p, 0, sizeof(packet_t));
			error = 1;
		}
		// time, error flag, sequence ID, values
		strftime(time_text, TIME_SIZE, "%m/%d/%y %H:%M:%S %z", nowlocal);
		pthread_mutex_lock(&dataServer.file_mutex);
		fprintf(dataServer.fp, "%s\t%d\t%u", time_text, error, p.sequenceID);
		for (i = 0; i < ADCs; ++i)
			fprintf(dataServer.fp, "\t%d", p.adc[i]);
		fprintf(dataServer.fp, "\n");
		if (dataServer.total_packets % 1000 == 0) printf("%s: at %s - %d total packets received\n", argv[0], time_text, dataServer.total_packets);
		fflush(dataServer.fp);
		pthread_mutex_unlock(&dataServer.file_mutex);
	}

	exit(EXIT_SUCCESS);
}
