#include "dataServer.h"

extern struct dataServer dataServer;

#define MAX_BUF 4096

void httpd_load_default_page_locations(void);

httpd_pages_t httpd_pages[] = { \
	{ "/config", 7, &httpd_page_config }, \
	{ "/halt", 5, &httpd_page_halt }, \
	{ "/", 1, &httpd_page_root }, \
	{ NULL, 0, NULL } \
};

int httpd_read_request(void) {
	int i, bytes_read;

	if (dataServer.httpd.client.request.len < MAX_REQUEST_SIZE) {
		dataServer.httpd.client.request.text = (char *)malloc(sizeof(char) * MAX_REQUEST_SIZE);
		memset(dataServer.httpd.client.request.text, 0, sizeof(char) * MAX_REQUEST_SIZE);
		dataServer.httpd.client.request.resource = (char *)malloc(sizeof(char) * MAX_REQUEST_SIZE);
		memset(dataServer.httpd.client.request.resource, 0, sizeof(char) * MAX_REQUEST_SIZE);
		dataServer.httpd.client.request.len = MAX_REQUEST_SIZE;
	}
	dataServer.httpd.client.request.size = 0;

	for (;;) {
		bytes_read = read(dataServer.httpd.client.sock, dataServer.httpd.client.request.text + dataServer.httpd.client.request.size, dataServer.httpd.client.request.len - dataServer.httpd.client.request.size - 1);

		if (bytes_read <= 0) {
			if (bytes_read == 0)
				warning("httpd_read_request: connection closed on read\n");
			else {
				if (errno == EAGAIN)
					warning("httpd_read_request: error EAGAIN\n");
				else
					warning("httpd_read_request: error on read\n");
			}
			return -1;
		}

		dataServer.httpd.client.request.size += bytes_read;
		dataServer.httpd.client.request.text[dataServer.httpd.client.request.size] = '\0';

		if (strstr (dataServer.httpd.client.request.text, "\r\r") || strstr (dataServer.httpd.client.request.text, "\n\n") || strstr (dataServer.httpd.client.request.text, "\r\n\r\n") || strstr (dataServer.httpd.client.request.text, "\n\r\n\r"))
			break;
	}

	/* check for GET method */
	if (strncmp(dataServer.httpd.client.request.text, HTTPD_GET, 4) != 0) {
		warning("httpd_read_request: client did not use GET method ('%s')", dataServer.httpd.client.request.text);
		net_send(dataServer.httpd.client.sock, HTTPD_METHOD_NOT_ALLOWED, strlen(HTTPD_METHOD_NOT_ALLOWED));
		return -1;
	}

	/* clip dataServer.httpd.client.request.textfer to end of first line */
	dataServer.httpd.client.request.text[strcspn(dataServer.httpd.client.request.text, "\n\r")] = '\0';

	/* parse name out of GET */	
	if (sscanf(dataServer.httpd.client.request.text, HTTPD_GET_LINE, dataServer.httpd.client.request.resource) != 1) {
		warning("httpd_read_request: unable to parse GET line ('%s')", dataServer.httpd.client.request.text);
		return -1;
	}

	/* run resource name through cgi library */
	for (i = 0; httpd_pages[i].name != NULL ; ++i) {
		if (strncmp(httpd_pages[i].name, dataServer.httpd.client.request.resource, httpd_pages[i].name_len) == 0) {
			dataServer.httpd.response.function = httpd_pages[i].function;
			if (dataServer.httpd.client.request.resource[httpd_pages[i].name_len] == '?')
				dataServer.httpd.response.params = &dataServer.httpd.client.request.resource[httpd_pages[i].name_len + 1];
			else
				dataServer.httpd.response.params = NULL;
			break;
		}
	}

	if (httpd_pages[i].name == NULL) {
		warning("httpd_read_request: resource not found ('%s')", dataServer.httpd.client.request.resource);
		net_send(dataServer.httpd.client.sock, HTTPD_NOT_FOUND, strlen(HTTPD_NOT_FOUND));
		return -1;
	}

	return i;
}

int httpd_send_response(void) {
	return dataServer.httpd.response.function();
}

