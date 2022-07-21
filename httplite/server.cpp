#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "net.h"
#include "server.h"

#define HTTPSVR_PORT 8899

int svr_init()
{
	int ret = net_init();
	if (ret < 0)
	{
		return -1;
	}

	ret = net_listen_port(HTTPSVR_PORT);

	return ret;
}


void svr_close()
{
	net_close();
}


static int svr_parse_method(client_t* client)
{
	int i = 0;
	int j = 0;
	// �������ո�
	while (true)
	{
		if (isspace(client->buff[j]))
		{
			++j;
			continue;
		}
		else
		{
			// ֱ����һ����Ч�ַ�
			break;
		}
	}

	// ֱ����һ���հ��ַ�֮ǰ����method
	while (!isspace(client->buff[j]))
	{
		if (i >= sizeof(client->method))
		{
			break;
		}

		client->method[i] = client->buff[j];
		++i;
		++j;
	}

	// �����հ׷�
	while (isspace(client->buff[j]))
		++j;

	i = 0;
	// ֱ����һ���հ��ַ�֮ǰ����url
	while (!isspace(client->buff[j]))
	{
		if (i >= sizeof(client->method))
		{
			break;
		}

		client->url[i] = client->buff[j];
		++i;
		++j;
	}

	return 0;
}


int svr_run()
{
	while (true)
	{
		client_t client;
		memset(&client, 0, sizeof(client));

		client.client_fd = net_onconnect();

		int ret = net_read_package(client.client_fd, client.buff, sizeof(client.buff));
		if (ret < 0)
		{
			break;
		}

		svr_parse_method(&client);

		net_close_client(client.client_fd);
	}

	return 0;
}