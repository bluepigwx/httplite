#include <WinSock2.h>
#include "net.h"


int http_socket = -1;

int net_init()
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return -1;
	}

	http_socket = -1;

	return 0;
}


void net_close()
{
	if (http_socket > 0)
	{
		closesocket(http_socket);
	}

	WSACleanup();

	return;
}


void net_close_client(int client_fd)
{
	closesocket(client_fd);
}

int net_listen_port(int port)
{
	http_socket = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (http_socket < 0)
	{
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = bind(http_socket, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{	
		net_close();
		return -1;
	}

	ret = listen(http_socket, 20);
	if (ret < 0)
	{
		net_close();
		return -1;
	}

	return 0;
}


int net_onconnect()
{
	if (http_socket < 0)
	{
		return -1;
	}

	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	int client_fd = (int)accept(http_socket, (struct sockaddr*)&client_addr, &addr_len);
	return client_fd;
}


int net_read_package(int fd, char* buff, int len)
{
	int n = recv(fd, buff, len, 0);
	if (n <= 0)
	{
		return-1;
	}

	return n;
}