void *httpd(void *arg) {
	unsigned char halt = 0;
	dataServer.httpd.listener = net_tcp_ser_init(dataServer.httpd.param.port, &dataServer.httpd.sin);
	if (dataServer.httpd.listener < 0)
		exit(EXIT_FAILURE);

	while (!halt) {
		dataServer.httpd.client.sock = net_tcp_ser_accept(dataServer.httpd.listener, &dataServer.httpd.sin, &dataServer.httpd.client.sin);
		if (dataServer.httpd.client.sock < 0)
			continue;

		if (httpd_read_request() >= 0)
#if 0
			if (httpd_send_response() == HTTPD_HALT)
				halt = 1;
#else
{
	int rv;
			rv = httpd_send_response();
			printf("httpd_send_response() = %d\n", rv);
			if (rv == HTTPD_HALT)
				warning("HALT requested, but I'm going to ignore it!");
}
#endif

		net_close(dataServer.httpd.client.sock);
	}

	net_close(dataServer.httpd.listener);

	printf("httpd daemon done\n");

	return NULL;
}

void httpd_load_page_parts(void) {
	if (dataServer.httpd.page.header_start.text) free(dataServer.httpd.page.header_start.text);
	if (dataServer.httpd.page.meta_refresh.text) free(dataServer.httpd.page.meta_refresh.text);
	if (dataServer.httpd.page.meta_redirect.text) free(dataServer.httpd.page.meta_redirect.text);
	if (dataServer.httpd.page.header_end.text) free(dataServer.httpd.page.header_end.text);
	if (dataServer.httpd.page.sidebar_start.text) free(dataServer.httpd.page.sidebar_start.text);
	if (dataServer.httpd.page.status_fmt.text) free(dataServer.httpd.page.status_fmt.text);
	if (dataServer.httpd.page.config_form_fmt.text) free(dataServer.httpd.page.config_form_fmt.text);
	if (dataServer.httpd.page.forms.text) free(dataServer.httpd.page.forms.text);
	if (dataServer.httpd.page.sidebar_end.text) free(dataServer.httpd.page.sidebar_end.text);
	if (dataServer.httpd.page.footer.text) free(dataServer.httpd.page.footer.text);

	dataServer.httpd.page.header_start.text = file_read_to_char_buf(dataServer.httpd.config.page.header_start.filename);
	dataServer.httpd.page.header_start.len = strlen(dataServer.httpd.page.header_start.text);
	dataServer.httpd.page.meta_refresh.text = file_read_to_char_buf(dataServer.httpd.config.page.meta_refresh.filename);
	dataServer.httpd.page.meta_refresh.len = strlen(dataServer.httpd.page.meta_refresh.text);
	dataServer.httpd.page.meta_redirect.text = file_read_to_char_buf(dataServer.httpd.config.page.meta_redirect.filename);
	dataServer.httpd.page.meta_redirect.len = strlen(dataServer.httpd.page.meta_redirect.text);
	dataServer.httpd.page.header_end.text = file_read_to_char_buf(dataServer.httpd.config.page.header_end.filename);
	dataServer.httpd.page.header_end.len = strlen(dataServer.httpd.page.header_end.text);
	dataServer.httpd.page.sidebar_start.text = file_read_to_char_buf(dataServer.httpd.config.page.sidebar_start.filename);
	dataServer.httpd.page.sidebar_start.len = strlen(dataServer.httpd.page.sidebar_start.text);
	dataServer.httpd.page.status_fmt.text = file_read_to_char_buf(dataServer.httpd.config.page.status_fmt.filename);
	dataServer.httpd.page.status_fmt.len = strlen(dataServer.httpd.page.status_fmt.text);
	dataServer.httpd.page.forms.text = file_read_to_char_buf(dataServer.httpd.config.page.forms.filename);
	dataServer.httpd.page.forms.len = strlen(dataServer.httpd.page.forms.text);
	dataServer.httpd.page.config_form_fmt.text = file_read_to_char_buf(dataServer.httpd.config.page.config_form_fmt.filename);
	dataServer.httpd.page.config_form_fmt.len = strlen(dataServer.httpd.page.config_form_fmt.text);
	dataServer.httpd.page.sidebar_end.text = file_read_to_char_buf(dataServer.httpd.config.page.sidebar_end.filename);
	dataServer.httpd.page.sidebar_end.len = strlen(dataServer.httpd.page.sidebar_end.text);
	dataServer.httpd.page.footer.text = file_read_to_char_buf(dataServer.httpd.config.page.footer.filename);
	dataServer.httpd.page.footer.len = strlen(dataServer.httpd.page.footer.text);
}

