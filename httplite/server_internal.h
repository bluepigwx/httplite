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
// 服务器核心结构
struct server_t {
	struct svr_backend_t* backend;
	void* backend_data;

	struct svr_event_t* event_head;

	int loop_break;
};
