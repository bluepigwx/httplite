#pragma once


//=============================================================
// ���Ӷ����ϵ��������
struct http_request {
	// �ڴ����
	http_request* pre;
	http_request* next;
	// �����·��
	char* url;
	// �����������Ӷ���
	struct http_connection* conn;
};


//=============================================================
// http�����������Ӷ���
struct http_connection {
	// �ڴ����
	http_connection* pre;
	http_connection* next;
	// ���Ӷ���
	int fd;
	// fd�¼�
	struct svr_event_t* ev;
	// ������
	struct szbuff* inputbuffer;
	// �ڴ������ϵ����������б�
	http_request* reqhead;

	struct http_server* httpsvr;
};


//=============================================================
// request����
typedef int (*request_handle)(struct http_request* req, void* arg);

// http��������handle����
struct http_request_handle {
	// �ڴ����
	http_request_handle* pre;
	http_request_handle* next;
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
	// request�ص����������ͷ
	http_request_handle* requect_cb_head;
	// ���������ӹ���
	http_connection* conn_head;
	// �������¼��������
	struct server_t* svrcore;
};


http_server* httpsvr_new(server_t* svrcore, int port);
void httpsvr_free(http_server* httpsvr);
int httpsvr_run(http_server* httpsvr);

// ע��url����ӿ�
int httpsvr_register_request(http_server* httpsvr, char* url, request_handle cb, void* arg);
