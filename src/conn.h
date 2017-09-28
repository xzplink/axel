/* Connection stuff */

#ifndef AXEL_CONN_H
#define AXEL_CONN_H

#define PROTO_SECURE_MASK   (1<<0)  /* bit 0 - 0 = insecure, 1 = secure */
#define PROTO_PROTO_MASK    (1<<1)  /* bit 1 = 0 = ftp,      1 = http   */

#define PROTO_INSECURE      (0<<0)
#define PROTO_SECURE        (1<<0)
#define PROTO_PROTO_FTP     (0<<1)
#define PROTO_PROTO_HTTP    (1<<1)

#define PROTO_IS_FTP(proto) \
    (((proto) & PROTO_PROTO_MASK) == PROTO_PROTO_FTP)
#define PROTO_IS_SECURE(proto) \
    (((proto) & PROTO_SECURE_MASK) == PROTO_SECURE)

#define PROTO_FTP       (PROTO_PROTO_FTP|PROTO_INSECURE)
#define PROTO_FTP_PORT      21

#define PROTO_FTPS      (PROTO_PROTO_FTP|PROTO_SECURE)
#define PROTO_FTPS_PORT     990

#define PROTO_HTTP      (PROTO_PROTO_HTTP|PROTO_INSECURE)
#define PROTO_HTTP_PORT     80

#define PROTO_HTTPS     (PROTO_PROTO_HTTP|PROTO_SECURE)
#define PROTO_HTTPS_PORT    443

#define PROTO_DEFAULT          PROTO_HTTP
#define PROTO_DEFAULT_PORT     PROTO_HTTP_PORT

typedef struct
{
    conf_t *conf;

    int proto;
    int port;
    int proxy;
    char host[MAX_STRING];
    char dir[MAX_STRING];
    char file[MAX_STRING];
    char user[MAX_STRING];
    char pass[MAX_STRING];
    char output_filename[MAX_STRING];

    ftp_t ftp[1];
    http_t http[1];
    long long int size;     /* File size, not 'connection size'.. */
    long long int currentbyte;
    long long int lastbyte;
    tcp_t *tcp;
    bool enabled;
    bool supported;
    int last_transfer;
    char *message;
    char *local_if;

    bool state;
    pthread_t setup_thread[1];
    pthread_mutex_t lock;
} conn_t;

int conn_set(conn_t *conn, const char *set_url);
char *conn_url(conn_t *conn);
void conn_disconnect(conn_t *conn);
int conn_init(conn_t *conn);
int conn_setup(conn_t *conn);
int conn_exec(conn_t *conn);
int conn_info(conn_t *conn);
const char *scheme_from_proto(int proto);

#endif /* AXEL_CONN_H */
