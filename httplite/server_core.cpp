#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "select_backend.h"
#include "buff_stream.h"
#include "server_core.h"


static void svr_process_active(server_t* server)
{
	svr_event_t* ev = server->active_evs;
	while (ev)
	{
		// 处理事件
		ev->event_callback(ev->fd, server);

		// 取出活动队列

	}
}


int svr_run(server_t* svr)
{
	while (1)
	{
		if (svr->loop_break)
		{
			break;
		}

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		svr->backend->func_dispatch(svr, &tv);

		// 处理活动队列
		svr_process_active(svr);
	}

	return 0;
}


server_t* svr_new(svr_backend_t* backend)
{
	server_t* svr = (server_t*)calloc(1, sizeof(server_t));
	svr->backend = backend;
	svr->backend_data = svr->backend->func_init(svr);

	return svr;
}


void svr_close(server_t* svr)
{
	svr->backend->func_finit(svr);
	free(svr);
}


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back)
{
	svr_event_t* ev = (svr_event_t*)calloc(1, sizeof(svr_event_t));
	if (ev == nullptr)
	{
		return nullptr;
	}

	ev->svr = server;
	ev->fd = fd;
	ev->event_callback = call_back;

	return ev;
}


void svr_delete_event(server_t* server, svr_event_t* ev)
{
	free(ev);
}


int svr_new_listener(server_t* server, int port, event_callback call_back)
{
	int svrfd = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (svrfd < 0)
	{
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = 0;
	svr_event_t* ev = nullptr;
	do 
	{
		ret = bind(svrfd, (struct sockaddr*)&addr, sizeof(addr));
		if (ret < 0)
		{
			break;
		}

		ret = listen(svrfd, 20);
		if (ret < 0)
		{
			break;
		}

		ev = svr_new_event(server, svrfd, call_back);
		if (ev == nullptr)
		{
			break;
		}

		ret = svr_event_add(ev, SVR_EV_QUEUE_WAIT);
		if (ret < 0)
		{
			break;
		}
	} while (0);

	if (ret < 0 || ev == nullptr)
	{
		closesocket(svrfd);
		if (ev)
		{
			svr_delete_event(server, ev);
		}

		return -1;
	}
	
	return 0;
}


int svr_event_add(svr_event_t* ev, int nqueue)
{
	svr_event_t* headev = nullptr;
	switch (nqueue)
	{
		case SVR_EV_QUEUE_WAIT:
		{
			headev = ev->svr->event_head;
			ev->pre = nullptr;
			ev->next = headev;
			headev->pre = ev;
			ev->svr->event_head = ev;

			return ev->svr->backend->func_add(ev->svr, ev);
		}
		break;
		case SVR_EV_QUEUE_ACTIVE:
		{
			headev = ev->svr->active_evs;
			ev->pre = nullptr;
			ev->next = headev;
			headev->pre = ev;
			ev->svr->active_evs = ev;
		}
		break;
		default:
			return -1;
	}

	return 0;
}


int svr_event_del(svr_event_t* ev, int nqueue)
{
	server_t* server = ev->svr;

	switch (nqueue)
	{
		case SVR_EV_QUEUE_WAIT:
		{
			if (ev == server->event_head)
			{
				server->event_head = nullptr;
				return 0;
			}

			if (ev->next)
			{
				ev->next->pre = ev->pre;
			}

			if (ev->pre)
			{
				ev->pre->next = ev->next;
			}
		}
		break;
		case SVR_EV_QUEUE_ACTIVE:
		{
			if (ev == server->active_evs)
			{
				server->active_evs = nullptr;
				return 0;
			}

			if (ev->next)
			{
				ev->next->pre = ev->pre;
			}

			if (ev->pre)
			{
				ev->pre->next = ev->next;
			}
		}
		break;
		default:
			return -1;
	}

	return 0;
}