void httpd_start(void) {
	dataServer.httpd.param.port = dataServer.httpd.config.port;

	printf("http daemon loading pages into memory\n");
	httpd_load_default_page_locations();
	httpd_load_page_parts();

	printf("http daemon listening on port %d\n", dataServer.httpd.param.port);
	pthread_create(&dataServer.httpd.thread, NULL, httpd, NULL);
}

int httpd_send_header(char *meta, int meta_size) {
	int ret = 0;

	ret = net_send(dataServer.httpd.client.sock, HTTPD_OK, strlen(HTTPD_OK));
	if (ret < 0) goto HTTPD_SEND_HEADER_FAIL;

	ret = net_send(dataServer.httpd.client.sock, dataServer.httpd.page.header_start.text, dataServer.httpd.page.header_start.len);
	if (ret < 0) goto HTTPD_SEND_HEADER_FAIL;

	ret = net_send(dataServer.httpd.client.sock, meta, meta_size);
	if (ret < 0) goto HTTPD_SEND_HEADER_FAIL;

	ret = net_send(dataServer.httpd.client.sock, dataServer.httpd.page.header_end.text, dataServer.httpd.page.header_end.len);
	if (ret < 0) goto HTTPD_SEND_HEADER_FAIL;

	ret = net_send(dataServer.httpd.client.sock, dataServer.httpd.page.sidebar_start.text, dataServer.httpd.page.sidebar_start.len);
	if (ret < 0) goto HTTPD_SEND_HEADER_FAIL;

	return 1;

HTTPD_SEND_HEADER_FAIL:
	return ret;
}

int httpd_send_footer(void) {
	int ret;

	ret = net_send(dataServer.httpd.client.sock, dataServer.httpd.page.sidebar_end.text, dataServer.httpd.page.sidebar_end.len);
	if (ret < 0) goto HTTPD_SEND_FOOTER_FAIL;

	ret = net_send(dataServer.httpd.client.sock, dataServer.httpd.page.footer.text, dataServer.httpd.page.footer.len);
	if (ret < 0) goto HTTPD_SEND_FOOTER_FAIL;

	return 1;

HTTPD_SEND_FOOTER_FAIL:
	return ret;
}

int httpd_send_message_page(char *message, int message_size, int redirect_to_root) {
	int ret = 0;

	if (redirect_to_root)
		ret = httpd_send_header(dataServer.httpd.page.meta_redirect.text, dataServer.httpd.page.meta_redirect.len);
	else
		ret = httpd_send_header(NULL, 0);
	if (ret < 0) goto HTTPD_SEND_MESSAGE_PAGE_FAIL;

	ret = net_send(dataServer.httpd.client.sock, message, message_size);
	if (ret < 0) goto HTTPD_SEND_MESSAGE_PAGE_FAIL;

	ret = httpd_send_footer();
	if (ret < 0) goto HTTPD_SEND_MESSAGE_PAGE_FAIL;

	return 1;

HTTPD_SEND_MESSAGE_PAGE_FAIL:
	return ret;
}

int httpd_send_basic_page(char *message, int message_size, int refresh) {
	int ret = 0;
	char buf[MAX_BUF];

	if (refresh)
		ret = httpd_send_header(dataServer.httpd.page.meta_refresh.text, dataServer.httpd.page.meta_refresh.len);
	else
		ret = httpd_send_header(NULL, 0);
	if (ret < 0) goto HTTPD_SEND_BASIC_PAGE_FAIL;

	snprintf(buf, MAX_BUF, dataServer.httpd.page.status_fmt.text, dataServer.total_packets);
	ret = net_send(dataServer.httpd.client.sock, buf, strlen(buf));

	if (message) {
		ret = net_send(dataServer.httpd.client.sock, message, message_size);
		if (ret < 0) goto HTTPD_SEND_BASIC_PAGE_FAIL;
	}

	ret = httpd_send_footer();
	if (ret < 0) goto HTTPD_SEND_BASIC_PAGE_FAIL;

	return 1;

HTTPD_SEND_BASIC_PAGE_FAIL:
	warning("httpd_send_basic_page: failed send\n");
	return -1;
}

