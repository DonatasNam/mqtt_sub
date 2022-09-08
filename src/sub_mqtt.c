#include "sub_mqtt.h"

/*
    MQTT CONNECTION AND MESSAGES
*/
void on_message_cb(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{   
    int rc = 0;
    config *conf;
    conf = (config *)obj;
    rc =check_for_event(conf->topic,message->topic, (char *)message->payload);
    if(rc != EVENT_FALSE){
        syslog(LOG_WARNING,"Event not possible for this topic, return code :%d", rc);
    }
    if(rc !=EVENT_TRUE){
        syslog(LOG_WARNING,"Event not possible for this topic, return code :%d", rc);
    }
    rc =db_add(db,message->topic,(char*)message->payload);
    if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
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
        return NULL;
    }
    val =json_object_object_get(obj,key);
    if(!val){
        return NULL;
    }

    result = strdup(json_object_to_json_string(val));
    
    int ret = 0;
    ret = json_object_put(obj);
    json_tokener_free(tok);
    return result;
}


static int evaluate_event_int(int value, int expected, int operator){   

    int rc =0;
    switch (operator)
    {
    case MORE:
       if (value > expected){
            rc =EVENT_TRUE;
        }
        break;
    case LESS:
        if (value < expected){
            rc = EVENT_TRUE;
        }
        break;
    case EQUAL:
        if (value == expected){
            rc = EVENT_TRUE;
        }
        break;
    case NEQUAL:
        if (value != expected){
            rc =EVENT_TRUE;    
        }
        break;
    case MORE_EQ:
        if (value >= expected){
            rc =EVENT_TRUE;
        }
        break;
    case LESS_EQ:
        if (value <= expected){
            rc =EVENT_TRUE;
        }
        break;
    default:
        rc =INVALID_OP;
        break;
    }
    return rc;
}

static int evaluate_event_string(char* value, char *expected, int operator){  

    int rc =0;
    switch (operator)
    {
    case EQUAL:
        if (strncmp(value,expected,strlen(expected)) == 0){
            rc =EVENT_TRUE;
        }
        break;
    case NEQUAL:
        if (strncmp(value,expected,strlen(expected)) != 0){
            rc =EVENT_TRUE;     
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
        return PARSE_ERR;
    }

    switch(type)
    {
    case 0:
        value =atoi(event_val);
        expected =atoi(exp);
        rc = evaluate_event_int(value,expected,operator);
        break;
    case 1:
        rc =evaluate_event_string(event_val,exp,operator);
        break;
    default:
        rc = INVALID_TYPE;
        break;
    }
    free(event_val);
    return rc;
}

static int event_handler(event *ev_head, char *payload){

    int rc = 0;
    if (!ev_head){
        return CONFIG_READ_ERR;
    }
    event *e_tmp = ev_head;
    while(e_tmp != NULL){


        rc = evaluate_event(payload,ev_head->value,ev_head->key,ev_head->operator,ev_head->type);
        if(rc == EVENT_TRUE){   
            //send event
        }

        e_tmp = e_tmp->next;
    }
    
}

static int check_for_event(topic *t_head,char *tpc, char *payload){

    int rc =0;

    if (!t_head){
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
        rc =mosquitto_tls_set(mosq,conf->cafile,conf->capath,conf->cerfile,conf->keyfile,NULL);
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
