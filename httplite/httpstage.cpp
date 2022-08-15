#include "httpstage.h"
#include "buff.h"



// ����request����
static int http_parse_req_line(http_request* req, char* szbuff, int len)
{
	return 0;
}


int http_stage_prepare(http_connection* conn, http_request* req)
{
	static char szbuff[1024];

	int ret = buff_read_line(conn->inputbuffer, szbuff, sizeof(szbuff));
	if (ret < 0)
	{	
		httpsvr_close_connection(conn, 0);
		return -1;
	}
	// ������
	if (ret == 0)
	{
		return 0;
	}
	// �յ�һ������
	ret = http_parse_req_line(req, szbuff, sizeof(szbuff));
	if (ret < 0)
	{
		httpsvr_close_connection(conn, 0);
		return -1;
	}

	conn->stage = HTTP_STAGE_READING_HEADER;

	http_stage_header(conn, req);

	return 0;
}


int http_stage_header(http_connection* conn, http_request* req)
{
	return 0;
}


int http_stage_body(http_connection* conn, http_request* req)
{
	return 0;
}