
#include "config_get.h"
/*
    TOPIC SECTION
*/
static topic *topic_node(char *tpc){

    topic *node =NULL;
    node = (topic *)malloc(sizeof(topic));
    if(!node){
        return NULL;
    }
    strncpy(node->name,tpc,127);
    node->next = NULL;

    return node;
}

static void append_topic(topic **head, topic *node){
    
    topic* tmp = *head;
    if(tmp == NULL){
        *head = node;
        return;
    }

    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    tmp->next = node;
}

/*

    EVENT SECTION

*/

static event* event_node(const char* tpc, const char* key, const char*email,const char* password,const char *smtp, const char*recipient, const char *operator, const char *type, const char* value){
    
    event *node =NULL;
    node = (event *)malloc(sizeof(event));
    if(!node){
        return NULL;
    }
    strncpy(node->topic,tpc,127);
    strncpy(node->value,value,31);
    strncpy(node->key,key,31);
    strncpy(node->email,email,63);
    strncpy(node->password,password,63);
    strncpy(node->smtp_server,smtp,31);
    strncpy(node->recipient,recipient,63);
    node->operator=atoi(operator);
    node->type = atoi(type);
    node->next = NULL;
    return node;
}

static void append_event(event **head,event *node){

    event* tmp = *head;
    if(tmp == NULL){
        *head = node;
        return;
    }
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    tmp->next = node;
}

static void delete_event_list(event *head){
    
    event* delete = head;
    while(delete != NULL){
        head = head->next;
        free(delete);
        delete = head;
    }
}

/*

    COMMON SECTION

*/


static void append_to_topics(topic * t_tmp, event *e_tmp){

    while(e_tmp != NULL){

        if(strncmp(t_tmp->name,e_tmp->topic,127) ==0){

            if(t_tmp->event_head == NULL){
                t_tmp->event_head=e_tmp;

            }else{
                t_tmp->event_head->next=e_tmp;
            }

        }
        e_tmp=e_tmp->next; 
    }
}

static void add_events_to_topics(topic **t_head, event **e_head){

    event *e_tmp= *e_head;
    topic *t_tmp = *t_head;
    if (!t_head ||!e_head){
        return;
    }

    while(t_tmp != NULL){

        t_tmp->event_head =NULL;
        append_to_topics(t_tmp,e_tmp);
        t_tmp = t_tmp->next;
    }
}



static void create_topic_list(topic **t_head, struct uci_element *ele, struct uci_option *option){

    uci_foreach_element(&option->v.list, ele){
        
        topic *node =NULL;

        node=topic_node(ele->name);
        if(!node){
            return;
        }
        append_topic(t_head,node);
    }
}

static int uci_get_topics(topic **t_head,char *path){

    int rc = 0;
    struct uci_context *ctx;
    struct uci_element *ele;
    struct uci_package *pack;

    ctx = uci_alloc_context();
    uci_load(ctx,path,&pack);

    struct uci_section *section =uci_lookup_section(ctx,pack,"mqtt_topics_sct");
    if(!section){
        return CONFIG_READ_ERR;
    }
    struct uci_option *option = uci_lookup_option(ctx,section,"topic");
    if(!option){
        return CONFIG_READ_ERR;
    }

    create_topic_list(t_head,ele,option);
    uci_free_context(ctx);
    return rc; 
}


static void create_event_list(struct uci_context *ctx, struct uci_package *pack,struct uci_element *ele,event **e_head){

    uci_foreach_element(&pack->sections,ele){
        
        event *node =NULL;
        struct uci_section *section =uci_to_section(ele);
        if(strncmp(section->type,"event",6)==0){
            node= event_node(uci_lookup_option_string(ctx,section,"topic"),
                uci_lookup_option_string(ctx,section,"key"),
                uci_lookup_option_string(ctx,section,"email"),
                uci_lookup_option_string(ctx,section,"password"),
                uci_lookup_option_string(ctx,section,"smtp"),
                uci_lookup_option_string(ctx,section,"recipient"),
                uci_lookup_option_string(ctx,section,"operator"),
                uci_lookup_option_string(ctx,section,"type"),
                uci_lookup_option_string(ctx,section,"value")
            );
            append_event(e_head,node);
        }
    }   
}

static int uci_get_events(event **e_head,char *path){

    int rc = 0;
    struct uci_context *ctx;
    struct uci_element *ele;
    struct uci_package *pack;
    ctx = uci_alloc_context();
    if(!ctx){
        return CONFIG_READ_ERR;
    }
    uci_load(ctx,path,&pack);
    if(!pack){
        uci_perror(ctx, "uci_load()");
        uci_free_context(ctx);
        return CONFIG_READ_ERR;
    }

    create_event_list(ctx,pack,ele,e_head);

    uci_free_context(ctx);
    return 0;
}

/*usable functions*/

int load_uci_config(topic **t_head, event **e_head, char* config_name){

    int rc =0;
    rc = uci_get_topics(t_head,config_name);
    if (rc != UCI_OK){
        syslog(LOG_ERR,"failed to load config err: %d",rc);
        return rc;
    }
    rc = uci_get_events(e_head,config_name);
    if (rc != UCI_OK){
        syslog(LOG_ERR,"failed to load config err: %d",rc);
        return rc;
    }
    add_events_to_topics(t_head,e_head);

    return rc;
}

void delete_full_list(topic *t_head){

    topic* delete = t_head;
    while(delete != NULL){

        if (t_head->event_head !=NULL){
            delete_event_list(t_head->event_head);
        }

        t_head = t_head->next;
        free(delete);
        delete = t_head;

    }
}


