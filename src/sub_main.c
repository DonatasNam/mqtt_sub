#include "sub_main.h"

void sig_hndlr(int signum){
    stop = 1;
    syslog(LOG_INFO, "system signal recieved");
}

int main(int arc, char **argv){

    struct mosquitto *mosq =NULL;
    struct mosq_conf mconf;
    topic* head_ref = NULL;
    char path[]= "mqtt_sub.mqtt_topics_sct.topic";
    int rc =0;
    mconf.u_topic =NULL;
    mconf.u_payload =NULL;
    mconf.credentials= false;
    mconf.tls=false;
    mconf.host = "test.mosquitto.org";
    mconf.port = 1883;
    mconf.keepalive =60;

    openlog("mqtt_sub",LOG_PID, LOG_USER);

    rc = config_entry_list(&head_ref,path);
    if (rc != UCI_OK){
        goto free_list;
    }
    db= db_init(&rc);
    if (!db || rc != SQLITE_OK){
        goto close_db;
    }

    mosq = mqtt_init(&rc, &mconf);
    if (!mosq || rc != MOSQ_ERR_SUCCESS){
        goto mosquitto_close;
    }

    rc = mqtt_start(mosq, &mconf,&head_ref);
    if (rc != MOSQ_ERR_SUCCESS){
        goto mosquitto_stop;
    }

    while(!stop){
        sleep(1);
    }

    free_list:
    delete_list(head_ref);

    mosquitto_stop:
    mosquitto_loop_stop(mosq,true);
    mosquitto_destroy(mosq);

    mosquitto_close:
    mosquitto_lib_cleanup();

    close_db:
    sqlite3_close(db);

    close_log:
    closelog();

    return rc;
}