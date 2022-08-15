#pragma once

#include "httpsvr.h"


/*
* ���ݽ��շ�Ϊ����״̬
* 1��������
* 2���������
* 3��������
*/
enum DATA_READ_STATE {
	DATA_STATE_OK,		// ��ȡ���
	DATA_STATE_BAD,		// ��ȡ����
	DATA_STATE_NEED_MORE,	// ���������ȡ
};



// ����׼���׶�
int http_stage_prepare(http_connection* conn, http_request* req);
// ��������ͷ
int http_stage_header(http_connection* conn, http_request* req);
// ����������
int http_stage_body(http_connection* conn, http_request* req);