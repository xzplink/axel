/* Main include file */

#ifndef AXEL_AXEL_H
#define AXEL_AXEL_H

#include "config.h"

#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#ifndef NOGETOPTLONG
#define _GNU_SOURCE
#include <getopt.h>
#endif
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

/* Internationalization */
#define PACKAGE         "axel"
#define _(x)          gettext(x)
#include <libintl.h>
#include <locale.h>

/* Compiled-in settings */
#define MAX_STRING      1024
#define MAX_ADD_HEADERS 10
#define MAX_REDIRECT        20
#define DEFAULT_USER_AGENT  "Axel " VERSION " (" ARCH ")"

typedef struct
{
    void *next;
    char text[MAX_STRING];
} message_t;

typedef message_t url_t;
typedef message_t if_t;

#include "conf.h"
#include "tcp.h"
#include "ftp.h"
#include "http.h"
#include "conn.h"
#include "ssl.h"
#include "search.h"

#define min(a, b)     ((a) < (b) ? (a) : (b))
#define max(a, b)     ((a) > (b) ? (a) : (b))

typedef struct
{
    conn_t *conn;
    conf_t *conf;
    char filename[MAX_STRING];
    double start_time;
    int next_state, finish_time;
    long long bytes_done, start_byte, size;
    int bytes_per_second;
    struct timespec delay_time;
    int outfd;
    int ready;
    message_t *message, *last_message;
    url_t *url;
} axel_t;

axel_t *axel_new(conf_t *conf, int count, const void *url);
int axel_open(axel_t *axel);
void axel_start(axel_t *axel);
void axel_do(axel_t *axel);
void axel_close(axel_t *axel);
void print_messages(axel_t *axel);

double gettime();

static inline int axel_nanosleep(struct timespec delay)
{
    int res;
    while((res = nanosleep(&delay, &delay)) && errno == EINTR);
    return res;
}

#endif /* AXEL_AXEL_H */
