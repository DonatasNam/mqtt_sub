#include "sub_db.h"


/**time utility**/
static void get_date(char **tim){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    *tim = (char*)malloc(20);
    sprintf(*tim,"%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return;
}

static int create_table(sqlite3 *db){

    char *errmsg;
    const char *sql ="CREATE TABLE IF NOT EXISTS MQTT_SUB_LOG(ID INTEGER PRIMARY KEY AUTOINCREMENT,DATE TEXT,TOPIC TEXT,PAYLOAD TEXT);";
    int rc =0;
    rc= sqlite3_exec(db, sql,0,0,&errmsg);
    if (rc != SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    return rc;
}


/**returns pointer to sqlite3 struct int err records any err codes**/
sqlite3* db_init(){

    int rc = 0;
    rc= sqlite3_open("/log/mqtt_sub_log.db", &db);
    if (rc != SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return NULL;
    }
    rc = create_table(db);
    if (rc != SQLITE_OK){
        return NULL;
    }
    return db;
}
int db_print(){
    sqlite3_stmt *stmt = NULL;
    int rc = 0;
    char *sql ="SELECT * FROM MQTT_SUB_LOG";
    rc= sqlite3_prepare(db,sql,-1, &stmt,NULL);
    do
    {
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW){
            syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
            return rc;
        }
        printf("step return integer %d\n",rc);
        int id = sqlite3_column_int(stmt,1);
        const unsigned char *time = sqlite3_column_text(stmt,2);
        const unsigned char *topic= sqlite3_column_text(stmt,3);
        const unsigned char *payload= sqlite3_column_text(stmt,4);
        printf("ID: %d, Time: %s, Topic: %s,\n Payload: %s\n", id,time,topic,payload);
    } while (rc != SQLITE_DONE);
    rc =sqlite3_reset(stmt);
    if (rc != SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    } 
    printf("reset return integer %d\n",rc);
    rc =sqlite3_finalize(stmt);
        if (rc != SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    } 
    printf("finalize return integer %d\n",rc);
    return rc;
}

int db_add(sqlite3 *db, char* topic, char* payload){

    sqlite3_stmt * stmt =NULL;
    int rc =0;
    char *sql ="INSERT INTO MQTT_SUB_LOG (ID,DATE,TOPIC,PAYLOAD) " "VALUES (NULL,?,?,?); ";
    
    char *date;
    get_date(&date);

    rc =sqlite3_prepare_v2(db,sql,-1, &stmt,NULL);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }

    /**add**/
    rc = sqlite3_bind_text(stmt,1,date,strlen(date),NULL);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    rc = sqlite3_bind_text(stmt,2,topic,strlen(topic),NULL);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    rc = sqlite3_bind_text(stmt,3,payload,strlen(payload),NULL);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }


    /**prepare**/
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    rc =sqlite3_clear_bindings( stmt );
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }   
    rc =sqlite3_reset(stmt);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    rc =sqlite3_finalize(stmt);
    if (rc !=SQLITE_OK){
        syslog(LOG_ERR,"%s, err: %d", sqlite3_errstr(rc), rc);
        return rc;
    }
    free(date);
    return rc;
}
