/* FTP control include file */

#ifndef AXEL_FTP_H
#define AXEL_FTP_H

#define FTP_PASSIVE 1
#define FTP_PORT    2

typedef struct
{
    char cwd[MAX_STRING];
    char *message;
    int status;
    tcp_t tcp;
    tcp_t data_tcp;
    int proto;
    int ftp_mode;
    char *local_if;
} ftp_t;

int ftp_connect(ftp_t *conn, int proto, char *host, int port, char *user, char *pass);
void ftp_disconnect(ftp_t *conn);
int ftp_wait(ftp_t *conn);
int ftp_command(ftp_t *conn, char *format, ...);
int ftp_cwd(ftp_t *conn, char *cwd);
int ftp_data(ftp_t *conn);
long long int ftp_size(ftp_t *conn, char *file, int maxredir);

#endif /* AXEL_FTP_H */
