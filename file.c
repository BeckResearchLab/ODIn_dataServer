#include "dataServer.h"

extern struct dataServer dataServer;

void file_open_log(char *filename) {
	time_t now;
	struct tm *nowlocal;
	char time_text[TIME_SIZE];
	int i;

	// start at the end and look back for a /
	for (i = strlen(filename) - 1; i >= 0; --i) {
		if (filename[i] == '/')
			break;
	}
	if (i != -1) {
		char *tmp = strdup(&filename[i+1]);
		filename = tmp;
	}

	printf("setting log output file to '%s'\n", filename);

	pthread_mutex_lock(&dataServer.file_mutex);

	if (dataServer.fp)
		fclose(dataServer.fp);
	if (dataServer.log_file)
		free(dataServer.log_file);

	dataServer.log_file = strdup(filename);
	if ((dataServer.fp = fopen(dataServer.log_file, "a")) == NULL) {
		fatal_error("%s: unable to open output file '%s'\n", "file_open_log", dataServer.log_file);
	}

	// initial setup of logging
	now = time(NULL);
	nowlocal = localtime(&now);
	strftime(time_text, TIME_SIZE, "%m/%d/%y %H:%M:%S %z", nowlocal);
	fprintf(dataServer.fp, "# %s - logging started at %s on UDP port %d\n", dataServer.log_file, time_text, dataServer.udp_port);
	fprintf(dataServer.fp, "# time\terror_flag\tsequenceID");
	for (i = 0; i < ADCs; ++i) {
		fprintf(dataServer.fp, "\tADC_%d", i);
	}
	fprintf(dataServer.fp, "\n");
	fflush(dataServer.fp);

	pthread_mutex_unlock(&dataServer.file_mutex);
}

char *file_read_to_char_buf(char *filename) {
	FILE *fp;
        char *buf = NULL;
        int buf_len = 0, c, l;

	if ((fp = fopen(filename, "r")) == NULL) {
		fatal_error("file_read_to_char_buf: unable to open file '%s'\n", filename);
	}

        buf = (char *)malloc(sizeof(char));
        buf[0] = '\0';
        buf_len = 1;

        while ((c = getc(fp)) != EOF) {
                l = buf_len++;
                buf = (char *)realloc(buf, buf_len * sizeof(char));
                buf[l - 1] = c;
                buf[l] = '\0';
        }

	fclose(fp);

        return buf;
}
