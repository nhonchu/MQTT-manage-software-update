/*******************************************************************************************************************
 
 MQTT Sample app to interface with AirVantage server

	This sample showcases the following features using the mqttAirVantage interface :
		- Publish simple message to AirVantage
		- Receive AirVantage commands along with command parameters
		- Receive Software/Firmware Installation (FOTA/SOTA) Request from AirVantage
			and ACKing the SW installation request

	Communication with AirVantage performed over MQTT prococol, with ot without secured transport : TLS

	Although the Software Package URL is provided by AirVantage, this sample does not perform
	software package download over https and does not handle software installation (platform specific).

	N. Chu
	June 2018

*******************************************************************************************************************/


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "mqttAirVantage.h"


//-------------------------------------------------------------------------------------------------------
int 		g_toStop = 0;						//set to 1 in onExit() to exit program
int			g_ackSWinstall = 0;
char		g_uidSWinstall[64] = {0};

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
				const char* id,
				const char* key,
				const char* value,
				const char* timestamp)
{
	fprintf(stdout, "Received message:\n");
	fprintf(stdout, "   Message id: %s\n", id);
	fprintf(stdout, "   Message timestamp epoch: %s\n", timestamp);
	fprintf(stdout, "   Parameter Name: %s\n", key);
	fprintf(stdout, "   Parameter Value: %s\n", value);

	return 0; //return 0 to ACK positively, 1 to ACK negatively
}

//-------------------------------------------------------------------------------------------------------
int OnSoftwareInstallRequest(
				const char* uid,
				const char* type,
				const char* revision,
				const char* softwarePkgUrl,
				const char* timestamp)
{
	fprintf(stdout, "Received Software Installation request from AirVanage:\n");
	fprintf(stdout, "   Operation uid: %s\n", uid);
	fprintf(stdout, "   Timestamp epoch: %s\n", timestamp);
	fprintf(stdout, "   SW pkg type: %s\n", type);
	fprintf(stdout, "   SW revision: %s\n", revision);
	fprintf(stdout, "   SW pkg download Url: %s\n", softwarePkgUrl);

	/*
		Your device should be :
		- downloading the software/firmware package with the provide url (not done in this sample)
		- authenticating the issuer of package, checking package integrity (not done here)
		- installing the software/firmware (not done here)
		- acking the SW installation operation to AirVantage (done below with a delay) */
	//Let's delay this ACK
	fprintf(stdout, "\nwill be ACKing this request in few seconds...\n");
	g_ackSWinstall = 5;
	strcpy(g_uidSWinstall, uid);

	return 0; //return 0 to ACK positively, 1 to ACK negatively
}


//-------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Usage: mqttSampleAirVantage serial password [tls]\n");
		return 1;
	}

	signal(SIGINT, onExit);
	signal(SIGTERM, onExit);

	//Set handler for AirVantage incoming message/command/SW-install
	mqtt_avSetIncomingMsgHandler(OnIncomingMessage);
	mqtt_avSetSoftwareInstallRequestHandler(OnSoftwareInstallRequest);

	int useTls = 0;

	if (argc > 3)
	{
		if (strcasecmp("tls", argv[3]) == 0)
		{
			useTls = 1;	//use TLS if specified in 3rd argument
		}
	}

	//Start MQTT session with AirVantage using the specified arguments
	if (mqtt_avStartSession(argv[1], argv[2], useTls))
	{
		fprintf(stdout, "Failed to start MQTT session on AirVantage server\n");
		return 1;
	}

	int count = 0;
	int i = 0;
	char	data[16] = {0};

	while (!g_toStop)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		//Must call this on a regular basis in order to process inbound mqtt messages & keep alive
		mqtt_avProcessEvent();

		sleep(1);

		count++;
		i++;

		if (i >= 5)
		{
			i = 0;

			sprintf(data, "%d", count);
			//Let's publish data
			mqtt_avPublishData("counter", data);
		}

		if (g_ackSWinstall > 0)
		{
			//If there is a SW install request, ack the operation here after some delay
			//the delay intends to simulate the SW download and install processes
			//This ACKing should be performed after installation procedure
			g_ackSWinstall--;
			if (0 == g_ackSWinstall)
			{
				//ACK the SW install request
				fprintf(stdout, "\nNow, ACKing the pending SW Install request\n");
				mqtt_avPublishAck(g_uidSWinstall, 0, "install success");
			}
		}
	}

	//Close the session and quit
	mqtt_avStopSession();

	return 0;
}