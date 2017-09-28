/* SSL interface */

#ifndef AXEL_SSL_H
#define AXEL_SSL_H

#ifdef HAVE_SSL

void ssl_init(conf_t *conf);
SSL* ssl_connect(int fd, char *hostname, char *message);
void ssl_disconnect(SSL *ssl);

#endif /* HAVE_SSL */

#endif /* AXEL_SSL_H */
