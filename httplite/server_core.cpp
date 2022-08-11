#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "select_backend.h"
#include "server_core.h"


static void svr_process_active(server_t* server)
{
	svr_event_t* ev = nullptr;
	DLIST_FOREEACH(ev, &server->act_head, actentry)
	{
		// 处理事件
		ev->event_callback(ev->fd, ev->arg);
		// 取出活动队列
		svr_event_del(ev, SVR_EV_QUEUE_ACTIVE);
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
		// 将带有事件的文件句柄放入活动队列
		svr->backend->func_dispatch(svr, &tv);
		// 处理活动队列
		svr_process_active(svr);
	}

	return 0;
}


server_t* svr_new(svr_backend_t* backend)
{
	server_t* svr = (server_t*)calloc(1, sizeof(server_t));
	if (nullptr == svr)
	{
		return nullptr;
	}
	svr->backend = backend;
	svr->backend_data = svr->backend->func_init(svr);

	return svr;
}


void svr_close(server_t* svr)
{
	svr->backend->func_finit(svr);
	free(svr);
}


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back, void* arg)
{
	svr_event_t* ev = (svr_event_t*)calloc(1, sizeof(svr_event_t));
	if (ev == nullptr)
	{
		return nullptr;
	}

	ev->svr = server;
	ev->fd = fd;
	ev->event_callback = call_back;
	ev->arg = arg;

	return ev;
}


void svr_delete_event(server_t* server, svr_event_t* ev)
{
	free(ev);
}


int svr_new_listener(server_t* server, int port, event_callback call_back, void* arg)
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

		ev = svr_new_event(server, svrfd, call_back, arg);
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
	server_t* server = ev->svr;
	svr_backend_t* backend = server->backend;
	// 如果还没有在IO后端监听事件中，先加入进去
	if ((nqueue == SVR_EV_QUEUE_WAIT) && !(ev->nqueue & SVR_EV_QUEUE_WAIT))
	{
		if (backend->func_add(server, ev) != 0)
		{
			return -1;
		}
	}

	ev->nqueue |= nqueue;
	// 插入到管理列表中
	if (nqueue == SVR_EV_QUEUE_WAIT)
	{
		DLIST_INSERT_HEAD(&server->ev_head, ev, entry);
	}
	else if (nqueue == SVR_EV_QUEUE_ACTIVE)
	{
		DLIST_INSERT_HEAD(&server->act_head, ev, actentry);
	}

	return 0;
}


int svr_event_del(svr_event_t* ev, int nqueue)
{
	server_t* server = ev->svr;
	svr_backend_t* backend = server->backend;

	if (nqueue == SVR_EV_QUEUE_WAIT && ev->nqueue & SVR_EV_QUEUE_WAIT)
	{
		// 从后端监听队列中取出
		backend->func_del(server, ev);
	}

	ev->nqueue &= ~nqueue;

	if (nqueue == SVR_EV_QUEUE_WAIT)
	{
		DLIST_REMOVE(ev, entry);
	}
	else if (nqueue == SVR_EV_QUEUE_ACTIVE)
	{
		DLIST_REMOVE(ev, actentry);
	}

	return 0;
}
