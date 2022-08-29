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