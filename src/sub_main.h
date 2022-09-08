#include "sub_mqtt.h"
#include <syslog.h>
#include <signal.h>
#include <unistd.h>


#ifndef CONFIG_H
#include "config_get.h"
#endif

#define SYSLOG_H
#define CONFIG_H

volatile sig_atomic_t stop;

char arg_doc[] = "host port keep-alive username password ca-path ca-file key-file";

struct argp_option options[]= {
    {"host",        'h', "HOST",        0, "MQTT broker host address"},
    {"port",        'p', "PORT",        0, "Port for MQTT connection"},
    {"keep-alive",  'k', "KEEPALIVE",   0, "Seconds before subscriber sends acknowledgement request"},
    {"username",    'U', "USRNAME",     OPTION_ARG_OPTIONAL, "username for mqtt broker"},
    {"password",    'P', "PASS",        OPTION_ARG_OPTIONAL, "password for mqtt broker"},
    {"ca-path",     'C', "PATH",        OPTION_ARG_OPTIONAL, "name of domain crt file with path"},
    {"ca-file",     'F', "FILE",        OPTION_ARG_OPTIONAL, "name of client crt file with path"},
    {"key-file",    'K', "KEY",        OPTION_ARG_OPTIONAL, "name of client key file with path"},
    {0}
};

error_t parse_opt(int key, char *arg, struct argp_state *state);
struct argp argp = {options, parse_opt, arg_doc};

void sig_hndlr(int signum);