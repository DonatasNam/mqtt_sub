#include <sqlite3.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef SYSLOG_H
#include <syslog.h>
#define SYSLOG_H
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

sqlite3 *db;

static void get_date(char **tim);
static int create_table(sqlite3 *db);

sqlite3* db_init();
int db_add(sqlite3*db, char* topic, char* text);
int db_print();