/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * LinuxSocket Class :  Integrating mBed's httpClient & mBed's TLS for Linux usage
 *
 * Nhon Chu - March 2016 
 *
 */

#ifndef LINUXTLSSOCKET_H
#define LINUXTLSSOCKET_H

#include "BaseSocket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>

 #if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#endif

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
 
/**
TCP socket connection
*/
class LinuxTLSSocket : public BaseSocket
{
	
public:
	/** TCP socket connection
	*/
	LinuxTLSSocket();
	
	~LinuxTLSSocket();

	/** Connects this TCP socket to the server
	\param host The host to connect to. It can either be an IP Address or a hostname that will be resolved with DNS.
	\param port The host's port to connect to.
	\return 0 on success, -1 on failure.
	*/
	int connect(const char* host, const int port);

	/** Close the TCP socket
	*/
	void close();
	
	/** Check if the socket is connected
	\return true if connected, false otherwise.
	*/
	bool is_connected(void);
	
	/** Set blocking or non-blocking mode of the socket and a timeout on
		blocking socket operations
	\param blocking  true for blocking mode, false for non-blocking mode.
	\param timeout   timeout in ms [Default: (1500)ms].
	*/
	void set_blocking(bool blocking, unsigned int timeout_ms=1500);

	/** Send data to the remote host.
	\param data The buffer to send to the host.
	\param length The length of the buffer to send.
	\return the number of written bytes on success (>=0) or -1 on failure
	 */
	int send(const char* data, int length);
	
	/** Send all the data to the remote host.
	\param data The buffer to send to the host.
	\param length The length of the buffer to send.
	\return the number of written bytes on success (>=0) or -1 on failure
	*/
	int send_all(const char* data, int length);
	
	/** Receive data from the remote host.
	\param data The buffer in which to store the data received from the host.
	\param length The maximum length of the buffer.
	\return the number of received bytes on success (>=0) or -1 on failure
	 */
	int receive(char* data, int length);

	/** Receive data from the remote host.
	\param data The buffer in which to store the data received from the host.
	\param dataSize The maximum length of the buffer.
	\param searchPattern : the pattern to search for, reading will stop when pattern is found
	\return the number of received bytes on success (>=0) or -1 on failure
	 */
	int receive(char* data, int dataSize, const char* searchPattern);


	

private:
	void 						freeSSL();
	void 						getSSLerror(int errorCode);


	char						_trustedCaFolderName[256];

	mbedtls_net_context 		_server_fd;
	mbedtls_entropy_context     _entropy;
	mbedtls_ctr_drbg_context    _ctr_drbg;
	mbedtls_ssl_context         _ssl;
	mbedtls_ssl_config          _conf;
	mbedtls_x509_crt            _cacert;

};

#endif
