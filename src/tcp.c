/* TCP control file */

#include "axel.h"

static void tcp_error(char *buffer, char *hostname, int port, const char *reason)
{
    sprintf(buffer, _("Unable to connect to server %s:%i: %s\n"),
        hostname, port, reason);
}

/* Get a TCP connection */
int tcp_connect(tcp_t *tcp, char *hostname, int port, int secure, char *local_if, char *message)
{
    struct sockaddr_in local_addr;
    const int portstr_len = 10;
    char portstr[portstr_len];
    struct addrinfo ai_hints;
    struct addrinfo *gai_results, *gai_result;
    int ret;
    int sock_fd = -1;


    if (tcp->ai_family == AF_INET && local_if && *local_if) {
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = 0;
        local_addr.sin_addr.s_addr = inet_addr(local_if);
    }

    snprintf(portstr, portstr_len, "%d", port);

    memset(&ai_hints, 0, sizeof(ai_hints));
    ai_hints.ai_family = tcp->ai_family;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_flags = AI_ADDRCONFIG;
    ai_hints.ai_protocol = 0;

    ret = getaddrinfo(hostname, portstr, &ai_hints, &gai_results);
    if (ret != 0) {
        tcp_error(message, hostname, port, gai_strerror(ret));
        return -1;
    }

    gai_result = gai_results;
    sock_fd = -1;
    while ((sock_fd == -1) && (gai_result != NULL)) {

        sock_fd = socket(gai_result->ai_family,
            gai_result->ai_socktype, gai_result->ai_protocol);

        if (sock_fd != -1) {

            if (gai_result->ai_family == AF_INET) {
                if (local_if && *local_if) {
                    ret = bind(sock_fd,
                        (struct sockaddr *) &local_addr,
                        sizeof(local_addr));
                    if (ret == -1) {
                        close(sock_fd);
                        sock_fd = -1;
                        gai_result =
                            gai_result->ai_next;
                    }
                }
            }

            if (sock_fd != -1) {
                ret = connect(sock_fd, gai_result->ai_addr,
                        gai_result->ai_addrlen);
                if (ret == -1) {
                    close(sock_fd);
                    sock_fd = -1;
                    gai_result = gai_result->ai_next;
                }
            }
        }
    }

    freeaddrinfo(gai_results);

    if (sock_fd == -1) {
        tcp_error(message, hostname, port, strerror(errno));
        return -1;
    }

#ifdef HAVE_SSL
    if (secure) {
        tcp->ssl = ssl_connect(sock_fd, hostname, message);
        if (tcp->ssl == NULL) {
            close(sock_fd);
            return -1;
        }
    }
#endif /* HAVE_SSL */
    tcp->fd = sock_fd;

    return 1;
}

int tcp_read(tcp_t *tcp, void *buffer, int size)
{
#ifdef HAVE_SSL
    if (tcp->ssl != NULL)
        return SSL_read(tcp->ssl, buffer, size);
    else
#endif /* HAVE_SSL */
        return read(tcp->fd, buffer, size);
}

int tcp_write(tcp_t *tcp, void *buffer, int size)
{
#ifdef HAVE_SSL
    if (tcp->ssl != NULL)
        return SSL_write(tcp->ssl, buffer, size);
    else
#endif /* HAVE_SSL */
        return write(tcp->fd, buffer, size);
}

void tcp_close(tcp_t *tcp)
{
    if (tcp->fd > 0) {
#ifdef HAVE_SSL
        if (tcp->ssl != NULL)
        {
            ssl_disconnect(tcp->ssl);
            tcp->ssl = NULL;
        }
        else
#endif /* HAVE_SSL */
            close(tcp->fd);
        tcp->fd = -1;
    }
}

int get_if_ip(char *iface, char *ip)
{
    struct ifreq ifr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    memset(&ifr, 0, sizeof(struct ifreq));

    strcpy(ifr.ifr_name, iface);
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
    {
        struct sockaddr_in *x = (struct sockaddr_in *) &ifr.ifr_addr;
        strcpy(ip, inet_ntoa(x->sin_addr));
        return(1);
    }
    else
    {
        return(0);
    }
}
