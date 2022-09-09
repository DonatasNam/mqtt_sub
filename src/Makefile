SRC:= sub_main.c sub_db.c sub_mqtt.c config_get.c email.c

LDFLAGS:= -lmosquitto -lsqlite3 -luci -ljson-c -largp -lcurl
CFLAGS:= -o
.PHONY:all clean

all:mqtt_sub

mqtt_sub: $(SRC)
	$(CC) $(CFLAGS) $@ $^ $(LDFLAGS)

clean:
	rm -rf mqtt_sub