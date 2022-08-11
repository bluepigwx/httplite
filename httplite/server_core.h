#pragma once

#include "queue.h"

// 事件队列类型，按位与关系，因为一个文件描述符可以同时处于监听状态和活动状态
#define SVR_EV_QUEUE_WAIT	0x1		// 监听队列
#define SVR_EV_QUEUE_ACTIVE	0x2		// 事件发生的活动队列


//=============================================================
// 事件回调接口规范
typedef int (*event_callback)(int fd, void* arg);

struct svr_event_t {
	// 监听队列中的存储
	DLIST_ENTRY(svr_event_t, entry);
	// 活动队列中的存储
	DLIST_ENTRY(svr_event_t, actentry);
	// 事件本身与回调处理
	int fd;
	event_callback event_callback;
	// 回调函数的参数
	void* arg;
	// server对象
	struct server_t* svr;
	// 所处队列类型，参考SVR_EV_QUEUE_*
	int nqueue;
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
	const char* name;	// 后端的自定义名字

	init func_init;
	add func_add;
	del func_del;
	dispatch func_dispatch;
	finit func_finit;
};


 //=============================================================
 // 服务器核心结构
 struct server_t {
	// IO后端对象接口与私有数据
	 struct svr_backend_t* backend;
	 void* backend_data;
	 // 文件事件监听队列
	 DLIST_HEAD(svr_event_t) ev_head;
	 // 事件已发生的活动队列
	 DLIST_HEAD(svr_event_t) act_head;
	 // 控制是否退出主循环
	 int loop_break;
};


// 分配，销毁事件
svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back, void* arg);
void svr_delete_event(server_t* server, svr_event_t* ev);

// 事件的加入，踢出队列
int svr_event_add(svr_event_t* svr_event, int nqueue);
int svr_event_del(svr_event_t* svr_event, int nqueue);
// 事件的激活
int svr_event_active(svr_event_t* svr_event);

// 开启server的监听端口
int svr_new_listener(server_t* server, int port, event_callback call_back, void* arg);

// 服务核心的生存期控制
server_t* svr_new(svr_backend_t* backend);
void svr_close(server_t* svr);
int svr_run(server_t* svr);


