#pragma once


//=============================================================
// 服务器核心结构所使用的消息队列结构
typedef int (*event_callback)(int fd, void* arg);

struct svr_event_t {
	int fd;
	event_callback event_callback;

	struct server_t* svr;
};

//=============================================================
// 多路IO复用后端

// 后端初始化接口
typedef void* (*init)(struct server_t* svr);
// 将一个文件描述符添加到后端驱动中
typedef int (*add)(struct server_t* svr, svr_event_t* ev);
// 从后端驱动中删除一个文件描述符
typedef void (*del)(struct server_t* svr, svr_event_t* ev);
// 后端驱动查看哪些文件描述符有所变动
typedef int (*dispatch)(struct server_t* svr, struct timeval* tm);
// 反初始化后端驱动
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
 // 服务器核心结构
 struct server_t {
	 struct svr_backend_t* backend;
	 void* backend_data;

	 // 文件事件列表
	 svr_event_t* event_head;
	 // 控制是否退出主循环
	 int loop_break;
 };


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back);
void svr_delete_event(server_t* server, svr_event_t* ev);
int svr_event_add(svr_event_t* svr_event);
int svr_event_del(svr_event_t* svr_event);

int svr_new_listener(server_t* server, int port, event_callback call_back);

// 放入活动队列
int svr_event_active(server_t* server, svr_event_t* ev);


server_t* svr_new(svr_backend_t* backend);
void svr_close(server_t* svr);
int svr_run(server_t* svr);


