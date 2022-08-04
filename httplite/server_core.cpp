#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "select_backend.h"
#include "server_core.h"


static void svr_process_active(server_t* server)
{
	svr_event_t* ev = server->active_evs;

	while (ev)
	{
		// 处理事件
		ev->event_callback(ev->fd, ev);
		// 取出活动队列
		svr_event_del(ev, SVR_EV_QUEUE_ACTIVE);
		// 取下一个元素
		ev = ev->act_next;
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

	svr_event_t** headev = nullptr;
	svr_event_t** pre = nullptr;
	svr_event_t** next = nullptr;
	if (nqueue == SVR_EV_QUEUE_WAIT)
	{
		headev = &server->event_head;
		pre = &ev->pre;
		next = &ev->next;
	}
	else if (nqueue == SVR_EV_QUEUE_ACTIVE)
	{
		headev = &server->active_evs;
		pre = &ev->act_pre;
		next = &ev->act_next;
	}

	if (headev == nullptr)
	{
		if ((ev->nqueue & SVR_EV_QUEUE_WAIT))
		{
			backend->func_del(server, ev);
		}
		return -1;
	}

	if (*headev == nullptr)
	{
		*headev = ev;
		*next = *pre = nullptr;

		return 0;
	}
	// 插入联表头部
	*pre = nullptr;
	(*headev)->pre = ev;
	*next = (*headev);
	(*headev) = ev;

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

	svr_event_t** headev = nullptr;
	svr_event_t** pre = nullptr;
	svr_event_t** next = nullptr;
	if (nqueue == SVR_EV_QUEUE_WAIT)
	{
		headev = &server->event_head;
		pre = &ev->pre;
		next = &ev->next;
	}
	else if (nqueue == SVR_EV_QUEUE_ACTIVE)
	{
		headev = &server->active_evs;
		pre = &ev->act_pre;
		next = &ev->act_next;
	}

	if (headev == nullptr)
	{
		return -1;
	}

	if (ev == *headev)
	{
		*headev = nullptr;
		*next = *pre = nullptr;
		return 0;
	}

	if (*next)
	{
		(*next)->pre = *pre;
	}

	if (*pre)
	{
		(*pre)->next = *next;
	}

	return 0;
}
