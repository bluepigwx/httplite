#pragma once


//=============================================================
// 服务器核心结构所使用的消息队列结构
typedef int (*event_callback)(int fd, void* arg);

 struct svr_event_t  {
	int fd;
	event_callback event_callback;

	struct server_t* svr;
};


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back);
void svr_delete_event(server_t* server, svr_event_t* ev);
int svr_event_add(svr_event_t* svr_event);
int svr_event_del(svr_event_t* svr_event);

int svr_new_listener(server_t* server, int port, event_callback call_back);


int svr_init();
void svr_close(server_t* svr);
int svr_run(server_t* svr);


