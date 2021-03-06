/* HTTP control file */

#include "axel.h"

inline static char chain_next(const char ***p)
{
    while (**p && !***p)++ *p;
    return **p ? *(**p)++ : 0;
}

static void http_auth_token(char *token, const char *user, const char *pass)
{
    const char base64_encode[64] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    const char *auth[] = { user, ":", pass, NULL };
    const char **p = auth;

    while (*p && **p)
    {
        char a = chain_next(&p);
        *token++ = base64_encode[ a >> 2 ];
        char b = chain_next(&p);
        *token++ = base64_encode[ ((a & 3) << 4) | (b >> 4) ];
        if (!b)
        {
            *token++ = '=';
            *token++ = '=';
            break;
        }
        else
        {
            char c = chain_next(&p);
            *token++ = base64_encode[((b & 15) << 2)
                         | (c >> 6)];
            if (!c)
            {
                *token++ = '=';
                break;
            }
            else
                *token++ = base64_encode[c & 63];
        }
    }
}

int http_connect(http_t *conn, int proto, char *proxy, char *host, int port, char *user, char *pass)
{
    char *puser, *ppass;
    conn_t tconn[1];

    strncpy(conn->host, host, MAX_STRING);
    conn->port = port;
    conn->proto = proto;

    if (proxy != NULL) { if (*proxy != 0)
    {
        sprintf(conn->host, "%s:%i", host, port);
        if (!conn_set(tconn, proxy))
        {
            /* We'll put the message in conn->headers, not in request */
            sprintf(conn->headers, _("Invalid proxy string: %s\n"), proxy);
            return(0);
        }
        host = tconn->host;
        port = tconn->port;
        proto = tconn->proto;
        puser = tconn->user;
        ppass = tconn->pass;
        conn->proxy = 1;
    }
    else
    {
        conn->proxy = 0;
    } }

    if (tcp_connect(&conn->tcp, host, port, PROTO_IS_SECURE(proto),
        conn->local_if, conn->headers) == -1)
        return(0);

    if (*user == 0)
    {
        *conn->auth = 0;
    }
    else
    {
        http_auth_token(conn->auth, user, pass);
    }

    if (!conn->proxy || *puser == 0)
    {
        *conn->proxy_auth = 0;
    }
    else
    {
        http_auth_token(conn->proxy_auth, puser, ppass);
    }

    return(1);
}

void http_disconnect(http_t *conn)
{
    tcp_close(&conn->tcp);
}

void http_get(http_t *conn, char *lurl)
{
    *conn->request = 0;
    if (conn->proxy)
    {
        const char* proto = scheme_from_proto(conn->proto);
        http_addheader(conn, "GET %s%s%s HTTP/1.0", proto, conn->host, lurl);
    }
    else
    {
        http_addheader(conn, "GET %s HTTP/1.0", lurl);
        if ((conn->proto == PROTO_HTTP && conn->port != PROTO_HTTP_PORT) ||
             (conn->proto == PROTO_HTTPS && conn->port != PROTO_HTTPS_PORT))
            http_addheader(conn, "Host: %s:%i", conn->host, conn->port);
        else
            http_addheader(conn, "Host: %s", conn->host);
    }
    if (*conn->auth)
        http_addheader(conn, "Authorization: Basic %s", conn->auth);
    if (*conn->proxy_auth)
        http_addheader(conn, "Proxy-Authorization: Basic %s", conn->proxy_auth);
    if (conn->firstbyte)
    {
        if (conn->lastbyte)
            http_addheader(conn, "Range: bytes=%lld-%lld", conn->firstbyte, conn->lastbyte);
        else
            http_addheader(conn, "Range: bytes=%lld-", conn->firstbyte);
    }
}

void http_addheader(http_t *conn, char *format, ...)
{
    char s[MAX_STRING];
    va_list params;

    va_start(params, format);
    vsnprintf(s, sizeof(s) - 3, format, params);
    strcat(s, "\r\n");
    va_end(params);

    strncat(conn->request, s, MAX_QUERY - strlen(conn->request) - 1);
}

