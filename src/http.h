/* HTTP control include file */

#ifndef AXEL_HTTP_H
#define AXEL_HTTP_H

#define MAX_QUERY   2048        /* Should not grow larger.. */

typedef struct
{
    char host[MAX_STRING];
    char auth[MAX_STRING];
    char request[MAX_QUERY];
    char headers[MAX_QUERY];
    int port;
    int proto;          /* FTP through HTTP proxies */
    int proxy;
    char proxy_auth[MAX_STRING];
    long long int firstbyte;
    long long int lastbyte;
    int status;
    tcp_t tcp;
    char *local_if;
} http_t;

int http_connect(http_t *conn, int proto, char *proxy, char *host, int port, char *user, char *pass);
void http_disconnect(http_t *conn);
void http_get(http_t *conn, char *lurl);
void http_addheader(http_t *conn, char *format, ...);
int http_exec(http_t *conn);
const char *http_header(const http_t *conn, const char *header);
void http_filename(const http_t *conn, char *filename);
long long int http_size(http_t *conn);
long long int http_size_from_range(http_t *conn);
void http_encode(char *s);
void http_decode(char *s);

#endif /* AXEL_HTTP_H */
