SRC:= sub_main.c sub_db.c sub_mqtt.c config_get.c

LDFLAGS:= -lmosquitto -lsqlite3 -luci
.PHONY:all clean

all:mqtt_sub

mqtt_sub: $(SRC)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf mqtt_sub