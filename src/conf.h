/* Configuration handling include file */

#ifndef AXEL_CONF_H
#define AXEL_CONF_H

typedef struct
{
    char default_filename[MAX_STRING];
    char http_proxy[MAX_STRING];
    char no_proxy[MAX_STRING];
    int strip_cgi_parameters;
    int save_state_interval;
    int connection_timeout;
    int reconnect_delay;
    int num_connections;
    int max_redirect;
    int buffer_size;
    int max_speed;
    int verbose;
    int alternate_output;
    int insecure;
    int no_clobber;

    if_t *interfaces;

    sa_family_t ai_family;

    int search_timeout;
    int search_threads;
    int search_amount;
    int search_top;

    int add_header_count;
    char add_header[MAX_ADD_HEADERS][MAX_STRING];

    char user_agent[MAX_STRING];
} conf_t;

int conf_loadfile(conf_t *conf, char *file);
int conf_init(conf_t *conf);
void conf_free(conf_t *conf);

#endif /* AXEL_CONF_H */
