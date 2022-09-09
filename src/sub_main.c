#include "sub_main.h"

void sig_hndlr(int signum){
    stop = 1;
    syslog(LOG_INFO, "system signal recieved");
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    config *conf = state->input;
    switch (key)
    {
    case 'h':
        conf->host= arg;
        break;
    case 'p':
        conf->port= atoi(arg);
        break;
    case 'k':
        conf->keepalive= atoi(arg);
        break;
    case 'U':
        conf->credentials = CREDENTIALS_ON;
        conf->username= arg;
        break;
    case 'P':
        conf->credentials = CREDENTIALS_ON;
        conf->password= arg;
        break;
    case 't':
        conf->tls =TLS_ON;
        conf->ca_path= arg;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 3)
            argp_usage(state);
            conf->args[state->arg_num] = arg;
        break;
    case ARGP_KEY_END:
        if ((!conf->host) ||(!conf->port) ||(!conf->keepalive)){
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char **argv){

    struct mosquitto *mosq =NULL;
    config conf;
    conf.topic =NULL;
    topic* t_head = NULL;
    event* e_head = NULL;
    char *config_name= "mqtt_sub";
    int rc =0;

    openlog("mqtt_sub",LOG_PID, LOG_USER);
    
    rc =argp_parse(&argp, argc, argv, 0, 0, &conf);
    rc = load_uci_config(&t_head,&e_head,config_name);
    conf.topic=t_head;
    if (rc != UCI_OK){
        goto free_list;
    }

    db= db_init();
    if (!db){
        goto close_db;
    }

    mosq = mqtt_init(&conf);
    if (!mosq){
        goto mosquitto_close;
    }

    rc = mqtt_start(mosq, &conf,&t_head);
    if (rc != MOSQ_ERR_SUCCESS){
        goto mosquitto_stop;
    }
    while(!stop){
        sleep(5);
    }

    free_list:
    delete_full_list(t_head);

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