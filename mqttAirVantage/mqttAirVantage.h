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

#ifndef _MQTT_AV_INTERFACE_H_
#define _MQTT_AV_INTERFACE_H_

typedef int (*incomingMessageHandler)(const char* id, const char* key, const char* value, const char* timestamp);
typedef int (*softwareInstallRequestHandler)(const char* uid, const char* type, const char* revision, const char* url, const char* timestamp);

int mqtt_avStartSession(const char* deviceId, const char* secret, int useTls);
void mqtt_avSetIncomingMsgHandler(incomingMessageHandler pHandler);
void mqtt_avSetSoftwareInstallRequestHandler(softwareInstallRequestHandler pHandler);
int mqtt_avProcessEvent();
int mqtt_avPublishAck(const char* szUid, int nAck, char* szMessage);
int mqtt_avPublishData(const char* szKey, const char* szValue);
int mqtt_avStopSession();

#endif	//_MQTT_AV_INTERFACE_H_