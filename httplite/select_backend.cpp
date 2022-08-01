#include "select_backend.h"
#include "server_core.h"


typedef struct {
	fd_set fdsets;
	svr_event_t* events[FD_SETSIZE];
}select_data;


static void* select_init(server_t* server)
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return nullptr;
	}

	select_data* data = (select_data*)calloc(1, sizeof(select_data));
	FD_ZERO(&data->fdsets);
	
	return (void*)data;
}

static int select_add(server_t* server, int fd)
{
	select_data* data = (select_data*)server->backend_data;
	FD_SET(fd, &data->fdsets);

	return 0;
}

static void select_del(server_t* server, int fd)
{
	select_data* data = (select_data*)server->backend_data;
	closesocket(fd);
	FD_CLR(fd, &data->fdsets);
}

static int select_dispatch(server_t* server, struct timeval* tm)
{
	select_data* data = (select_data*)server->backend_data;

	fd_set tmp;
	FD_ZERO(&tmp);

	tmp = data->fdsets;
	int ret = select(0, &tmp, NULL, NULL, tm);
	if (ret < 0)
	{
		int err = ::WSAGetLastError();
		return -1;
	}

	for (int i = 0; i < tmp.fd_count; ++i)
	{
		svr_event_t* ev = nullptr;
		if (FD_ISSET(i, tmp.fd_array))
		{
			ev = data->events[i];
			svr_event_active(server, ev);
		}
	}

	return 0;
}


static int select_finit(server_t* server)
{
	
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

	fd_set tmp;
	FD_ZERO(&tmp);

	while (true)
	{
		tmp = backend->fdsets;
		int ret = select(0, &tmp, NULL, NULL, &t);
		if (ret < 0)
		{
			int err = ::WSAGetLastError();
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