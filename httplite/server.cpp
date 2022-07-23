#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "net.h"
#include "buff_stream.h"
#include "server.h"

#define HTTPSVR_PORT 8899

#define MAX_CLENT_CNT 63



#define BUFF_LEN_256 256

typedef struct {
	int client_fd;
	stream_t stream;
	char method[BUFF_LEN_256];
	char url[BUFF_LEN_256];
} client_t;


typedef struct {
	client_t clients[MAX_CLENT_CNT];
	int cnt;
} client_set_t;


net_backend_t backend;

client_set_t client_sets;


// GET or POST
static int svr_parse_method(const char* inbuff, int inlen, char* outbuff, int outlen)
{
	int i = 0;
	int j = 0;
	// 先跳过空格
	while (true)
	{
		if (isspace(inbuff[j]))
		{
			++j;
			continue;
		}
		else
		{
			// 直到第一个有效字符
			break;
		}
	}

	// 直到第一个空白字符之前都是method
	while (!isspace(inbuff[j]))
	{
		if (i >= sizeof(outbuff))
		{
			break;
		}

		outbuff[i] = inbuff[j];
		++i;
		++j;
	}

	return j;
}


static int svr_parse_url(const char* inbuff, int inlen, char* outbuff, int outlen)
{
	// 跳过空白符
	int j = 0;
	int i = 0;
	while (isspace(inbuff[j]))
		++j;

	// 直到第一个空白字符之前都是url
	while (!isspace(inbuff[j]))
	{
		if (i >= outlen)
		{
			break;
		}

		outbuff[i] = inbuff[j];
		++i;
		++j;
	}

	return 0;
}


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


static int svr_parse_http(client_t* client)
{
	// GET or POST
	int index = svr_parse_method(client->buff, sizeof(client->buff), client->method, sizeof(client->method));
	if (index < 0)
	{
		return -1;
	}
	// URL
	index += svr_parse_url(&client->buff[index], sizeof(client->buff) - index, client->url, sizeof(client->url));

	if (strcmp(client->method, "GET") == 0)
	{
		svr_response_file(client);
	}

	return 0;
}

static int svr_on_accept(int clientfd)
{
	if (client_sets.cnt >= MAX_CLENT_CNT)
	{
		return -1;
	}

	client_t* client = &client_sets.clients[client_sets.cnt];
	memset(client, 0, sizeof(client_t));
	client->client_fd = clientfd;
	client->buff.cap = MAX_BUFF_LEN;
	client->buff.len = 0;

	++client_sets.cnt;

	return 0;
}

int svr_run()
{
	return net_loop(&backend);
}

void svr_close()
{
	net_close(&backend);
}

int svr_init()
{
	memset(&backend, 0, sizeof(backend));
	int ret = net_init(&backend);
	if (ret < 0)
	{
		return -1;
	}

	backend.accept_cb = svr_on_accept;

	return net_listen_port(&backend, HTTPSVR_PORT);
}