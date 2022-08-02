#include "select_backend.h"
#include "server_core.h"


typedef struct {
	int cnt;
	svr_event_t** events;
}events_list;


typedef struct {
	fd_set read_fdsets;
	events_list ev_list;
}select_data;


static svr_event_t* get_event(select_data* data, int fd)
{
	events_list* evlist = &data->ev_list;
	for (int i = 0; i < evlist->cnt; ++i)
	{
		if (evlist->events[i]->fd == fd)
		{
			return evlist->events[i];
		}
	}
	return nullptr;
}


static void* select_init(server_t* server)
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return nullptr;
	}

	select_data* data = (select_data*)calloc(1, sizeof(select_data));
	FD_ZERO(&data->read_fdsets);

	data->ev_list.events = (svr_event_t**)calloc(FD_SETSIZE, sizeof(svr_event_t*));
	data->ev_list.cnt = 0;
	
	return (void*)data;
}

static int select_add(server_t* server, svr_event_t* ev)
{
	select_data* data = (select_data*)server->backend_data;
	if (data->ev_list.cnt >= FD_SETSIZE)
	{
		return -1;
	}

	FD_SET(ev->fd, &data->read_fdsets);

	data->ev_list.events[data->ev_list.cnt] = ev;
	++data->ev_list.cnt;

	return 0;
}

static void select_del(server_t* server, svr_event_t* ev)
{
	select_data* data = (select_data*)server->backend_data;
	closesocket(ev->fd);
	FD_CLR(ev->fd, &data->read_fdsets);

	events_list* ev_list = &data->ev_list;
	for (int i = 0; i < ev_list->cnt; ++i)
	{
		if (ev_list->events[i]->fd == ev->fd)
		{
			ev_list->events[i] = ev_list->events[ev_list->cnt - 1];
			--ev_list->cnt;

			break;
		}
	}
}

static int select_dispatch(server_t* server, struct timeval* tm)
{
	select_data* data = (select_data*)server->backend_data;

	fd_set tmp;
	FD_ZERO(&tmp);

	tmp = data->read_fdsets;
	int ret = select(0, &tmp, NULL, NULL, tm);
	if (ret < 0)
	{
		int err = ::WSAGetLastError();
		return -1;
	}

	for (int i = 0; i < (int)tmp.fd_count; ++i)
	{
		svr_event_t* ev = nullptr;
		if (FD_ISSET(i, tmp.fd_array))
		{
			svr_event_t* ev = get_event(data, (int)tmp.fd_array[i]);
			if (!ev)
			{
				continue;
			}

			svr_event_active(server, ev);
		}
	}

	return 0;
}


static int select_finit(server_t* server)
{
	select_data* data = (select_data*)server->backend_data;
	if (data->ev_list.events)
	{
		free(data->ev_list.events);
	}

	free(data);

	return 0;
}


int net_read_package(int fd)
{
	int len = 0;
	stream_t* stream = nullptr;//backend->get_stream_cb(fd);
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
		//backend->close_cb(fd);
	}
	else if (n < 0 && ::WSAGetLastError() != WSAEWOULDBLOCK)
	{
		// some error
		//backend->err_cb(fd);
	}
	else
	{
		// process
		//backend->process_cb(fd, stream, overflow);
	}

	return 0;
}


/*
int net_loop()
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
					//net_onconnect(backend);
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
*/


int net_send_package(int fd, char* buff, int len)
{
	return send(fd, buff, len, 0);
}