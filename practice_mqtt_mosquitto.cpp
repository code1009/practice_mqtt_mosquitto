#include <iostream>
#include <cstdio>
#include <cstring>

#include <mosquittopp.h>
#include <Windows.h>

#if 0

### setup
https ://mosquitto.org/files/binary/win64/
mosquitto - 2.0.14  - install - windows - x64.exe : websocket 정상 동작
mosquitto - 2.0.18a - install - windows - x64.exe : websocket 정상 동작
mosquitto - 2.0.19  - install - windows - x64.exe : websocket 정상 동작 안함 "error: websockets support not available."
mosquitto - 2.0.20  - install - windows - x64.exe : websocket 정상 동작 안함 "error: websockets support not available."



### include dir
C : \Program Files\mosquitto\devel



### mosquitto.conf
```
# 일반 MQTT 리스너(기본 포트 1883)
listener 1883
protocol mqtt

# WebSocket 리스너(포트 9001)
listener 9001
protocol websockets

allow_anonymous true

```


### 실행
C:\Program Files\mosquitto>mosquitto.exe -c mosquitto.conf -v


#endif

#pragma comment(lib, "C:/Program Files/mosquitto/devel/mosquitto.lib")
#pragma comment(lib, "C:/Program Files/mosquitto/devel/mosquittopp.lib")

class mqtt_tempconv : public mosqpp::mosquittopp
{
public:
	mqtt_tempconv(const char* id, const char* host, int port);
	virtual ~mqtt_tempconv();

	virtual void on_connect(int rc) override;
	virtual void on_message(const struct mosquitto_message* message) override;
	virtual void on_subscribe(int mid, int qos_count, const int* granted_qos) override;

public:
	void publish_my_message(void);
};

mqtt_tempconv::mqtt_tempconv(const char* id, const char* host, int port) : 
	mosquittopp(id)
{
	int keepalive = 5; // 최소 5로 해야함

	/* Connect immediately. This could also be done by calling
	 * mqtt_tempconv->connect(). */
	connect(host, port, keepalive);
};

mqtt_tempconv::~mqtt_tempconv()
{
}

void mqtt_tempconv::on_connect(int rc)
{
	printf("[%08x] Connected with code %d.\n", GetCurrentThreadId(), rc);
	
	if (rc == 0) 
	{
		/* Only attempt to subscribe on a successful connect. */
		subscribe(NULL, "temperature/celsius");
	}
}

void mqtt_tempconv::on_message(const struct mosquitto_message* message)
{
	printf("[%08x] on_message() %s : %u bytes\r\n", GetCurrentThreadId(), message->topic, message->payloadlen);


	double temp_celsius, temp_fahrenheit;
	char buf[51];

	if (!strcmp(message->topic, "temperature/celsius")) 
	{
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
	printf("[%08x] Subscription succeeded.\n", GetCurrentThreadId());
}

void mqtt_tempconv::publish_my_message(void)
{
	char buf[51];
	static int x = 0;


	snprintf(buf, 50, "code1009-%u", x++);


	publish(NULL, "temperature/fahrenheit", strlen(buf), buf);

	Sleep(1000);
}

int main()
{
	class mqtt_tempconv* tempconv;
	int rc;

	mosqpp::lib_init();

	tempconv = new mqtt_tempconv("tempconv", "127.0.0.1", 1883);

	//tempconv->loop_forever();
	while (1)
	{
		rc= tempconv->loop(-1, 1);
		if (rc)
		{
			Sleep(1000);
			tempconv->reconnect();
		}
		tempconv->publish_my_message();
		printf("[%08x] loop\n", GetCurrentThreadId());
	}

	mosqpp::lib_cleanup();


	return 0;
}
