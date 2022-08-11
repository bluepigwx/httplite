#include <WinSock2.h>
#include "httpsvr.h"
#include "server_core.h"
#include "select_backend.h"
#include "buff.h"
#include "queue.h"


static http_request* httpsvr_new_request(http_connection* conn);
static void httpsvr_free_request(http_request* req);

static http_connection* httsvr_new_connection(http_server* httpsvr, int fd);
static void httpsvr_free_connection(http_server* httpsvr, http_connection* conn);



// �����ر�����
static void httpsvr_close_connection(http_connection* conn, int reason)
{
	http_server* httpsvr = conn->httpsvr;
	// �ر�����
	closesocket(conn->fd);
	// �����ڴ�
	httpsvr_free_connection(httpsvr, conn);
}


// ƴ��tcp��
static int httpsvr_read_callback(int fd, void* arg)
{
	http_connection* conn = (http_connection*)arg;
	szbuff* buff = conn->inputbuffer;
	int maxlen = buff->total - (buff->align + buff->off);
	int ret = buff_read_fd(buff, fd, maxlen);
	// ����ret�ķ���ֵ�ж��������ӵ�״̬
	if (ret == -1)
	{
		if (errno != EINTR && errno != EAGAIN)
		{
			// ������
			httpsvr_close_connection(conn, 0);
			return -1;
		}
	}
	else if (ret == 0)
	{
		// ������
		httpsvr_close_connection(conn, 0);
	}
	// else �����հ�

	// ��ʼ����request״̬��

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
	// �ͷŰ󶨵�request
	http_request* req = DLIST_GET_FIRST(&conn->reqhead);
	while (req)
	{
		httpsvr_free_request(req);
		req = DLIST_GET_FIRST(&conn->reqhead);
	}
	// �ͷ���������
	DLIST_REMOVE(conn, entry);
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
	DLIST_INSERT_HEAD(&httpsvr->conn_head, conn, entry);

	return conn;
}


static void httpsvr_free_request(http_request* req)
{
	// ������������ȡ��
	DLIST_REMOVE(req, entry);
	// �ͷ��ַ����ڴ�
	if (req->url)
	{
		free(req->url);
	}
	// �ͷ��ڴ�
	free(req);
}


static http_request* httpsvr_new_request(http_connection* conn)
{
	http_request* req = (http_request*)calloc(1, sizeof(http_request));
	if (nullptr == req)
	{
		return nullptr;
	}

	req->conn = conn;
	// �����ϼ���
	DLIST_INSERT_HEAD(&conn->reqhead, req, entry);

	return req;
}


static int on_accept(int fd, void* arg)
{
	http_server* httpsvr = (http_server*)arg;

	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	int client_fd = (int)accept(fd, (struct sockaddr*)&client_addr, &addr_len);
	// ���÷�����ģʽ
	unsigned long mod = 1;
	int ret = ioctlsocket(client_fd, FIONBIO, &mod);

	// ��������fd�����Ӷ���
	http_connection* conn = httsvr_new_connection(httpsvr, client_fd);
	if (conn == nullptr)
	{
		closesocket(client_fd);
		return -1;
	}
	// ����request��������������������ʵreq������Ǹ�״̬������Ϊtcp��������һ�����ղ���
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
	// �����ϻص�����
	DLIST_INSERT_HEAD(&httpsvr->req_cb_head, handle, entry);

	return 0;
}

