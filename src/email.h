#ifndef CONFIG_H
#include "config_get.h"
#define CONFIG_H
#endif

#include <curl/curl.h>

#define SSL_CERT_PATH "/etc/ssl/certs/ca-certificates.crt"


struct upload_status {
  int bytes_read;
  char *email_msg;
};

static char *mail_message(char *recipient, char *email, char *subject, char *msg);
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp);
int send_mail(event *ev_head,char *payload);