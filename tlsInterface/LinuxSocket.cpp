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
 * LinuxSocket Class :  customized mBed's httpClient for Linux usage
 *
 * Nhon Chu - March 2016 
 *
 */


#include "LinuxSocket.h"


LinuxSocket::LinuxSocket() :
		_sock_fd(-1),
		_timeout_ms(2000)
{
}


LinuxSocket::~LinuxSocket()
{
	close();
}

int LinuxSocket::connect(const char* host, const int port)
{
	int                 type = SOCK_STREAM;
	struct sockaddr_in  address;
	int                 rc = -1;
	sa_family_t         family = AF_INET;
	struct addrinfo *   result = NULL;
	struct addrinfo     hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(host, NULL, &hints, &result)) == 0)
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
		{
			rc = -1;
		}

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		_sock_fd = socket(family, type, 0);
		if (_sock_fd >= 0)
		{
			rc = ::connect(_sock_fd, (struct sockaddr*)&address, sizeof(address));

			if (0 == rc)
			{
				_is_connected = true;
			}
		}
	}

	return rc;
}

void  LinuxSocket::close()
{
	if (_sock_fd >= 0)
	{
		::close(_sock_fd);
		_sock_fd = -1;
	}
}

bool LinuxSocket::is_connected(void)
{
	return _is_connected;
}

void LinuxSocket::set_blocking(bool blocking, unsigned int timeout_ms)
{
	_timeout_ms = timeout_ms;
}

int LinuxSocket::send(const char* data, int length)
{
	if ((_sock_fd < 0) || !_is_connected)
	{
		return -1;
	}
	
	struct timeval interval;

	interval.tv_sec = 0;  /* Secs Timeout */
	interval.tv_usec = _timeout_ms * 1000;

	setsockopt(_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int rc = write(_sock_fd, data, length);

	_is_connected = (rc != 0);

	return rc;
}

int LinuxSocket::send_all(const char* data, int length)
{
	return send(data, length);
}

int LinuxSocket::receive(char* data, int length)
{
	if ((_sock_fd < 0) || !_is_connected)
	{
		fprintf(stdout, "LinuxSocket::receive - oops, problem here");
		return -1;
	}
	
	struct timeval interval = {_timeout_ms / 1000, (_timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < length)
	{
		//fprintf(stdout, "LinuxSocket::receive - reading %lu bytes", (size_t)(length - bytes));

		int rc = recv(_sock_fd, &data[bytes], (size_t)(length - bytes), 0);
		if (rc < 0)
		{
			//_is_connected = false;
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else if (rc > 0)
		{
			bytes += rc;
		}
		else
		{
			break;
		}
	}

	//fprintf(stdout, "LinuxSocket::receive - read %d bytes within %u ms", bytes, _timeout_ms);
	return bytes;
}


int LinuxSocket::receive(char* data, int dataSize, const char* searchPattern)
{
    if ((_sock_fd < 0) || !_is_connected)
    {
        return -1;
    }
    
    if (NULL == searchPattern)
    {
    	fprintf(stdout, "LinuxSocket::receive - error : No Pattern");
    	return 0;
    }

    if (strlen(searchPattern) == 0)
    {
    	fprintf(stdout, "LinuxSocket::receive - error : Empty Pattern");
    	return 0;
    }

    struct timeval interval;

    interval.tv_sec = 0;  /* Secs Timeout */
    interval.tv_usec = 1000 * 1000;

    setsockopt(_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    //fprintf(stdout, "LinuxSocket::receive - search for Pattern %s", searchPattern);

    int 	index = 0;
    int     nMatchCount = strlen(searchPattern);
	int     nSearchIndex = 0;
	char 	buf;

	while (recv(_sock_fd, &buf, 1, 0) == 1)
	{
		data[index++] = buf;

		if (buf == searchPattern[nSearchIndex])
		{
			nSearchIndex++;
			if (nSearchIndex == nMatchCount)
			{
				fprintf(stdout, "LinuxSocket::receive - Found Pattern. Data size = %d", index);
				break;
			}
		}
		else
		{
			nSearchIndex = 0;
		}

		if (index >= dataSize)
		{
			fprintf(stdout, "LinuxSocket::receive - Pattern not found. Buffer too short");
			break;		
		}
	}

	//fprintf(stdout, "LinuxSocket::receive - read count = %d", index);
	data[index] = '\0';
	return index;
}


