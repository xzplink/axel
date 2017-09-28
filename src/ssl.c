/* SSL interface */

#include "axel.h"

#ifdef HAVE_SSL

#include <openssl/err.h>

static pthread_mutex_t ssl_lock;
static bool ssl_inited = false;
static conf_t *conf = NULL;

void ssl_init(conf_t *global_conf)
{
    pthread_mutex_init(&ssl_lock, NULL);
    conf = global_conf;
}

void ssl_startup(void)
{
    pthread_mutex_lock(&ssl_lock);
    if (!ssl_inited)
    {
        SSL_library_init();
        SSL_load_error_strings();

        ssl_inited = true;
    }
    pthread_mutex_unlock(&ssl_lock);
}

SSL* ssl_connect(int fd, char *hostname, char *message)
{

    SSL_CTX* ssl_ctx;
    SSL* ssl;

    ssl_startup();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (!conf->insecure) {
        SSL_CTX_set_default_verify_paths(ssl_ctx);
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
    }
    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);

    ssl = SSL_new(ssl_ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_tlsext_host_name(ssl, hostname);

    int err = SSL_connect(ssl);
    if (err <= 0) {
        sprintf(message, _("SSL error: %s\n"), ERR_reason_error_string(ERR_get_error()));
        return NULL;
    }

    return ssl;
}

void ssl_disconnect(SSL *ssl)
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

#endif /* HAVE_SSL */
