#include <uci.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef SYSLOG_H
#include <syslog.h>
#define SYSLOG_H
#endif

typedef enum{

    EVENT_FALSE = 0,
    EVENT_TRUE = 100,
    INVALID_OP = 101,
    INVALID_TYPE = 102,
    PARSE_ERR = 103,
    CONFIG_READ_ERR = 104
}event_ret;

typedef enum{

    TLS_ON = 200,
    CREDENTIALS_ON = 201
}config_opts;

typedef struct event
{
    char topic[127];
    char key[32];
    char value[32];
    char email[64];
    char password[64];
    char recipient[64];
    char smtp_server[32];
    int operator;
    int type;
    struct event *next;
}event;

typedef struct topic{
    char name[128];
    event *event_head;
    struct topic *next;
}topic;

typedef struct config_data{

    /*array for argp parser*/
    char *args[3];
    /***conf data ***/
    char *host;
    int port;
    int keepalive;

    /*** User credentials ***/
    int credentials;
    const char *username;
    const char *password;

    /*** TLS certificate files ***/
    int tls;
    const char *ca_path;
    /*** list from congig file***/

    topic *topic;

}config;



static topic *topic_node(char *tpc);
static void append_topic(topic **head, topic *node);
static event* event_node(const char* tpc, const char* key, const char*email,const char* password,const char * smtp, const char*recipient, const char *operator, const char *type, const char* value);
static void append_event(event **head,event *node);
static void delete_event_list(event *head);
static void append_to_topics(topic * t_tmp, event *e_tmp);
static void add_events_to_topics(topic **t_head, event **e_head);
static void create_topic_list(topic **t_head, struct uci_element *ele, struct uci_option *option);
static int uci_get_topics(topic **t_head,char *path);
static void create_event_list(struct uci_context *ctx, struct uci_package *pack,struct uci_element *ele,event **e_head);
static int uci_get_events(event **e_head,char *path);

int load_uci_config(topic **t_head, event **e_head, char* config_name);
void delete_full_list(topic *t_head);
