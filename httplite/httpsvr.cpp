#include <WinSock2.h>
#include "httpsvr.h"
#include "server_core.h"
#include "select_backend.h"
#include "buff.h"


// ƴ��tcp��
static int httpsvr_read_callback(int fd, void* arg)
{
	http_connection* conn = (http_connection*)arg;
	return 0;
}


static void httpsvr_free_connection(http_server* httpsvr, http_connection* conn)
{
	// �ͷ�event
	if (conn->ev)
	{
		// �Ӻ�˼���������ȡ��
		svr_event_del(conn->ev, SVR_EV_QUEUE_WAIT);
		// �ͷ��ڴ�
		svr_delete_event(httpsvr->svrcore, conn->ev);
		conn->ev = nullptr;
	}
	// �ͷ����뻺����
	if (conn->inputbuffer)
	{
		buff_delete(conn->inputbuffer);
		conn->inputbuffer = nullptr;
	}
	// �ͷ�����
	if (httpsvr->conn_head == conn)
	{
		httpsvr->conn_head = nullptr;
	}
	else
	{
		if (conn->pre)
		{
			conn->pre->next = conn->next;
		}

		if (conn->next)
		{
			conn->next->pre = conn->pre;
		}

		conn->next = conn->pre = nullptr;
	}
	// �ͷ������ڴ汾��
	free(conn);	
}


static http_connection* httsvr_new_connection(http_server* httpsvr, int fd)
{
	http_connection* conn = (http_connection*)calloc(1, sizeof(http_connection));
	if (conn == nullptr)
	{
		return nullptr;
	}

	conn->ev = svr_new_event(httpsvr->svrcore, fd, httpsvr_read_callback, conn);
	conn->inputbuffer = buff_new();
	if (conn->ev == nullptr || conn->inputbuffer == nullptr)
	{
		httpsvr_free_connection(httpsvr, conn);
		return nullptr;
	}

	// Ͷ�ݶ��¼�
	svr_event_add(conn->ev, SVR_EV_QUEUE_WAIT);

	conn->fd = fd;
	conn->httpsvr = httpsvr;
	// ��������
	http_connection** head = &httpsvr->conn_head;
	if ((*head) == nullptr)
	{
		(*head) = conn;
	}
	else
	{
		// ���뵽����ͷ��
		conn->next = *head;
		conn->pre = nullptr;
		(*head)->pre = conn;
		(*head) = conn;
	}

	return conn;
}


static void httpsvr_free_request(http_request* req)
{
	// ��������ȡ��

	// �ͷ��ַ����ڴ�
	
	// �ͷ��ڴ�
}


static http_request* httpsvr_new_request(http_connection* conn)
{
	http_request* req = (http_request*)calloc(1, sizeof(http_request));
	if (nullptr == req)
	{
		return nullptr;
	}

	return nullptr;
}


static int on_accept(int fd, void* arg)
{
	http_server* httpsvr = (http_server*)arg;

	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	int client_fd = (int)accept(fd, (struct sockaddr*)&client_addr, &addr_len);

	unsigned long mod = 1;
	int ret = ioctlsocket(client_fd, FIONBIO, &mod);

	// ��������fd�����Ӷ���
	http_connection* conn = httsvr_new_connection(httpsvr, client_fd);
	if (conn == nullptr)
	{
		closesocket(client_fd);
		return -1;
	}
	// ����request����������������
	http_request* req = httpsvr_new_request(conn);
	if (req == nullptr)
	{
		httpsvr_free_connection(httpsvr, conn);
		closesocket(client_fd);
		return -1;
	}

	return 0;
}


http_server* httpsvr_new(server_t* svrcore, int port)
{
	server_t* svr = svrcore;
	if (svr == nullptr)
	{
		svr_backend_t* bk = get_select_backend();
		svr = svr_new(bk);
	}

	if (svr == nullptr)
	{
		return nullptr;
	}

	http_server* httpsvr = (http_server*)calloc(1, sizeof(http_server));
	if (nullptr == httpsvr)
	{
		svr_close(svr);
		return nullptr;
	}

	httpsvr->svrcore = svr;

	int ret = svr_new_listener(svr, port, on_accept, httpsvr);
	if (ret != 0)
	{
		httpsvr_free(httpsvr);
		return nullptr;
	}

	return httpsvr;
}


void httpsvr_free(http_server* httpsvr)
{
	server_t* svr = httpsvr->svrcore;
	if (svr)
	{
		svr_close(svr);
	}
	free(httpsvr);
}


int httpsvr_run(http_server* httpsvr)
{
	return svr_run(httpsvr->svrcore);
}


// ע��url����ӿ�
int httpsvr_register_request(http_server* httpsvr, char* url, request_handle cb, void* arg)
{
	http_request_handle* handle = (http_request_handle*)calloc(1, sizeof(http_request_handle));
	if (handle == nullptr)
	{
		return -1;
	}

	handle->arg = arg;
	handle->cb = cb;
	handle->url = _strdup(url);

	http_request_handle** head = &httpsvr->requect_cb_head;
	if (*head == nullptr)
	{
		*head = handle;
		(*head)->next = (*head)->pre = nullptr;
	}
	else
	{
		// ���뵽����ͷ��
		handle->next = *head;
		handle->pre = nullptr;
		(*head)->pre = handle;
		(*head) = handle;
	}

	return 0;
}