int http_exec(http_t *conn)
{
    int i = 0;
    ssize_t nwrite = 0;
    char s[2] = " ", *s2;

#ifdef DEBUG
    fprintf(stderr, "--- Sending request ---\n%s--- End of request ---\n", conn->request);
#endif

    http_addheader(conn, "");

    while (nwrite < strlen(conn->request)) {
        if ((i = tcp_write(&conn->tcp, conn->request + nwrite, strlen(conn->request) - nwrite)) < 0) {
    if (errno == EINTR || errno == EAGAIN) continue;
            /* We'll put the message in conn->headers, not in request */
            sprintf(conn->headers, _("Connection gone while writing.\n"));
            return(0);
        }
        nwrite += i;
    }

    *conn->headers = 0;
    /* Read the headers byte by byte to make sure we don't touch the
       actual data */
    while (1)
    {
        if (tcp_read(&conn->tcp, s, 1) <= 0)
        {
            /* We'll put the message in conn->headers, not in request */
            sprintf(conn->headers, _("Connection gone.\n"));
            return(0);
        }
        if (*s == '\r')
        {
            continue;
        }
        else if (*s == '\n')
        {
            if (i == 0)
                break;
            i = 0;
        }
        else
        {
            i++;
        }
        strncat(conn->headers, s, MAX_QUERY - strlen(conn->headers) - 1);
    }

#ifdef DEBUG
    fprintf(stderr, "--- Reply headers ---\n%s--- End of headers ---\n", conn->headers);
#endif

    sscanf(conn->headers, "%*s %3i", &conn->status);
    s2 = strchr(conn->headers, '\n'); *s2 = 0;
    strcpy(conn->request, conn->headers);
    *s2 = '\n';

    return(1);
}

const char *http_header(const http_t *conn, const char *header)
{
    const char *p = conn->headers;
    size_t hlen = strlen(header);

    do
    {
        if (strncasecmp(p, header, hlen) == 0)
            return(p + hlen);
        while (*p != '\n' && *p)
            p++;
        if (*p == '\n')
            p++;
    }
    while (*p);

    return(NULL);
}

long long int http_size(http_t *conn)
{
    const char *i;
    long long int j;

    if ((i = http_header(conn, "Content-Length:")) == NULL)
        return(-2);

    sscanf(i, "%lld", &j);
    return(j);
}

long long int http_size_from_range(http_t *conn)
{
    const char *i;
    long long int j;

    if ((i = http_header(conn, "Content-Range:")) == NULL)
        return(-2);

    i = strchr(i, '/');
    if (i == NULL)
        return(-2);

    if (sscanf(i + 1, "%lld", &j) != 1)
        return(-3);

    return(j);
}

void http_filename(const http_t *conn, char *filename)
{
    const char *h;
    if ((h = http_header(conn, "Content-Disposition:")) != NULL) {
        sscanf(h, "%*s%*[ \t]filename%*[ \t=\"\']%254[^\n\"\' ]", filename);

        http_decode(filename);

        /* Replace common invalid characters in filename
          https://en.wikipedia.org/wiki/Filename#Reserved_characters_and_words */
        char *i = filename;
        const char *invalid_characters = "/\\?%*:|<>";
        const char replacement = '_';
        while ((i = strpbrk(i, invalid_characters)) != NULL) {
            *i = replacement;
            i++;
        }
    }
}

inline static char decode_nibble(char n)
{
    if (n <= '9')
        return(n - '0');
    if (n >= 'a')
        n -= 'a' - 'A';
    return(n - 'A' + 10);
}

inline static char encode_nibble(char n)
{
    return(n > 9 ? n + 'a' - 10 : n + '0');
}

inline static void encode_byte(char dst[3], char n)
{
    *dst++ = '%';
    *dst++ = encode_nibble(n >> 4);
    *dst = encode_nibble(n & 15);
}

/* Decode%20a%20file%20name */
void http_decode(char *s)
{
    for (; *s && *s != '%'; s++);
    if (!*s)
        return;

    char *p = s;
    do {
        if (!s[1] || !s[2])
            break;
        *p++ = (decode_nibble(s[1]) << 4) | decode_nibble(s[2]);
        s += 3;
        while (*s && *s != '%')
            *p++ = *s++;
    } while (*s == '%');
    *p = 0;
}

void http_encode(char *s)
{
    char t[MAX_STRING];
    int i, j;

    for (i = j = 0; s[i]; i++, j++)
    {
        /* Fix buffer overflow */
        if (j >= MAX_STRING - 1) {
            break;
        }

        t[j] = s[i];
        if (s[i] <= 0x20 || s[i] >= 0x7f)
        {
            /* Fix buffer overflow */
            if (j >= MAX_STRING - 3) {
                break;
            }

            encode_byte(t + j, s[i]);
            j += 2;
        }
    }
    t[j] = 0;

    strcpy(s, t);
}
