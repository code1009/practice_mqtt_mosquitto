#include <iostream>
#include <cstdio>
#include <cstring>

#include <mosquittopp.h>

#if 0

# setup
mosquitto-2.0.18a-install-windows-x64.exe
 
# include dir
C:\Program Files\mosquitto\devel

# mosquitto.conf
```
# 일반 MQTT 리스너(기본 포트 1883)
listener 1883
protocol mqtt

# WebSocket 리스너(포트 9001)
listener 9001
protocol websockets

allow_anonymous true

```


#endif

#pragma comment(lib, "C:/Program Files/mosquitto/devel/mosquitto.lib")
#pragma comment(lib, "C:/Program Files/mosquitto/devel/mosquittopp.lib")

class mqtt_tempconv : public mosqpp::mosquittopp
{
public:
	mqtt_tempconv(const char* id, const char* host, int port);
	~mqtt_tempconv();

	void on_connect(int rc);
	void on_message(const struct mosquitto_message* message);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);
};
mqtt_tempconv::mqtt_tempconv(const char* id, const char* host, int port) : mosquittopp(id)
{
	int keepalive = 10;

	/* Connect immediately. This could also be done by calling
	 * mqtt_tempconv->connect(). */
	connect(host, port, keepalive);
};

mqtt_tempconv::~mqtt_tempconv()
{
}

void mqtt_tempconv::on_connect(int rc)
{
	printf("Connected with code %d.\n", rc);
	if (rc == 0) {
		/* Only attempt to subscribe on a successful connect. */
		subscribe(NULL, "temperature/celsius");
	}
}

void mqtt_tempconv::on_message(const struct mosquitto_message* message)
{
	double temp_celsius, temp_fahrenheit;
	char buf[51];

	if (!strcmp(message->topic, "temperature/celsius")) {
		memset(buf, 0, 51 * sizeof(char));
		/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, 50 * sizeof(char));
		temp_celsius = atof(buf);
		temp_fahrenheit = temp_celsius * 9.0 / 5.0 + 32.0;
		snprintf(buf, 50, "code1009%f", temp_fahrenheit);
		publish(NULL, "temperature/fahrenheit", strlen(buf), buf);
	}
}

void mqtt_tempconv::on_subscribe(int mid, int qos_count, const int* granted_qos)
{
	printf("Subscription succeeded.\n");
}

int main()
{
	class mqtt_tempconv* tempconv;
	int rc;

	mosqpp::lib_init();

	tempconv = new mqtt_tempconv("tempconv", "127.0.0.1", 1883);
	tempconv->loop_forever();

	mosqpp::lib_cleanup();


	return 0;
}
