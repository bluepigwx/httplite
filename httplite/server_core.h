#pragma once


//=============================================================
// 多路IO复用后端
typedef void* (*init)(struct server_t* svr);
typedef int (*add)(struct server_t* svr, int fd);
typedef void (*del)(struct server_t* svr, int fd);
typedef int (*dispatch)(struct server_t* svr, struct timeval* tm);
typedef void (*finit)(struct server_t* svr);

struct svr_backend_t {
	const char* name;

	init func_init;
	add func_add;
	del func_del;
	dispatch func_dispatch;
	finit func_finit;
};

//=============================================================
// 服务器核心结构所使用的消息队列结构
typedef int (*event_callback)(int fd, void* arg);

 struct svr_event_t  {
	int fd;
	event_callback event_callback;

	struct server_t* svr;
};


 //=============================================================
 // 服务器核心结构
 struct server_t {
	 struct svr_backend_t* backend;
	 void* backend_data;

	 struct svr_event_t* event_head;

	 int loop_break;
 };


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back);
void svr_delete_event(server_t* server, svr_event_t* ev);
int svr_event_add(svr_event_t* svr_event);
int svr_event_del(svr_event_t* svr_event);

int svr_new_listener(server_t* server, int port, event_callback call_back);


int svr_init();
void svr_close(server_t* svr);
int svr_run(server_t* svr);


