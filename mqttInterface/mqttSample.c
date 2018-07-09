/*******************************************************************************************************************
 
 MQTT Sample app to interface with MQTT broker

	This sample showcases the following features using mqttInterface :
		- Publish simple message to broker (e.g. iot.eclipse.org)
		- receive message from the subscribed topic

	N. Chu
	June 2018

*******************************************************************************************************************/


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "mqttInterface.h"


#define		AV_MQTT_KEEP_ALIVE				30
#define		AV_MQTT_QOS						QOS0

#define		PUB_INTERVAL					5

mqtt_interface_st*							g_mqttObject = NULL;

//-------------------------------------------------------------------------------------------------------
int 		g_toStop = 0;						//set to 1 in onExit() to exit program


//-------------------------------------------------------------------------------------------------------
void onExit(int sig)
{
	signal(SIGINT, NULL);
	g_toStop = 1;

	fprintf(stdout, "\nterminating app...\n");
	fflush(stdout);
}

//-------------------------------------------------------------------------------------------------------
int OnIncomingMessage(
				const char* topic,
				const char* payload)
{
	fprintf(stdout, "Received message:\n");
	fprintf(stdout, "   Topic: %s\n", topic);
	fprintf(stdout, "   Message: %s\n", payload);

	return 0; //return 0 to ACK positively, 1 to ACK negatively
}



//-------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	if (argc < 8)
	{
		printf("Usage: mqttSample broker port tls(0-1) deviceId password pubTopic subTopic\n");
		printf("    for instance: ./mqttSample iot.eclipse.org 8883 1 device1234 1234 pubTopic subTopic\n");
		return 1;
	}

	signal(SIGINT, onExit);
	signal(SIGTERM, onExit);



	if (!g_mqttObject)
	{
		g_mqttObject = mqtt_CreateInstance(
								argv[1],
								atoi(argv[2]),
								atoi(argv[3]),
								argv[4],
								argv[5],
								AV_MQTT_KEEP_ALIVE,
								AV_MQTT_QOS);
	}

	if (SUCCESS == mqtt_StartSession(g_mqttObject))
	{		
		fprintf(stdout, "MQTT session started OK\n");
	}
	else
	{
		fprintf(stdout, "Failed to start MQTT session\n");
		return 1;
	}

	mqtt_SubscribeTopic(g_mqttObject, argv[7], NULL);

	int count = 0;
	int i = PUB_INTERVAL - 1;
	char	data[16] = {0};

	while (!g_toStop)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		//Must call this on a regular basis in order to process inbound mqtt messages & keep alive
		mqtt_ProcessEvent(g_mqttObject, 1000);

		sleep(1);
		
		i++;

		if (i >= PUB_INTERVAL)
		{
			i = 0;

			sprintf(data, "mqttSample-%d", count);
			//Let's publish data
			mqtt_PublishData(g_mqttObject, data, strlen(data), argv[6]);
		}

		count++;
	}

	//Close the session and quit
	mqtt_StopSession(g_mqttObject);

	return 0;
}