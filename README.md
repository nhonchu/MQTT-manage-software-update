Software/Firmware Update with AirVantage over MQTT protocol
===========================================================

This work enables your device to easily interface with AirVantage to :

- Publish data
- Receive commands along with parameters
- Receive software/firmware installation request and to report installation status

Data exchange over MQTT can be secured with TLS (using Mbed TLS v2.9.0, Apache 2.0 licensed version).

Refer to the sample code, mqttSampleAirVantage.c, for how to use the mqttAirVantage interface.

The software stack is follow:

- mqttSampleAirVantage.c : sample code, publish data and software installation management
- mqttAirVantage : implementing AirVantage-specific MQTT topics and ack mechanism
- mqttInterface : encapsulates Paho MQTTClient, exposing generic mqtt key functions
- tlsInterface  : manage underlying socket (non-secured and TLS) and interface with Mbed TLS
- Mbed TLS : TLS features

This code has been tested under Ubuntu 16.04

Build the application
---------------------
~~~
make

~~~


Create a system in AirVantage
-----------------------------------------

- If you need an AirVantage trial account, [check here](https://eu.airvantage.net/accounts/signup?type=AVEP)
- Save the following sample app model with .app extension, zip it and [release the zip to AirVantage](https://doc.airvantage.net/av/reference/develop/howtos/releaseApplication/), this will release a new MQTT application on AirVantage
- [Create a system in AirVantage](https://doc.airvantage.net/av/reference/inventory/howtos/createSystem/), select the application you've released in the Applications field
- Assign a password to your MQTT application
- [Activate your system](https://doc.airvantage.net/av/reference/inventory/howtos/activateSystem/)
- Go to the Monitor view

~~~
<?xml version="1.0" encoding="ISO-8859-1"?><app:application xmlns:app="http://www.sierrawireless.com/airvantage/application/1.0" type="com.sample.device" name="mqtt-sample" revision="1.0">
    <capabilities>
    
        <communication>
            <protocol comm-id="IMEI" type="MQTT"/>
        </communication>
     
        <dm>
            <action impl="MQTT_SW_INSTALL"/>
        </dm>
                
        <data>
            <encoding type="MQTT">
                <asset default-label="Device" id="device">
                    <setting default-label="Threshold" path="counter" type="int"/>
                    
                    <command path="TurnOn" default-label="Turn on">
                        <parameter id="Light" default-label="Light" type="boolean"/>
                    </command>

                    <command path="Message" default-label="Message">
                            <parameter id="msg" default-label="msg" type="string"/>
                    </command>

                </asset>
            </encoding>
        </data>  

    </capabilities>

    <application-manager use="MQTT_SW"/>
</app:application>
~~~



Start Application
-----------------

~~~
./mqttSampleAirVantage <IMEI> <password> [tls]
~~~

The first and second argument shall match the IMEI and password you've provided in the previous step. The third argument (tls) is optional, if provided TLS will be used to secured the communication


View published data in AirVantage
---------------------------------

The sample app automatically publish data points ("counter") to AirVantage, they are visible in the Timeline widget 


Send commands to device
-----------------------

To send a command to your device, in AirVantage portal, go to the *More* menu then *Custom Command*, select your MQTT application.
You should be seeing the command arriving in the device sample application.


Create a new Software/Firmware package
--------------------------------------

- Modify and save the below sample app model to file as .app extension. It represents a new MQTT application along with a new software or firmware package for your device.
- Zip this app file along with the specfied software/firmware binary file
- Release this new application to AirVantage

~~~
<?xml version="1.0" encoding="ISO-8859-1"?><app:application xmlns:app="http://www.sierrawireless.com/airvantage/application/1.0" type="com.sample.device" name="mqtt-sample" revision="2.0">
    <capabilities>
    
        <communication>
            <protocol comm-id="IMEI" type="MQTT"/>
        </communication>
     
        <dm>
            <action impl="MQTT_SW_INSTALL"/>
        </dm>
                
        <data>
            <encoding type="MQTT">
                <asset default-label="Device" id="device">
                    <variable default-label="Temperature" path="temperature" type="double"/>
                    <variable default-label="Humidity" path="humidity" type="double"/>
                    <variable default-label="Luminosity" path="luminosity" type="double"/>
                    <setting default-label="Light" path="light" type="double"/>
                    
                    <command path="TurnOn" default-label="Turn on">
                        <parameter id="Light" default-label="Light" type="boolean"/>
                    </command>

                    <command path="Message" default-label="Message">
                            <parameter id="msg" default-label="msg" type="string"/>
                    </command>

                </asset>
            </encoding>
        </data>  

    </capabilities>

    <application-manager use="MQTT_SW"/>

    <binaries>
        <binary file="softwarePackage.bin" revision="2.0" type="FULL"/>
    </binaries>

</app:application>
~~~


Software/Firmware Install over the Air
--------------------------------------

- In AirVantage portal, click the *More* menu then *Install Application*. Select the new MQTT application you've released previously to start a FOTA/SOTA operation.
- You should be seeing the software installation request arriving in the sample device application
- The software package download and install procedure is not implemented in the sample. Your device should handle these device-specfic procedures (e.g. authenticate the package by checking signature, integrity check, sw/fw install).
- Once the software installation is performed, your application should report the status to AirVantage, by sending an ACK along with an operation id. This is showcased in the sample application.

