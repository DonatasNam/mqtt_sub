#include <mosquitto.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include <argp.h>
#include <argp.h>

#ifndef CONFIG_H
#include "config_get.h"
#define CONFIG_H
#endif
#ifndef DB_H
#include "sub_db.h"
#define DB_H
#endif

#define MORE 10
#define LESS 11
#define EQUAL 12
#define NEQUAL 13
#define MORE_EQ 14
#define LESS_EQ 15

void on_message_cb(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
static int bulk_sub(struct mosquitto *mosq,topic **t_head);
static char* payload_parse(char *payload,char *key);
static int evaluate_event_int(int value, int expected, int operator);
static int evaluate_event_string(char* value, char *expected, int operator);
static int evaluate_event(char *payload, char* exp, char *key, int operator, int type);
static int event_handler(event *ev_head, char *payload);
static int check_for_event(topic *t_head,char *tpc, char *payload);

struct mosquitto *mqtt_init(config *conf);
int mqtt_start(struct mosquitto *mosq, config *conf, topic **head_ref);