int httpd_page_halt(void) {
	printf("httpd: serving /halt\n");

	httpd_send_basic_page("Bye!\n", 0, 1);

	return -1;
}

int httpd_page_root(void) {
	printf("httpd: serving /\n");

	return httpd_send_basic_page(NULL, 0, 1);
}

httpd_ppm_t httpd_config_ppm[] = { \
	{ "filename", (void *)&dataServer.log_file, &httpd_ppm_charbuf, &dataServer.file_mutex }, \
	{ NULL, NULL, NULL } \
};

int httpd_page_config(void) {
	int message_len, ret;
	char *message;

	if (dataServer.httpd.response.params) {
		printf("httpd: parsing config form\n");

		char *pl, *param;

		for (param = strtok_r(dataServer.httpd.response.params, "&", &pl); param; param = strtok_r(NULL, "&", &pl)) {
			char *name, *value, *nvl;
			name = strtok_r(param, "=", &nvl);
			value = strtok_r(NULL, "=", &nvl);

			httpd_ppm_process(httpd_config_ppm, name, value);
			if (strcmp(name, "filename") == 0) { 
				file_open_log(value);
			}
		}
	}

	printf("httpd: sending config form\n");

	/* order is as-is from struct */
	message = sndup(dataServer.httpd.page.config_form_fmt.text, dataServer.log_file);

	message_len = strlen(message);
	ret = httpd_send_basic_page(message, message_len, 0);

	free(message);

	return ret;
}

int httpd_ppm_process(httpd_ppm_t *ppm, char *name, char *value) {
	int i;

	if (!value)
		value = "";
	
	for (i = 0; ppm[i].name != NULL; ++i) {
		httpd_ppm_t *h = &ppm[i];
		if (strcmp(h->name, name) == 0) {
#ifdef DEBUG
			printf("matched name '%s' to '%s' and will convert '%s' to value w/ func @ 0x%x", name, h->name, value, h->conv_func);
#endif
			return h->conv_func(value, h->ptr, h->mutex);
		}
	}

	printf("httpd_ppm_process: unknown parameter name '%s' (with value '%s') found in resource string", name, value);

	return 0;
}

int httpd_ppm_charbuf(char *value, void *d, pthread_mutex_t *mutex) {
	char **dc = (char **)d, *tmp;

#if 0
	if (*dc != NULL)
		free(*dc);
#endif

	tmp = strdup(value);

	pthread_mutex_lock(mutex);
#ifdef DEBUG
	printf("value '%s' = %s", value, tmp);
#endif
	*dc = tmp;
	pthread_mutex_unlock(mutex);	

	return 1;
}

int httpd_ppm_uint(char *value, void *d, pthread_mutex_t *mutex) {
	unsigned int *ui = (unsigned int *)d;
	unsigned long int ul;
	char *end;

	ul = strtoul(value, &end, 10);
	if (value == end) {
		warning("httpd_ppm_uint: unable to parse unsigned int in string '%s'", value);
		return 0;
	}

	pthread_mutex_lock(mutex);	
#ifdef DEBUG
	printf("value '%s' = %u", value, (unsigned int)ul);
#endif
	*ui = (unsigned int)ul;
	pthread_mutex_unlock(mutex);	

	return 1;
}

int httpd_ppm_ulong(char *value, void *d, pthread_mutex_t *mutex) {
	unsigned long int *ulp = (unsigned long int *)d;
	unsigned long int ul;
	char *end;

	ul = strtoul(value, &end, 10);
	if (value == end) {
		warning("httpd_ppm_ulong: unable to parse unsigned long int in string '%s'", value);
		return 0;
	}

	pthread_mutex_lock(mutex);	
#ifdef DEBUG
	printf("value '%s' = %u", value, (unsigned int)ul);
#endif
	*ulp = ul;
	pthread_mutex_unlock(mutex);	

	return 1;
}

