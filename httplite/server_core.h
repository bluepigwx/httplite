#pragma once


//=============================================================
// ���������Ľṹ��ʹ�õ���Ϣ���нṹ
typedef int (*event_callback)(int fd, void* arg);

struct svr_event_t {
	// �洢���
	svr_event_t* pre;
	svr_event_t* next;
	// �¼�����
	int fd;
	event_callback event_callback;
	// server����
	struct server_t* svr;
	// ������������
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
	const char* name;

	init func_init;
	add func_add;
	del func_del;
	dispatch func_dispatch;
	finit func_finit;
};


 //=============================================================
 // ���������Ľṹ
 struct server_t {
	 struct svr_backend_t* backend;
	 void* backend_data;

	 // �ļ��¼��б�
	 svr_event_t* event_head;
	 // �����Ƿ��˳���ѭ��
	 int loop_break;

	 // �����
	 svr_event_t* active_evs;
 };


#define SVR_EV_QUEUE_WAIT	0x1		// �ȴ�����
#define SVR_EV_QUEUE_ACTIVE	0x2		// �¼������Ļ����

svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back);
void svr_delete_event(server_t* server, svr_event_t* ev);
int svr_event_add(svr_event_t* svr_event, int nqueue);
int svr_event_del(svr_event_t* svr_event, int nqueue);

// ����server�ļ����˿�
int svr_new_listener(server_t* server, int port, event_callback call_back);


server_t* svr_new(svr_backend_t* backend);
void svr_close(server_t* svr);
int svr_run(server_t* svr);


