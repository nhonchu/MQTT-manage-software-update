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

#include <stdio.h>
#include "MQTTClient.h"
#include <memory.h>
#include "mqttInterface.h"

/*---------- Default parameters ---------------------------------*/
#define 	TIMEOUT_MS					5000	//second time-out, MQTT client init
#define 	MQTT_VERSION				3

#define		DEFAULT_PORT				1883
#define		DEFAULT_KEEP_ALIVE			30
#define		DEFAULT_QOS					QOS0



//-------------------------------------------------------------------------------------------------------
mqtt_interface_st * mqtt_CreateInstance(const char* brokerUrl, int brokerPort, int useTLS, const char* deviceId, const char* secret, int keepAlive, int qos)
{
	mqtt_interface_st * mqttObject = (mqtt_interface_st *) malloc(sizeof(mqtt_interface_st));

	memset(mqttObject, 0, sizeof(mqtt_interface_st));

	strcpy(mqttObject->deviceId, deviceId);
	strcpy(mqttObject->serverUrl, brokerUrl);
	if (brokerPort <= 0)
	{
		mqttObject->serverPort = DEFAULT_PORT;
	}
	else
	{
		mqttObject->serverPort = brokerPort;
	}
	mqttObject->useTLS = useTLS;
	strcpy(mqttObject->secret, secret);
	if (keepAlive <= 0)
	{
		mqttObject->keepAlive = DEFAULT_KEEP_ALIVE;	
	}
	else
	{
		mqttObject->keepAlive = keepAlive;
	}
	if (qos <= 0 || qos > QOS2)
	{
		mqttObject->qoS = DEFAULT_QOS;	
	}
	else
	{
		mqttObject->qoS = qos;
	}

	return mqttObject;
}
//-------------------------------------------------------------------------------------------------------
mqtt_interface_st* mqtt_DeleteInstance(mqtt_interface_st* mqttObject)
{
	if (mqttObject)
	{
		free(mqttObject);
	}

	return NULL;
}
//-------------------------------------------------------------------------------------------------------
int mqtt_PublishData(mqtt_interface_st * mqttObject, const char* data, size_t dataLen, const char* topicName)
{

	//printf("Sending Data: %s\n", data);

	MQTTMessage		msg;
	msg.qos = mqttObject->qoS;
	msg.retained = 0;
	msg.dup = 0;
	msg.id = 0;
	msg.payload = (void *) data;
	msg.payloadlen = dataLen;

	printf("Publishing data on %s : %s ... ", topicName, data);
	fflush(stdout);

	int rc = MQTTPublish(&mqttObject->mqttClient, topicName, &msg);
	if (rc != SUCCESS)
	{
		printf("publish error: %d\n", rc);
		
	}
	else
	{
		printf("OK\n");
	}
	fflush(stdout);

	return rc;
}

