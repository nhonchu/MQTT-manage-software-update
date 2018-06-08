CC=gcc
CXX=g++
CFLAGS=-c -Wall -Ipaho -ItlsInterface -Imbedtls/include -ImqttInterface -ImqttAirVantage
LDFLAGS=-lpthread

SOURCES=mqttSampleAirVantage.c \
mqttAirVantage/mqttAirVantage.c mqttAirVantage/swir_json.c \
mqttInterface/mqttInterface.c \
paho/MQTTClient.c paho/MQTTLinux.c \
paho/MQTTConnectClient.c paho/MQTTConnectServer.c paho/MQTTUnsubscribeClient.c \
paho/MQTTUnsubscribeServer.c paho/MQTTSerializePublish.c paho/MQTTSubscribeClient.c \
paho/MQTTDeserializePublish.c paho/MQTTSubscribeServer.c paho/MQTTPacket.c \
mbedtls/library/aes.c mbedtls/library/cipher_wrap.c mbedtls/library/entropy_poll.c \
mbedtls/library/net.c mbedtls/library/ripemd160.c mbedtls/library/threading.c mbedtls/library/aesni.c \
mbedtls/library/error.c mbedtls/library/oid.c mbedtls/library/rsa.c mbedtls/library/timing.c mbedtls/library/arc4.c \
mbedtls/library/ctr_drbg.c mbedtls/library/gcm.c mbedtls/library/padlock.c mbedtls/library/sha1.c \
mbedtls/library/version.c mbedtls/library/asn1parse.c mbedtls/library/debug.c mbedtls/library/havege.c \
mbedtls/library/pem.c mbedtls/library/sha256.c mbedtls/library/version_features.c mbedtls/library/asn1write.c \
mbedtls/library/des.c mbedtls/library/hmac_drbg.c mbedtls/library/pk.c mbedtls/library/sha512.c mbedtls/library/x509.c \
mbedtls/library/base64.c mbedtls/library/dhm.c mbedtls/library/pkcs11.c mbedtls/library/ssl_cache.c \
mbedtls/library/x509_create.c mbedtls/library/bignum.c mbedtls/library/ecdh.c mbedtls/library/md2.c \
mbedtls/library/pkcs12.c mbedtls/library/ssl_ciphersuites.c mbedtls/library/x509_crl.c mbedtls/library/blowfish.c \
mbedtls/library/ecdsa.c mbedtls/library/md4.c mbedtls/library/pkcs5.c mbedtls/library/ssl_cli.c mbedtls/library/x509_crt.c \
mbedtls/library/camellia.c mbedtls/library/ecjpake.c mbedtls/library/md5.c mbedtls/library/pkparse.c \
mbedtls/library/ssl_cookie.c mbedtls/library/x509_csr.c mbedtls/library/ccm.c mbedtls/library/ecp.c \
mbedtls/library/md.c mbedtls/library/pk_wrap.c mbedtls/library/ssl_srv.c mbedtls/library/x509write_crt.c \
mbedtls/library/certs.c mbedtls/library/ecp_curves.c mbedtls/library/md_wrap.c mbedtls/library/pkwrite.c \
mbedtls/library/ssl_ticket.c mbedtls/library/x509write_csr.c mbedtls/library/cipher.c mbedtls/library/entropy.c \
mbedtls/library/memory_buffer_alloc.c mbedtls/library/platform.c mbedtls/library/ssl_tls.c mbedtls/library/xtea.c

CXXSOURCES=tlsInterface/BaseSocket.cpp tlsInterface/LinuxSocket.cpp tlsInterface/LinuxTLSSocket.cpp tlsInterface/SocketInterface.cpp

OBJECTS=$(SOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
EXECUTABLE=mqttSampleAirVantage

all: $(SOURCES) $(CXXSOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) $(CXXOBJECTS)
	$(CXX) $(OBJECTS) $(CXXOBJECTS) -o $@ $(LDFLAGS) 
	

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o \
	rm -rf tlsInterface/*.o \
	rm -rf mqttInterface/*.o \
	rm -rf paho/*.o \
	rm -rf mqttAirVantage/*.o \
	rm -rf mbedtls/library/*.o