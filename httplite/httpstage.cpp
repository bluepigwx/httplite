#include "httpstage.h"




int http_stage_prepare(http_connection* conn, http_request* req)
{
	DATA_READ_STATE ret = DATA_STATE_OK;

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