#include "sub_mqtt.h"


void on_connect_cb(struct mosquitto *mosq, void *obj, int result){
    return;
}


void on_message_cb(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{   
    int rc = 0;
    rc =db_add(db,message->topic,(char*)message->payload);
    printf("%s, err: %d", sqlite3_errstr(rc), rc);
    return;
}

/*
int on_tls_cb(char *buf, int size, int rwflag, void *userdata){
    int rc =0;
    return rc;
}
*/
int bulk_sub(struct mosquitto *mosq,topic **head_ref){

    int rc =0;
    if (!head_ref){
        syslog(LOG_ERR,"recieved a NULL pointer");
        return -1;
    }
    topic *tmp = *head_ref;
    while(tmp != NULL){
        rc =mosquitto_subscribe(mosq,NULL,tmp->name,0);
        if(rc != MOSQ_ERR_SUCCESS){
            syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            return rc;
        }
        tmp = tmp->next;
    }
    return rc;
}

struct mosquitto *mqtt_init(int *err, struct mosq_conf*mconf){

    struct mosquitto *mosq;
    int rc = 0;
    /**create new mosquitto instance**/
    rc =mosquitto_lib_init();
        if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        *err= rc;
        return mosq;
    }
    mosq =mosquitto_new(NULL,true, mconf);

        if(!mosq){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        *err= errno;
        return mosq;
    }

    
    mosquitto_connect_callback_set(mosq,on_connect_cb);
    mosquitto_message_callback_set(mosq,on_message_cb);


    if(mconf->credentials == true){
        if (mconf->username == NULL && mconf->password!= NULL){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            *err= -1;
            return mosq;
        }
        rc =mosquitto_username_pw_set(mosq, mconf->username, mconf->password);
        if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
            *err= rc;
            return mosq;
        }
    }
    if (mconf->tls == true){
        rc =mosquitto_tls_set(mosq,mconf->cafile,mconf->capath,mconf->cerfile,mconf->keyfile,NULL);
        if(rc != MOSQ_ERR_SUCCESS){
        syslog(LOG_ERR,"%s err: %d\n",mosquitto_strerror(rc),rc);
        *err= rc;
        return mosq;
        }
    }
    
    return mosq;
}

int mqtt_start(struct mosquitto *mosq, struct mosq_conf *mconf, topic **head_ref){

    int rc = 0;
    rc =mosquitto_connect(mosq,mconf->host,mconf->port,mconf->keepalive);
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