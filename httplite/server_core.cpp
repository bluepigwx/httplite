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


static client_t* svr_get_client(int clientfd)
{
	for (int i = 0; i < client_sets.cnt; ++i)
	{
		if (client_sets.clients[i].client_fd == clientfd)
		{
			return &client_sets.clients[i];
		}
	}

	return NULL;
}


static stream_t* svr_get_fd_stream(int fd)
{
	client_t* client = svr_get_client(fd);
	if (client == NULL)
	{
		return NULL;
	}

	return &client->stream;
}


static void svr_on_close(int clientfd)
{
	// 从业务层移除
	for (int i = 0; i < client_sets.cnt; ++i)
	{
		if (client_sets.clients[i].client_fd == clientfd)
		{
			client_sets.clients[i] = client_sets.clients[client_sets.cnt - 1];
			--client_sets.cnt;

			break;
		}
	}
	// 从网络层移除
	net_close_client(&backend, clientfd);
}

static int svr_on_accept(int clientfd)
{
	if (clientfd <= 0)
	{
		return -1;
	}

	if (client_sets.cnt >= MAX_CLENT_CNT)
	{
		return -1;
	}

	client_t* client = &client_sets.clients[client_sets.cnt];
	memset(client, 0, sizeof(client_t));
	client->client_fd = clientfd;

	client->stream.cap = MAX_BUFF_LEN;
	client->stream.len = 0;

	++client_sets.cnt;

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


int svr_run(server_t* svr)
{
	return net_loop(&backend);
}

void svr_close(server_t* svr)
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
	backend.close_cb = svr_on_close;
	backend.err_cb = svr_on_close;
	backend.get_stream_cb = svr_get_fd_stream;
	backend.process_cb = svr_process_packet;

	return net_listen_port(&backend, HTTPSVR_PORT);
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


int svr_event_add(svr_event_t* svr_event)
{
	return 0;
}


int svr_event_del(svr_event_t* svr_event)
{
	return 0;
}