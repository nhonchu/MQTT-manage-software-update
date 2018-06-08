/*******************************************************************************************************************
 
 MQTT interface

	This layer encapsules the MQTT Paho client and exposes generic mqtt functions

	Communication with MQTT broker can performed with or without secured transport : TLS


	View of the stack :
	_________________________
	
	 mqttSampleAirVantage.c
	_________________________

	 mqttAirVantage interface  
	_________________________

	 mqttInterface interface  <--- this file
	_________________________

	 paho
	_________________________

	 TLSinterface
	_________________________

	 Mbed TLS
	_________________________


	N. Chu
	June 2018

*******************************************************************************************************************/

#ifndef _MQTT_INTERFACE_H_
#define _MQTT_INTERFACE_H_


#define MQTT_BROKER		"MqttBrokerUrl"
#define	MQTT_PORT		"MqttBrokerPort"
#define MQTT_ENDPOINT	"MqttEndPointName"
#define MQTT_SECRET		"MqttSecret"
#define MQTT_KEEPALIVE	"MqttKeepAlive"
#define MQTT_QOS		"MqttQoS"

#define 	MAX_PAYLOAD_SIZE			2048	//Default payload buffer size

typedef struct {
	char			deviceId[32];
	char			serverUrl[256];
	int				serverPort;
	int				useTLS;
	char			secret[32];
	int				keepAlive;
	int				qoS;

	Network 		network;
	Client 			mqttClient;
	unsigned char	mqttBuffer[MAX_PAYLOAD_SIZE];
	unsigned char	mqttReadBuffer[MAX_PAYLOAD_SIZE];
} mqtt_interface_st;

mqtt_interface_st * mqtt_CreateInstance(
								const char* brokerUrl,
								int brokerPort,
								int useTLS, 
								const char* deviceId,
								const char* secret,
								int keepAlive,
								int qos);
mqtt_interface_st* mqtt_DeleteInstance(mqtt_interface_st* mqttObject);
int mqtt_SetConfig(mqtt_interface_st * mqttObject, const char* configName, const char * value);
int mqtt_GetConfig(mqtt_interface_st * mqttObject, const char* configName, char * value, size_t valueLen);

int mqtt_StartSession(mqtt_interface_st * mqttObject);
int mqtt_StopSession(mqtt_interface_st * mqttObject);

int mqtt_SubscribeTopic(mqtt_interface_st * mqttObject, const char* topicName, messageHandler msgHandler);
int mqtt_UnscribeTopic(mqtt_interface_st * mqttObject, const char* topicName);

int mqtt_ProcessEvent(mqtt_interface_st * mqttObject, unsigned waitDelayMs);

int  mqtt_PublishKeyValue(mqtt_interface_st * mqttObject, const char* szKey, const char* szValue, const char* topicName);
int  mqtt_PublishData(mqtt_interface_st * mqttObject, const char* data, size_t dataLen, const char* topicName);


#endif	//_MQTT_INTERFACE_H_
