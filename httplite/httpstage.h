#pragma once

#include "httpsvr.h"


/*
* 数据接收分为三种状态
* 1、接收中
* 2、接收完成
* 3、数据损坏
*/
enum DATA_READ_STATE {
	DATA_STATE_OK,		// 收取完成
	DATA_STATE_BAD,		// 收取错误
	DATA_STATE_NEED_MORE,	// 还需继续收取
};



// 数据准备阶段
int http_stage_prepare(http_connection* conn, http_request* req);
// 解析请求头
int http_stage_header(http_connection* conn, http_request* req);
// 解析请求体
int http_stage_body(http_connection* conn, http_request* req);