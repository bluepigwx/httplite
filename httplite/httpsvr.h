#pragma once


//=============================================================
// 链接对象上的请求对象
struct http_request {
	// 内存管理
	http_request* pre;
	http_request* next;
	// 请求的路径
	char* url;
	// 所依附的连接对象
	struct http_connection* conn;
};


//=============================================================
// http服务器的连接对象
struct http_connection {
	// 内存管理
	http_connection* pre;
	http_connection* next;
	// 连接对象
	int fd;
	// fd事件
	struct svr_event_t* ev;
	// 读缓冲
	struct szbuff* inputbuffer;
	// 在此连接上的所有请求列表
	http_request* reqhead;

	struct http_server* httpsvr;
};


//=============================================================
// request处理
typedef int (*request_handle)(struct http_request* req, void* arg);

// http服务器的handle对象
struct http_request_handle {
	// 内存管理
	http_request_handle* pre;
	http_request_handle* next;
	// 请求的路径
	char* url;
	// 请求的回调处理
	request_handle cb;
	// 回调参数
	void* arg;
};


//=============================================================
// http服务器的定义
// 连接来了放conn_head链表，连接自己的request来了放request_cb_head链表
struct http_server {
	// request回调处理的链表头
	http_request_handle* requect_cb_head;
	// 服务器连接管理
	http_connection* conn_head;
	// 服务器事件处理核心
	struct server_t* svrcore;
};


http_server* httpsvr_new(server_t* svrcore, int port);
void httpsvr_free(http_server* httpsvr);
int httpsvr_run(http_server* httpsvr);

// 注册url处理接口
int httpsvr_register_request(http_server* httpsvr, char* url, request_handle cb, void* arg);
