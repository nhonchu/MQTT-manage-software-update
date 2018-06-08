/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTLinux.h"

#define USE_SOCKET_CLASS

#ifdef USE_SOCKET_CLASS
#include "SocketInterface.h"
#endif

char expired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void countdown_ms(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void countdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int left_ms(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}


void InitTimer(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
}


int linux_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	if (timeout_ms <= 0)
	{
		timeout_ms = 500;
	}
	
#ifdef USE_SOCKET_CLASS
	if (n->pSocketInstance)
	{
		SOCKET_setTimeout(n->pSocketInstance, timeout_ms);
		return SOCKET_receive(n->pSocketInstance, (char *) buffer, len);
	}
	return -1;
#else
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
			bytes += rc;
	}
	return bytes;
#endif
}


int linux_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
#ifdef USE_SOCKET_CLASS
	if (n->pSocketInstance)
	{
		SOCKET_setTimeout(n->pSocketInstance, timeout_ms);
		return SOCKET_send(n->pSocketInstance, (char *)buffer, len);
	}
	return 0;
#else
	struct timeval tv;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = write(n->my_socket, buffer, len);
	return rc;
#endif
}


void linux_disconnect(Network* n)
{
	if (!n->disconnect)
	{
		return;
	}
	
#ifdef USE_SOCKET_CLASS
	if (n->pSocketInstance)
	{
		SOCKET_close(n->pSocketInstance);
		n->pSocketInstance = NULL;	
	}
#else
	if (n->my_socket != -1)
	{
		close(n->my_socket);
		n->my_socket = -1;
	}

#endif
}


void NewNetwork(Network* n)
{
	n->pSocketInstance = NULL;
	n->my_socket = -1;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
	n->connect = linux_connect;
	n->disconnect = linux_disconnect;
}


int linux_connect(Network* n, char* addr, int port, int useTLS)
{
	int rc = -1;

#ifdef USE_SOCKET_CLASS
	if (n->pSocketInstance)
	{
		SOCKET_close(n->pSocketInstance);
	}
	n->pSocketInstance = SOCKET_connect(addr, port, useTLS);
	if (n->pSocketInstance)
	{
		rc = 0;
	}

#else
	if (n->my_socket != -1)
	{
		close(n->my_socket);
	}
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
		{
			//int opt = 1;			
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
		}
	}
#endif

	return rc;
}