//-------------------------------------------------------------------------------------------------------
int  mqtt_PublishKeyValue(mqtt_interface_st * mqttObject, const char* szKey, const char* szValue, const char* topicName)
{
	char * message = malloc(strlen(szKey) + strlen(szValue) + 10);

	sprintf(message, "{\"%s\":\"%s\"}", szKey, szValue);	

	int rc = mqtt_PublishData(mqttObject, message, strlen(message), topicName);

	free(message);

	return rc;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_SetConfig(mqtt_interface_st * mqttObject, const char* configName, const char * value)
{
	if (configName == NULL) return -1;
	if (value == NULL) return -1;

	if (!strlen(configName)) return -1;

	int ret = 0;

	if (strcasecmp(MQTT_BROKER, configName) == 0)
	{
		strcpy(mqttObject->serverUrl, value);
	}
	else if (strcasecmp(MQTT_PORT, configName) == 0)
	{
		mqttObject->serverPort = atoi(value);
	}
	else if (strcasecmp(MQTT_ENDPOINT, configName) == 0)
	{
		strcpy(mqttObject->deviceId, value);
	}
	else if (strcasecmp(MQTT_SECRET, configName) == 0)
	{
		strcpy(mqttObject->secret, value);
	}
	else if (strcasecmp(MQTT_KEEPALIVE, configName) == 0)
	{
		int val = atoi(value);
		if (val > 0)
		{
			mqttObject->keepAlive = val;
		}
		else
		{
			ret = 1;
		}
	}
	else if (strcasecmp(MQTT_QOS, configName) == 0)
	{
		mqttObject->qoS = atoi(value);
	}

	return ret;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_GetConfig(mqtt_interface_st * mqttObject, const char* configName, char * value, size_t valueLen)
{
	if (configName == NULL) return -1;
	if (value == NULL) return -1;

	if (!strlen(configName)) return -1;

	int ret = 0;

	if (strcasecmp(MQTT_BROKER, configName))
	{
		strncpy(value, mqttObject->serverUrl, valueLen);
	}
	else if (strcasecmp(MQTT_PORT, configName))
	{
		sprintf(value, "%d", mqttObject->serverPort);
	}
	else if (strcasecmp(MQTT_ENDPOINT, configName))
	{
		strncpy(value, mqttObject->deviceId, valueLen);
	}
	else if (strcasecmp(MQTT_SECRET, configName))
	{
		strncpy(value, mqttObject->secret, valueLen);
	}
	else if (strcasecmp(MQTT_KEEPALIVE, configName))
	{
		sprintf(value, "%d", mqttObject->keepAlive);
	}
	else if (strcasecmp(MQTT_QOS, configName))
	{
		sprintf(value, "%d", mqttObject->qoS);
	}
	else
	{
		ret = -1;
		value[0] = 0;
	}

	return ret;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_ProcessEvent(mqtt_interface_st * mqttObject, unsigned waitDelayMs)
{
	return MQTTYield(&mqttObject->mqttClient, 1000);
}

//-------------------------------------------------------------------------------------------------------
int mqtt_StartSession(mqtt_interface_st * mqttObject)
{
	int 			rc = 0;
	
	int				nMaxRetry = 3;
	int				nRetry = 0;


	for (nRetry=0; nRetry<nMaxRetry; nRetry++)
	{
		linux_disconnect(&mqttObject->network);
		NewNetwork(&mqttObject->network);
		mqttObject->network.connect(&mqttObject->network, mqttObject->serverUrl, mqttObject->serverPort, mqttObject->useTLS);

		MQTTClient(&mqttObject->mqttClient, &mqttObject->network, TIMEOUT_MS, mqttObject->mqttBuffer, sizeof(mqttObject->mqttBuffer), mqttObject->mqttReadBuffer, sizeof(mqttObject->mqttReadBuffer));
	 
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
		data.willFlag = 0;
		data.MQTTVersion = MQTT_VERSION;
		data.clientID.cstring = mqttObject->deviceId;
		data.username.cstring = mqttObject->deviceId;
		data.password.cstring = mqttObject->secret;

		data.keepAliveInterval = mqttObject->keepAlive;
		data.cleansession = 1;
		printf("Attempting (%d/%d) to connect to tcp://%s:%d... ", nRetry+1, nMaxRetry, mqttObject->serverUrl, mqttObject->serverPort);

		fflush(stdout);
	
		rc = MQTTConnect(&mqttObject->mqttClient, &data);
		//printf("Connected %d\n", rc);
		printf("%s\n", rc == SUCCESS ? "OK" : "Failed");
	    fflush(stdout);

		if (rc == SUCCESS) 
		{
			//connected			
			break;
		}
		else
		{
			MQTTDisconnect(&mqttObject->mqttClient);
			mqttObject->network.disconnect(&mqttObject->network);
		}
	}

	if (rc != SUCCESS)
	{
		printf("Failed to connect to AirVantage server\n");
		fflush(stdout);
	}

	return rc;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_SubscribeTopic(mqtt_interface_st * mqttObject, const char* topicName, messageHandler msgHandler)
{
	printf("Subscribing to topic %s... ", topicName);
	int rc = MQTTSubscribe(&mqttObject->mqttClient, topicName, mqttObject->qoS, msgHandler);
	printf("%s\n", rc == 0 ? "OK" : "Failed");
	//printf("Subscribed %d\n", rc);
	fflush(stdout);

	return rc;
}


//-------------------------------------------------------------------------------------------------------
int mqtt_UnscribeTopic(mqtt_interface_st * mqttObject, const char* topicName)
{
	printf("Unsubscribing to %s\n", topicName);
	int rc = MQTTUnsubscribe(&mqttObject->mqttClient, topicName);
	printf("Unsubscribed %d\n", rc);
	fflush(stdout);

	return rc;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_StopSession(mqtt_interface_st * mqttObject)
{
	int rc = MQTTDisconnect(&mqttObject->mqttClient);

	mqttObject->network.disconnect(&mqttObject->network);

	return rc;
}

