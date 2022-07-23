#include "net.h"


int net_init(net_backend_t* backend)
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return -1;
	}

	backend->svrfd = -1;
	FD_ZERO(&backend->fdsets);

	return 0;
}


void net_close(net_backend_t* backend)
{
	if (backend->svrfd > 0)
	{
		closesocket(backend->svrfd);
	}

	WSACleanup();
	FD_ZERO(&backend->fdsets);
}


void net_close_client(net_backend_t* backend, int client_fd)
{
	closesocket(client_fd);
	FD_CLR(client_fd, &backend->fdsets);
}

int net_listen_port(net_backend_t* backend, int port)
{
	backend->svrfd = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (backend->svrfd < 0)
	{
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = bind(backend->svrfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		net_close(backend);
		return -1;
	}

	ret = listen(backend->svrfd, 20);
	if (ret < 0)
	{
		net_close(backend);
		return -1;
	}

	return 0;
}


static int net_onconnect(net_backend_t* backend)
{
	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	int client_fd = (int)accept(backend->svrfd, (struct sockaddr*)&client_addr, &addr_len);
	int ret = backend->accept_cb(client_fd);
	if (ret < 0)
	{
		closesocket(client_fd);
		return -1;
	}

	FD_SET(client_fd, &backend->fdsets);

	return client_fd;
}


static int net_read_package(net_backend_t* backend, int fd)
{
	int len = 0;
	stream_t* stream = backend->get_stream_cb(fd);
	if (stream == NULL)
	{
		return -1;
	}

	int n = 0;
	bool overflow = false;
	do 
	{
		if (stream->len >= stream->cap)
		{
			overflow = true;
			break;
		}

		char* begin = stream->buff + stream->len;
		int size = stream->cap - stream->len;
		n = recv(fd, begin, size, 0);
		if (n > 0)
		{
			stream->len += n;
		}

	} while ( n > 0 && overflow == false);

	if (n == 0)
	{
		// gracefully closed
		backend->close_cb(fd);
	}
	else if (n < 0 && ::WSAGetLastError() != WSAEWOULDBLOCK)
	{
		// some error
		backend->err_cb(fd);
	}
	else
	{
		// process
		backend->process_cb(fd, stream, overflow);
	}

	return 0;
}


int net_loop(net_backend_t* backend)
{
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1000;

	fd_set tmp = backend->fdsets;

	while (true)
	{
		int ret = select(0, &tmp, NULL, NULL, &t);
		if (ret < 0)
		{
			return -1;
		}

		for (unsigned int i = 0; i < tmp.fd_count; ++i)
		{
			int fd = (int)tmp.fd_array[i];
			if (FD_ISSET(fd, &tmp))
			{
				if (fd == backend->svrfd)
				{
					// do accept
					net_onconnect(backend);
				}
				else
				{
					// do recv
					net_read_package(backend, fd);
				}
			}
		}
	}

	return 0;
}


int net_send_package(int fd, char* buff, int len)
{
	return send(fd, buff, len, 0);
}