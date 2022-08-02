#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "select_backend.h"
#include "buff_stream.h"
#include "server_core.h"
#include "server_internal.h"


#define HTTPSVR_PORT 8899

#define MAX_CLENT_CNT 63

#define BUFF_LEN_256 256



/*
static int svr_file_not_found(client_t* client)
{
	char szResponse[1024] = {0};

	strcat_s(szResponse, "HTTP/1.0 404 NOT FOUND\r\n");
	strcat_s(szResponse, "Server: jdbhttpd/0.1.0\r\n");
	strcat_s(szResponse, "Content-Type: text/html\r\n");
	strcat_s(szResponse, "\r\n");
	strcat_s(szResponse, "<HTML><TITLE>Not Found</TITLE>\r\n");
	strcat_s(szResponse, "<BODY><P>The server could not fulfill\r\n");
	strcat_s(szResponse, "your request because the resource specified\r\n");
	strcat_s(szResponse, "is unavailable or nonexistent.\r\n");
	strcat_s(szResponse, "</BODY></HTML>\r\n");

	net_send_package(client->client_fd, szResponse, (int)strlen(szResponse));

	return 0;
}


static int svr_send_header(client_t* client)
{
	char szResponse[1024] = { 0 };

	strcat_s(szResponse, "HTTP/1.0 200 OK\r\n");
	strcat_s(szResponse, "Server: jdbhttpd/0.1.0\r\n");
	strcat_s(szResponse, "Content-Type: text/html\r\n");
	strcat_s(szResponse, "\r\n");

	net_send_package(client->client_fd, szResponse, (int)strlen(szResponse));

	return 0;
}


static int svr_response_file(client_t* client)
{
	char buff[1024];
	sprintf_s(buff, "htdocs/%s", client->url);
	FILE* file = NULL;
	int ret = fopen_s(&file, buff, "r");
	if (file == NULL)
	{
		svr_file_not_found(client);
		return -1;
	}

	svr_send_header(client);

	fgets(buff, sizeof(buff), file);
	while (!feof(file))
	{
		net_send_package(client->client_fd, buff, (int)strlen(buff));
		fgets(buff, sizeof(buff), file);
	}

	fclose(file);
	
	return 0;
}


static int svr_process_packet(int fd, stream_t* stream, bool is_overflow)
{
	if (is_overflow == true)
	{
		svr_on_close(fd);
		return -1;
	}

	client_t* client = svr_get_client(fd);
	if (client == NULL)
	{
		svr_on_close(fd);
		return -1;
	}

	int index = svr_parse_method(stream->buff, stream->len, client->method, sizeof(client->method));
	if (index < 0)
	{
		svr_on_close(fd);
		return -1;
	}

	index += svr_parse_url(&stream->buff[index], stream->len - index, client->url, sizeof(client->url));
	if (strcmp(client->method, "GET") == 0)
	{
		svr_response_file(client);
	}

	stream->len = 0;

	return 0;
}

*/


int svr_run(server_t* svr)
{
	while (1)
	{
		if (svr->loop_break)
		{
			break;
		}

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		svr->backend->func_dispatch(svr, &tv);

		// 处理活动队列
	}

	return 0;
}


server_t* svr_new(svr_backend_t* backend)
{
	server_t* svr = (server_t*)calloc(1, sizeof(server_t));
	return svr;
}


void svr_close(server_t* svr)
{
	svr->backend->func_finit(svr);
	free(svr);
}


svr_event_t* svr_new_event(server_t* server, int fd, event_callback call_back)
{
	svr_event_t* ev = (svr_event_t*)calloc(1, sizeof(svr_event_t));
	if (ev == nullptr)
	{
		return nullptr;
	}

	ev->svr = server;
	ev->fd = fd;
	ev->event_callback = call_back;

	return ev;
}


void svr_delete_event(server_t* server, svr_event_t* ev)
{
	free(ev);
}


int svr_new_listener(server_t* server, int port, event_callback call_back)
{
	int svrfd = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (svrfd < 0)
	{
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = 0;
	svr_event_t* ev = nullptr;
	do 
	{
		ret = bind(svrfd, (struct sockaddr*)&addr, sizeof(addr));
		if (ret < 0)
		{
			break;
		}

		ret = listen(svrfd, 20);
		if (ret < 0)
		{
			break;
		}

		ev = svr_new_event(server, svrfd, call_back);
		if (ev == nullptr)
		{
			break;
		}

		ret = svr_event_add(ev);
		if (ret < 0)
		{
			break;
		}
	} while (0);

	if (ret < 0 || ev == nullptr)
	{
		closesocket(svrfd);
		if (ev)
		{
			svr_delete_event(server, ev);
		}

		return -1;
	}
	
	return 0;
}


int svr_event_active(server_t* server, svr_event_t* ev)
{
	return 0;
}


int svr_event_add(svr_event_t* svr_event)
{
	return 0;
}


int svr_event_del(svr_event_t* svr_event)
{
	return 0;
}