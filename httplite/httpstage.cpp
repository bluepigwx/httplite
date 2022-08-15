#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "httpstage.h"
#include "buff.h"

static char* _strsep(char** ss, const char* del);



static char* _strsep(char** ss, const char* del)
{
	return nullptr;
}

// 解析request请求，填充request结构
static int http_parse_req_line(http_request* req, char* szbuff, int len)
{
	char* method = _strsep(&szbuff, " ");
	if (method == nullptr)
	{
		return -1;
	}

	char* url = _strsep(&szbuff, " ");
	if (url == nullptr)
	{
		return -1;
	}

	char* version = _strsep(&szbuff, " ");
	if (version == nullptr)
	{
		return -1;
	}
	
	return 0;
}


int http_stage_prepare(http_connection* conn, http_request* req)
{
	static char szbuff[1024];

	int ret = buff_read_line(conn->inputbuffer, szbuff, sizeof(szbuff));
	if (ret < 0)
	{	
		// 报错了
		httpsvr_close_connection(conn, 0);
		return -1;
	}
	// 接着收
	if (ret == 0)
	{
		return 0;
	}
	//else 收到一整行了

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