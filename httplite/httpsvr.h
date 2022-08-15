#pragma once

#include "queue.h"

/*
* httpsvr->conn_head
*				|
*				conn<-->conn<-->conn....
*				|
*				request->request->request....
* 服务器是一个网络连接的集合，这里使用双向链表数据结构进行管理
* 而连接则是一个请求request的队列，这里使用FIFO数据结构进行管理
* 每个request则是一个收取tcp数据流的状态机
*/


//=============================================================
// 链接对象上的请求对象

// 请求方法
#define HTTP_METHOD_REQ_GET		1
#define HTTP_METHOD_REQ_POST	2

struct http_request {
	// 内存管理
	QUEUE_ENTRY(http_request, qentry);
	// 请求的路径
	char* url;
	// 所依附的连接对象
	struct http_connection* conn;
	// 处理方法，参见HTTP_METHOD_**
	unsigned char method;
};


//=============================================================
// http服务器的连接对象
// HTTP_STAGE_READING_PREPARE -> HTTP_STAGE_READING_HEADER -> HTTP_STAGE_READING_BODY

// 处理阶段
#define HTTP_STAGE_READING_PREPARE	1	// 数据准备阶段
#define HTTP_STAGE_READING_HEADER	2	// 读取http头
#define HTTP_STAGE_READING_BODY		3	// 请求体

struct http_connection {
	// 内存管理
	DLIST_ENTRY(http_connection, entry);
	// 连接对象
	int fd;
	// fd事件
	struct svr_event_t* ev;
	// 读缓冲
	struct szbuff* inputbuffer;
	// 在此连接上的所有请求队列
	QUEUE_HEAD(http_request) reqhead;
	// 当前处理阶段，参见HTTP_REQ_STAGE_***
	unsigned char stage;

	struct http_server* httpsvr;
};


//=============================================================
// request处理
typedef int (*request_handle)(struct http_request* req, void* arg);

// http服务器的handle对象
struct http_request_handle {
	// 内存管理
	DLIST_ENTRY(http_request_handle, entry);
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
	// 请求回调管理
	DLIST_HEAD(http_request_handle) req_cb_head;
	// 服务器连接管理
	DLIST_HEAD(http_connection) conn_head;
	// 服务器事件处理核心
	struct server_t* svrcore;
};


http_server* httpsvr_new(server_t* svrcore, int port);
void httpsvr_free(http_server* httpsvr);
int httpsvr_run(http_server* httpsvr);

// 注册url处理接口
int httpsvr_register_request(http_server* httpsvr, char* url, request_handle cb, void* arg);
// 关闭连接
void httpsvr_close_connection(http_connection* conn, int reason);
