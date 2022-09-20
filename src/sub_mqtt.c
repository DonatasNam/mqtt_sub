#include "sub_mqtt.h"

/*
    MQTT CONNECTION AND MESSAGES
*/
void on_message_cb(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{   
    int rc = 0;
    struct json_object *ob;
    config *conf;
    conf = (config *)obj;
    rc =db_add(db,message->topic,(char*)message->payload);
    if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
    }
    ob =json_tokener_parse((char*)message->payload);
    if(!ob){
        syslog(LOG_WARNING,"Wrong message format for topic: %s",message->topic);
    }else{
        rc =check_for_event(conf->topic,message->topic, (char *)message->payload);
        if(rc != EVENT_FALSE && rc !=EVENT_TRUE){
            syslog(LOG_WARNING,"Event not possible for this topic %s, return code :%d",message->topic, rc);
        }
    }
    return;
}

static int bulk_sub(struct mosquitto *mosq,topic **t_head){

    int rc =0;
    if (!t_head){
        syslog(LOG_ERR,"Error reading config file");
        return CONFIG_READ_ERR;
    }
    topic *t_tmp = *t_head;
    while(t_tmp != NULL){
        rc =mosquitto_subscribe(mosq,NULL,t_tmp->name,0);
        if(rc != MOSQ_ERR_SUCCESS){
            syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            return rc;
        }
        t_tmp = t_tmp->next;
    }
    return rc;
}

/*
    EVENT HANDLING
*/

/*Free the returned string*/
static char* payload_parse(char *payload,char *key){

    char *result;

    struct json_tokener *tok;
    struct json_object *obj;
    struct json_object *val;
    tok =json_tokener_new_ex(JSON_TOKENER_DEFAULT_DEPTH);
    if(!tok){
        return NULL;
    }
    obj =json_tokener_parse_ex(tok,payload,(strlen(payload)+1));
    if(!obj){
        json_tokener_reset(tok);
        return NULL;
    }
    val =json_object_object_get(obj,key);
    if(!val){
        return NULL;
    }
    
    result = strdup(json_object_to_json_string(val));
    int ret = 0;
    ret = json_object_put(obj);
    if(ret != 1){
        syslog(LOG_EMERG,"Failed to free json object memory leak possible");
    }
    json_tokener_free(tok);
    return result;
}


static int __evaluate_event(char* value, char* expected, int operator,int type){   

    int rc =0;
    switch (operator)
    {
    case MORE:
        if (type == 1){
            return INVALID_OP;
        }else{
            if (atoi(value) > atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    case LESS:
        if (type == 1){
            return INVALID_OP;
        }else{
            if (atoi(value) < atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    case EQUAL:
        if (type == 1){

            if (strcmp(value,expected) == 0){
                rc =EVENT_TRUE;
            }
        }else{

            if (atoi(value) == atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    case NEQUAL:

        if (type == 1){

            if (strcmp(value,expected) != 0){
                rc =EVENT_TRUE;
            }
        }else{

            if (atoi(value) != atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    case MORE_EQ:
        if (type == 1){
            return INVALID_OP;
        }else{
            if (atoi(value) >= atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    case LESS_EQ:
        if (type == 1){
            return INVALID_OP;
        }else{
            if (atoi(value) <= atoi(expected)){
                rc =EVENT_TRUE;
            }
        }
        break;
    default:
        rc =INVALID_OP;
        break;
    }
    return rc;
}

static int evaluate_event(char *payload, char* exp, char *key, int operator, int type){

    int rc = 0;
    char *event_val =NULL;
    int value = 0;
    int expected = 0; 
    event_val =payload_parse(payload,key);
    if(!event_val){
        syslog("Failed to parse topic: s value \"%s\"",key);
        return PARSE_ERR;
    }
    __evaluate_event(event_val,exp,operator,type);
    free(event_val);
    return rc;
}

static int event_handler(event *ev_head, char *payload){

    int rc = 0;
    if (!ev_head){
        syslog(LOG_ERR,"Event handler failed to get event data from config err: %d",CONFIG_READ_ERR);
        return CONFIG_READ_ERR;
    }
    event *e_tmp = ev_head;
    while(e_tmp != NULL){


        rc = evaluate_event(payload,ev_head->value,ev_head->key,ev_head->operator,ev_head->type);
        if(rc == EVENT_TRUE){   
            rc =send_mail(e_tmp,payload);
            if(rc != CURLE_OK){
                syslog(LOG_ERR,"%s err:%d",curl_easy_strerror(rc),rc);
                return rc;
            }
        }

        e_tmp = e_tmp->next;
    }
    
}

static int check_for_event(topic *t_head,char *tpc, char *payload){

    int rc =0;

    if (!t_head){
        syslog(LOG_ERR,"Event handler failed to get event data from config err: %d",CONFIG_READ_ERR);
        return CONFIG_READ_ERR;
    }
    topic *t_tmp = t_head;

    while(t_tmp != NULL){

        if(strncmp(t_tmp->name,tpc,(strlen(tpc)+1)) == 0){

            if(t_tmp->event_head != NULL){
                
               rc = event_handler(t_tmp->event_head,payload);
            }
        }
        t_tmp=t_tmp->next;
    }
    return rc;
}

/*
    USABELE FUNCTIONS
*/

struct mosquitto *mqtt_init(config *conf){

    struct mosquitto *mosq;
    int rc = 0;
    /**create new mosquitto instance**/
    rc =mosquitto_lib_init();
        if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        return NULL;
    }
    mosq =mosquitto_new(NULL,true,conf);
        if(!mosq){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        return NULL;
    }

    mosquitto_message_callback_set(mosq,on_message_cb);

    if(conf->credentials == true){
        if (conf->username == NULL && conf->password!= NULL){
            syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            return NULL;
        }
        rc =mosquitto_username_pw_set(mosq, conf->username, conf->password);
        if(rc != MOSQ_ERR_SUCCESS){
            syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            return NULL;
        }
    }
    if (conf->tls == true){
        rc =mosquitto_tls_set(mosq,conf->ca_file,NULL,conf->cert_file,conf->key_file,NULL);
        if(rc != MOSQ_ERR_SUCCESS){
            syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            return NULL;
        }
    }
    return mosq;
}

int mqtt_start(struct mosquitto *mosq, config *conf, topic **head_ref){

    int rc = 0;
    rc =mosquitto_connect(mosq,conf->host,conf->port,conf->keepalive);
    if(rc != MOSQ_ERR_SUCCESS){
    
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        return rc;
    }
    rc =bulk_sub(mosq,head_ref);
    if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        return rc;
    }
    rc =mosquitto_loop_start(mosq);
        if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        return rc;
    }

}
