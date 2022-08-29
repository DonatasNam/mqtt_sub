#include <mosquitto.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


#ifndef CONFIG_H
#include "config_get.h"
#define CONFIG_H
#endif
#ifndef DB_H
#include "sub_db.h"
#define DB_H
#endif

struct mosq_conf{

    /***conf data ***/
    char * host;
    int port;
    int keepalive;
    int mid;

    /***sub data ***/
    char *topics;
    int t_count;

    /*** User credentials ***/
    bool credentials;
    const char *username;
    const char *password;

    /*** TLS certificate files ***/
    bool tls;
    const char *cafile;
    const char *capath;
    const char *cerfile;
    const char *keyfile;

    /*** message data***/
    void *u_payload;
    char *u_topic;
};


void on_connect_cb(struct mosquitto *mosq, void *obj, int result);
void on_message_cb(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

struct mosquitto *mqtt_init(int *err, struct mosq_conf*mconf);
int mqtt_start(struct mosquitto *mosq, struct mosq_conf *mconf, topic **head_ref);
int bulk_sub(struct mosquitto *mosq,topic **head_ref);


