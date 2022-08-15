#pragma once

#include "queue.h"

/*
* httpsvr->conn_head
*				|
*				conn<-->conn<-->conn....
*				|
*				request->request->request....
* ��������һ���������ӵļ��ϣ�����ʹ��˫���������ݽṹ���й���
* ����������һ������request�Ķ��У�����ʹ��FIFO���ݽṹ���й���
* ÿ��request����һ����ȡtcp��������״̬��
*/


//=============================================================
// ���Ӷ����ϵ��������

// ���󷽷�
#define HTTP_METHOD_REQ_GET		1
#define HTTP_METHOD_REQ_POST	2

struct http_request {
	// �ڴ����
	QUEUE_ENTRY(http_request, qentry);
	// �����·��
	char* url;
	// �����������Ӷ���
	struct http_connection* conn;
	// ���������μ�HTTP_METHOD_**
	unsigned char method;
};


//=============================================================
// http�����������Ӷ���
// HTTP_STAGE_READING_PREPARE -> HTTP_STAGE_READING_HEADER -> HTTP_STAGE_READING_BODY

// ����׶�
#define HTTP_STAGE_READING_PREPARE	1	// ����׼���׶�
#define HTTP_STAGE_READING_HEADER	2	// ��ȡhttpͷ
#define HTTP_STAGE_READING_BODY		3	// ������

struct http_connection {
	// �ڴ����
	DLIST_ENTRY(http_connection, entry);
	// ���Ӷ���
	int fd;
	// fd�¼�
	struct svr_event_t* ev;
	// ������
	struct szbuff* inputbuffer;
	// �ڴ������ϵ������������
	QUEUE_HEAD(http_request) reqhead;
	// ��ǰ����׶Σ��μ�HTTP_REQ_STAGE_***
	unsigned char stage;

	struct http_server* httpsvr;
};


//=============================================================
// request����
typedef int (*request_handle)(struct http_request* req, void* arg);

// http��������handle����
struct http_request_handle {
	// �ڴ����
	DLIST_ENTRY(http_request_handle, entry);
	// �����·��
	char* url;
	// ����Ļص�����
	request_handle cb;
	// �ص�����
	void* arg;
};


//=============================================================
// http�������Ķ���
// �������˷�conn_head���������Լ���request���˷�request_cb_head����
struct http_server {
	// ����ص�����
	DLIST_HEAD(http_request_handle) req_cb_head;
	// ���������ӹ���
	DLIST_HEAD(http_connection) conn_head;
	// �������¼��������
	struct server_t* svrcore;
};


http_server* httpsvr_new(server_t* svrcore, int port);
void httpsvr_free(http_server* httpsvr);
int httpsvr_run(http_server* httpsvr);

// ע��url����ӿ�
int httpsvr_register_request(http_server* httpsvr, char* url, request_handle cb, void* arg);
// �ر�����
void httpsvr_close_connection(http_connection* conn, int reason);
