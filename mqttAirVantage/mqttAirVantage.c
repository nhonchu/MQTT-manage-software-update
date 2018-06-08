/*******************************************************************************************************************
 
 MQTT AirVantage interface

	This layer provides simple interface to :
		- Start a mqtt session with AirVantage
		- Publish message
		- Receive AirVantage commands along with command parameters
		- Receive Software/Firmware Installation (FOTA/SOTA) Request from AirVantage
		- ACKing the SW installation request

	Communication with AirVantage performed over MQTT prococol, with ot without secured transport : TLS

	Refer to mqttSampleAirVantage.c for how to use this interface

	View of the stack :
	_________________________
	
	 mqttSampleAirVantage.c
	_________________________

	 mqttAirVantage interface  <--- this file
	_________________________

	 mqttInterface interface
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
#include "mqttInterface.h"
#include "mqttAirVantage.h"
#include "swir_json.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>
#include <pthread.h>
#include <dirent.h>



/*---------- Parameters for AirVantage ---------------------------------*/
#define 	URL_AIRVANTAGE_SERVER			"eu.airvantage.net"
#define 	TOPIC_NAME_PUBLISH				"/messages/json"
#define 	TOPIC_NAME_SUBSCRIBE			"/tasks/json"
#define 	TOPIC_NAME_ACK					"/acks/json"

#define		AV_MQTT_KEEP_ALIVE				30
#define		AV_MQTT_QOS						QOS0


mqtt_interface_st*				g_mqttObject = NULL;

incomingMessageHandler			g_pfnUserCommandHandler = NULL;
softwareInstallRequestHandler	g_pfnUserSWInstallHandler = NULL;

