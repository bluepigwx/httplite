#include <WinSock2.h>
#include "httpsvr.h"
#include "server_core.h"

static int on_read_callback(int fd, void* arg)
{
	return 0;
}

static int on_accept(int fd, void* arg)
{
	svr_event_t* ev = (svr_event_t*)arg;
	server_t* server = ev->svr;
	svr_backend_t* backend = server->backend;

	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	int client_fd = (int)accept(fd, (struct sockaddr*)&client_addr, &addr_len);

	svr_event_t* new_ev = svr_new_event(server, client_fd, on_read_callback);
	if (new_ev == nullptr)
	{
		closesocket(client_fd);
		return -1;
	}

	int ret = backend->func_add(server, new_ev);
	if (ret < 0)
	{
		svr_delete_event(server, new_ev);
		closesocket(client_fd);
		return -1;
	}

	unsigned long mod = 1;
	ret = ioctlsocket(client_fd, FIONBIO, &mod);

	// 加入到等待事件队列
	svr_event_add(new_ev);

	return 0;
}