int httpd_ppm_ushort(char *value, void *d, pthread_mutex_t *mutex) {
	unsigned short *us = (unsigned short *)d;
	unsigned long int ul;
	char *end;

	ul = strtoul(value, &end, 10);
	if (value == end) {
		warning("httpd_ppm_ushort: unable to parse unsigned short in string '%s'", value);
		return 0;
	}

	pthread_mutex_lock(mutex);	
#ifdef DEBUG
	printf("value '%s' = %u", value, (unsigned short)ul);
#endif
	*us = (unsigned short)ul;
	pthread_mutex_unlock(mutex);	

	return 1;
}

int httpd_ppm_uchar(char *value, void *d, pthread_mutex_t *mutex) {
	unsigned char *uc = (unsigned char *)d;
	unsigned long int ul;
	char *end;

	ul = strtoul(value, &end, 10);
	if (value == end) {
		warning("httpd_ppm_uchar: unable to parse unsigned byte number in string '%s'", value);
		return 0;
	}

	pthread_mutex_lock(mutex);	
#ifdef DEBUG
	printf("value '%s' = %u", value, (unsigned char)ul);
#endif
	*uc = (unsigned char)ul;
	pthread_mutex_unlock(mutex);	

	return 1;
}

int httpd_ppm_float(char *value, void *d, pthread_mutex_t *mutex) {
	float *f = (float *)d;
	float myf;
	char *end;

	myf = strtof(value, &end);
	if (value == end) {
		warning("httpd_ppm_float: unable to parse floating point value in string '%s'", value);
		return 0;
	}

	pthread_mutex_lock(mutex);	
#ifdef DEBUG
	printf("value '%s' = %g", value, myf);
#endif
	*f = myf;
	pthread_mutex_unlock(mutex);	

	return 1;
}

void httpd_load_default_page_locations(void) {
        if (dataServer.httpd.config.page.header_start.filename) free(dataServer.httpd.config.page.header_start.filename);
        if (dataServer.httpd.config.page.meta_refresh.filename) free(dataServer.httpd.config.page.meta_refresh.filename);
        if (dataServer.httpd.config.page.meta_redirect.filename) free(dataServer.httpd.config.page.meta_redirect.filename);
        if (dataServer.httpd.config.page.header_end.filename) free(dataServer.httpd.config.page.header_end.filename);
        if (dataServer.httpd.config.page.sidebar_start.filename) free(dataServer.httpd.config.page.sidebar_start.filename);
        if (dataServer.httpd.config.page.status_fmt.filename) free(dataServer.httpd.config.page.status_fmt.filename);
        if (dataServer.httpd.config.page.config_form_fmt.filename) free(dataServer.httpd.config.page.config_form_fmt.filename);
        if (dataServer.httpd.config.page.forms.filename) free(dataServer.httpd.config.page.forms.filename);
        if (dataServer.httpd.config.page.sidebar_end.filename) free(dataServer.httpd.config.page.sidebar_end.filename);
        if (dataServer.httpd.config.page.footer.filename) free(dataServer.httpd.config.page.footer.filename);
        dataServer.httpd.config.page.header_start.filename = strdup(HTTPD_DEFAULT_HEADER_START);
        dataServer.httpd.config.page.meta_refresh.filename = strdup(HTTPD_DEFAULT_META_REFRESH);
        dataServer.httpd.config.page.meta_redirect.filename = strdup(HTTPD_DEFAULT_META_REDIRECT);
        dataServer.httpd.config.page.header_end.filename = strdup(HTTPD_DEFAULT_HEADER_END);
        dataServer.httpd.config.page.sidebar_start.filename = strdup(HTTPD_DEFAULT_SIDEBAR_START);
        dataServer.httpd.config.page.status_fmt.filename = strdup(HTTPD_DEFAULT_STATUS_FMT);
        dataServer.httpd.config.page.config_form_fmt.filename = strdup(HTTPD_DEFAULT_CONFIG_FORM_FMT);
        dataServer.httpd.config.page.forms.filename = strdup(HTTPD_DEFAULT_FORMS);
        dataServer.httpd.config.page.sidebar_end.filename = strdup(HTTPD_DEFAULT_SIDEBAR_END);
        dataServer.httpd.config.page.footer.filename = strdup(HTTPD_DEFAULT_FOOTER);
}
