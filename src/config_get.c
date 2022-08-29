#include "config_get.h"


topic *node_create(char *string){

    topic *node = NULL;
    node = (topic *)malloc(sizeof(topic));
    if(!node){
        return NULL;
    }
    strncpy(node->name,string,127);
    node->next = NULL;
    return node;
}

void append_list(struct topic **head_ref, topic *node){

    topic* tmp = *head_ref;
    if(tmp == NULL){
        *head_ref = node;
        return;
    }
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    tmp->next = node;
}


void delete_list(topic *head_ref){

    topic* delete = head_ref;
    while(delete != NULL){
        head_ref = head_ref->next;
        free(delete);
        delete = head_ref;
    }
}


int config_entry_list (topic **head_ref,char *path){

    struct uci_element *ele;
    struct uci_context *ctx;
    struct uci_ptr ptr;

    int rc =0;
    ctx = uci_alloc_context ();
    if(!ctx){
        return rc;
    }
    rc =uci_lookup_ptr (ctx, &ptr, path, true);
    if (rc != UCI_OK){
      return rc;
    }
    switch (ptr.o->type)
    {
    case UCI_TYPE_LIST:
        uci_foreach_element(&ptr.o->v.list, ele) {
            topic *node =NULL;

            node=node_create(ele->name);
            if(!node){
                return -1;
            }
            append_list(head_ref,node);

        }
        break;
    
    default:
        return UCI_ERR_UNKNOWN;
        break;
    }
    uci_free_context(ctx);

    return 0;
}
