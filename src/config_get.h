#include <uci.h>
#include <string.h>
#include <stdlib.h>

typedef struct topic{
    char name[128];
    struct topic *next;
}topic;

topic *node_create(char *string);
void append_list(struct topic **head_ref, topic *node);
void delete_list(topic *head_ref);
int config_entry_list (topic **head_ref,char *path);
