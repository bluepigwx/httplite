#pragma once

#include "queue.h"

// �¼��������ͣ���λ���ϵ����Ϊһ���ļ�����������ͬʱ���ڼ���״̬�ͻ״̬
#define SVR_EV_QUEUE_WAIT	0x1		// ��������
#define SVR_EV_QUEUE_ACTIVE	0x2		// �¼������Ļ����


//=============================================================
// �¼��ص��ӿڹ淶
typedef int (*event_callback)(int fd, void* arg);

struct svr_event_t {
	// ���������еĴ洢
	DLIST_ENTRY(svr_event_t, entry);
	// ������еĴ洢
	DLIST_ENTRY(svr_event_t, actentry);
	// �¼�������ص�����
	int fd;
	event_callback event_callback;
	// �ص������Ĳ���
	void* arg;
	// server����
	struct server_t* svr;
	// �����������ͣ��ο�SVR_EV_QUEUE_*
	int nqueue;
};


//=============================================================
// ��·IO���ú��

// ��˳�ʼ���ӿ�
typedef void* (*init)(struct server_t* svr);
// ��һ���ļ���������ӵ����������
typedef int (*add)(struct server_t* svr, svr_event_t* ev);
// �Ӻ��������ɾ��һ���ļ�������
typedef void (*del)(struct server_t* svr, svr_event_t* ev);
// ��������鿴��Щ�ļ������������䶯
typedef int (*dispatch)(struct server_t* svr, struct timeval* tm);
// ����ʼ���������
typedef void (*finit)(struct server_t* svr);

struct svr_backend_t {
	const char* name;	// ��˵��Զ�������

	init func_init;
	add func_add;
	del func_del;
	dispatch func_dispatch;
	finit func_finit;
};


 //=============================================================
 // ���������Ľṹ
 struct server_t {
	// IO��˶���ӿ���˽������
	 struct svr_backend_t* backend;
	 void* backend_data;
	 // �ļ��¼���������
	 DLIST_HEAD(svr_event_t) ev_head;
	 // �¼��ѷ����Ļ����
	 DLIST_HEAD(svr_event_t) act_head;
	 // �����Ƿ��˳���ѭ��
	 int loop_break;
};


// ���䣬�����¼�
svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back, void* arg);
void svr_delete_event(server_t* server, svr_event_t* ev);

// �¼��ļ��룬�߳�����
int svr_event_add(svr_event_t* svr_event, int nqueue);
int svr_event_del(svr_event_t* svr_event, int nqueue);
// �¼��ļ���
int svr_event_active(svr_event_t* svr_event);

// ����server�ļ����˿�
int svr_new_listener(server_t* server, int port, event_callback call_back, void* arg);

// ������ĵ������ڿ���
server_t* svr_new(svr_backend_t* backend);
void svr_close(server_t* svr);
int svr_run(server_t* svr);