//-------------------------------------------------------------------------------------------------------
char* getDeviceId()
{
	if (g_mqttObject)
	{
		return g_mqttObject->deviceId;
	}

	return "";
}
//-------------------------------------------------------------------------------------------------------
int  mqtt_avPublishData(const char* szKey, const char* szValue)
{
	char* 	pTopic = (char *) malloc(strlen(getDeviceId()) + strlen(TOPIC_NAME_PUBLISH) + 1);
	sprintf(pTopic, "%s%s", getDeviceId(), TOPIC_NAME_PUBLISH);

	int rc = mqtt_PublishKeyValue(g_mqttObject, szKey, szValue, pTopic);

	free(pTopic);

	return rc;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_avProcessEvent()
{
	return mqtt_ProcessEvent(g_mqttObject, 1000);
}

//-------------------------------------------------------------------------------------------------------
int mqtt_avPublishAck(const char* szUid, int nAck, char* szMessage)
{
	char* szPayload = (char*) malloc(strlen(szUid)+strlen(szMessage)+48);

	if (nAck == 0)
	{
		sprintf(szPayload, "[{\"uid\": \"%s\", \"status\" : \"OK\"", szUid);
	}
	else
	{
		sprintf(szPayload, "[{\"uid\": \"%s\", \"status\" : \"ERROR\"", szUid);
	}

	if (strlen(szMessage) > 0)
	{
		sprintf(szPayload, "%s, \"message\" : \"%s\"}]", szPayload, szMessage);
	}
	else
	{
		sprintf(szPayload, "%s}]", szPayload);
	}

	printf("Sending ACK: %s\n", szPayload);

	char* 	pTopic = (char *) malloc(strlen(getDeviceId()) + strlen(TOPIC_NAME_ACK) + 1);
	sprintf(pTopic, "%s%s", getDeviceId(), TOPIC_NAME_ACK);

	int rc =  mqtt_PublishData(g_mqttObject, szPayload, strlen(szPayload), pTopic);

	free(pTopic);
	free(szPayload);

	return rc;
}

//-------------------------------------------------------------------------------------------------------
void onIncomingMessage(MessageData* md)
{
	/*
		This is a callback function (handler), invoked by MQTT client whenever there is an incoming message
		It performs the following actions :
		  - deserialize the incoming MQTT JSON-formatted message
		  - call convertDataToCSV()
	*/

	MQTTMessage* message = md->message;
	MQTTString*  topicName = md->topicName;

	int payloadLen = (int)message->payloadlen;

	char* topic = malloc(topicName->lenstring.len + 1);
	memcpy(topic, topicName->lenstring.data, topicName->lenstring.len);
	topic[topicName->lenstring.len] = 0;

	printf("\nIncoming data from topic %s :\n", topic);
	printf("%.*s\n", payloadLen, (char*)message->payload);

	payloadLen++;
	char* szPayload = (char *) malloc(payloadLen);

	memcpy(szPayload, (char*)message->payload, payloadLen);
	szPayload[payloadLen] = 0;

	//decode JSON payload

	char* pszCommand = swirjson_getValue(szPayload, -1, (char *) "command");
	if (pszCommand)
	{
		char*	pszUid = swirjson_getValue(szPayload, -1, (char *) "uid");
		char*	pszTimestamp = swirjson_getValue(szPayload, -1, (char *) "timestamp");
		char*	pszId = swirjson_getValue(pszCommand, -1, (char *) "id");
		char*	pszParam = swirjson_getValue(pszCommand, -1, (char *) "params");

		int     i;
		int		rc = 0;

		#define     AV_JSON_KEY_MAX_COUNT       	10
		#define     AV_JSON_KEY_MAX_LENGTH      	32

		for (i=0; i<AV_JSON_KEY_MAX_COUNT; i++)
		{
			char szKey[AV_JSON_KEY_MAX_LENGTH];

			char * pszValue = swirjson_getValue(pszParam, i, szKey);

			if (pszValue)
			{
				if (g_pfnUserCommandHandler)
				{
					if (g_pfnUserCommandHandler(pszId, szKey, pszValue, pszTimestamp))
					{
						rc = 1;
					}
				}
				else
				{
					fprintf(stdout, "Command[%d] : %s, %s, %s, %s\n", i, pszId, szKey, pszValue, pszTimestamp);	
				}
				free(pszValue);
			}
			else
			{
				break;
			}
		}

		mqtt_avPublishAck(pszUid, rc, (char *) "");

		free(pszCommand);

		if (pszId)
		{
			free(pszId);
		}
		if (pszParam)
		{
			free(pszParam);
		}
		if (pszTimestamp)
		{
			free(pszTimestamp);
		}
		if (pszUid)
		{
			free(pszUid);
		}
	}
	else
	{
		pszCommand = swirjson_getValue(szPayload, -1, (char *) "swinstall");
		if (pszCommand)
		{
			char*	uid = swirjson_getValue(szPayload, -1, (char *) "uid");
			char*	pszTimestamp = swirjson_getValue(szPayload, -1, (char *) "timestamp");
			char*	type = swirjson_getValue(pszCommand, -1, (char *) "type");
			char*	revision = swirjson_getValue(pszCommand, -1, (char *) "revision");
			char*	url = swirjson_getValue(pszCommand, -1, (char *) "url");

			if (g_pfnUserSWInstallHandler)
			{
				g_pfnUserSWInstallHandler(uid, type, revision, url, pszTimestamp);
			}
			else
			{
				fprintf(stdout, "SW install Request : %s, %s, %s, %s, %s\n", uid, type, revision, url, pszTimestamp);	
			}

			free(pszCommand);

			if (uid)
			{
				free(uid);
			}
			if (type)
			{
				free(type);
			}
			if (pszTimestamp)
			{
				free(pszTimestamp);
			}
			if (revision)
			{
				free(revision);
			}
			if (url)
			{
				free(url);
			}
		}
	}

	if (topic)
	{
		free(topic);
	}
	if (szPayload)
	{
		free(szPayload);
	}

	fflush(stdout);
}

//-------------------------------------------------------------------------------------------------------
void mqtt_avSetIncomingMsgHandler(incomingMessageHandler pHandler)
{
	g_pfnUserCommandHandler = pHandler;
}

//-------------------------------------------------------------------------------------------------------
void mqtt_avSetSoftwareInstallRequestHandler(softwareInstallRequestHandler pHandler)
{
	g_pfnUserSWInstallHandler = pHandler;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_avStartSession(const char* deviceId, const char* secret, int useTls)
{
	if (!g_mqttObject)
	{
		g_mqttObject = mqtt_CreateInstance(
								URL_AIRVANTAGE_SERVER,
								useTls > 0 ? 8883 : 1883,
								useTls,
								deviceId,
								secret,
								AV_MQTT_KEEP_ALIVE,
								AV_MQTT_QOS);
	}

	if (SUCCESS == mqtt_StartSession(g_mqttObject))
	{
		char* 	pTopic = (char *) malloc(strlen(deviceId) + strlen(TOPIC_NAME_SUBSCRIBE) + 1);
		sprintf(pTopic, "%s%s", deviceId, TOPIC_NAME_SUBSCRIBE);
		int rc = mqtt_SubscribeTopic(g_mqttObject, pTopic, onIncomingMessage);

		free(pTopic);

		return rc;
	}

	return FAILURE;
}

//-------------------------------------------------------------------------------------------------------
int mqtt_avStopSession()
{
	int rc = mqtt_StopSession(g_mqttObject);

	g_mqttObject = mqtt_DeleteInstance(g_mqttObject);
	return rc;
}

