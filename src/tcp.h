/* TCP control include file */

#ifndef AXEL_TCP_H
#define AXEL_TCP_H

typedef struct {
    int fd;
    sa_family_t ai_family;
#ifdef HAVE_SSL
    SSL *ssl;
#endif
} tcp_t;

int tcp_connect(tcp_t *tcp, char *hostname, int port, int secure, char *local_if, char *message);
void tcp_close(tcp_t *tcp);

int tcp_read(tcp_t *tcp, void *buffer, int size);
int tcp_write(tcp_t *tcp, void *buffer, int size);

int get_if_ip(char *iface, char *ip);

#endif /* AXEL_TCP_H */
