#ifndef __HTTPD_H__
#define __HTTPD_H__

#define HTTPD_DEFAULT_PORT 8080

#define MAX_REQUEST_SIZE 4096

#define HTTPD_OK "HTTP/1.1 200 OK\r\n\r\n"
#define HTTPD_METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed\r\n\r\n<HEAD><TITLE>Method Not Allowed</TITLE></HEAD>\n<BODY><H1>Method Not Allowed</H1></BODY>\n"
#define HTTPD_NOT_FOUND "HTTP/1.1 404 Not found\r\n\r\n<HEAD><TITLE>File Not Found</TITLE></HEAD>\n<BODY><H1>File Not Found</H1></BODY>\n"

#define HTTPD_OK_PNG "HTTP/1.1 200 OK\r\nContent-Type:image/png\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nExpires: -1\r\n\r\n"

#define HTTPD_GET "GET "
#define HTTPD_GET_LINE "GET %s HTTP/1.1"

#define HTTPD_DEFAULT_HEADER_START "defaults/header_start"
#define HTTPD_DEFAULT_META_REFRESH "defaults/meta_refresh"
#define HTTPD_DEFAULT_META_REDIRECT "defaults/meta_redirect"
#define HTTPD_DEFAULT_HEADER_END "defaults/header_end"
#define HTTPD_DEFAULT_SIDEBAR_START "defaults/sidebar_start"
#define HTTPD_DEFAULT_STATUS_FMT "defaults/status_fmt"
#define HTTPD_DEFAULT_FORMS "defaults/forms"
#define HTTPD_DEFAULT_CONFIG_FORM_FMT "defaults/config_form_fmt"
#define HTTPD_DEFAULT_LOG_START "defaults/log_start"
#define HTTPD_DEFAULT_LOG_INTERLEAVE "defaults/log_interleave"
#define HTTPD_DEFAULT_LOG_END "defaults/log_end"
#define HTTPD_DEFAULT_SIDEBAR_END "defaults/sidebar_end"
#define HTTPD_DEFAULT_FOOTER "defaults/footer"

#define HTTPD_MESSAGE_CONFIGURATION_SAVED "Configuration saved."
#define HTTPD_MESSAGE_CONFIGURATION_UPDATED "Configuration updated."

#define HTTPD_HALT -1

typedef struct httpd {
	pthread_t thread;
	struct param {
		int port;
	} param;
	int listener;
	struct sockaddr_in sin;
	struct page {
		struct part {
			char *text;
			int len;
		} header_start, meta_refresh, meta_redirect, header_end, sidebar_start, status_fmt, config_form_fmt, forms, log_start, log_interleave, log_end, sidebar_end, footer;
	} page;
        struct config_httpd {
                uint32_t port;
                struct mcp_config_httpd_page {
                        struct mcp_config_httpd_page_part {
                                char *filename;
                        } header_start, meta_refresh, meta_redirect, header_end, sidebar_start, status_fmt, config_form_fmt, forms, log_start, log_interleave, log_end, sidebar_end, footer;
                } page;
        } config; 
	struct client {
		int sock;
		struct sockaddr_in sin;
		struct request {
			char *text;
			char *resource;
			int len;
			int size;
		} request;
	} client;
	struct response {
		int (*function)(void);
		char *params;
		struct httpd_response_image {
			char *name;
			unsigned int height;
			unsigned int width;
		} image;
	} response;
} httpd_t;

typedef struct httpd_pages {
	char	*name;
	int	name_len;
	int (*function)(void);
} httpd_pages_t;

typedef struct httpd_param_parse_map {
	char *name;
	void *ptr;
	int (*conv_func)(char *value, void *d, pthread_mutex_t *);
	pthread_mutex_t *mutex;
} httpd_ppm_t;

void *httpd(void *arg);
void httpd_start(void);

/* httpd/pages.c */
int httpd_page_config(void);
int httpd_page_halt(void);
int httpd_page_root(void);

/* httpd/param.c */
int httpd_ppm_charbuf(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_uint(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_ulong(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_uchar(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_ushort(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_float(char *value, void *d, pthread_mutex_t *mutex);
int httpd_ppm_process(httpd_ppm_t *httpd_config_ppm, char *name, char *value);

#endif /* __HTTPD_H__ */
