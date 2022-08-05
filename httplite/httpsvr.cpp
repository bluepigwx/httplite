#include <WinSock2.h>
#include "httpsvr.h"
#include "server_core.h"
#include "select_backend.h"


static int on_read_callback(int fd, void* arg)
{
	int len = 0;
	stream_t stream;
	memset(&stream, 0, sizeof(stream_t));
	stream.cap = sizeof(stream.buff);

	int n = 0;
	bool overflow = false;
	do
	{
		if (stream.len >= stream.cap)
		{
			overflow = true;
			break;
		}

		char* begin = stream.buff + stream.len;
		int size = stream.cap - stream.len;
		n = recv(fd, begin, size, 0);
		if (n > 0)
		{
			stream.len += n;
		}

	} while (n > 0 && overflow == false);

	if (n == 0)
	{
		// gracefully closed
		//backend->close_cb(fd);
	}
	else if (n < 0 && ::WSAGetLastError() != WSAEWOULDBLOCK)
	{
		// some error
		//backend->err_cb(fd);
	}
	else
	{
		// process
		//backend->process_cb(fd, stream, overflow);
	}

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

	unsigned long mod = 1;
	int ret = ioctlsocket(client_fd, FIONBIO, &mod);

	// 加入到等待事件队列
	svr_event_add(new_ev, SVR_EV_QUEUE_WAIT);

	return 0;
}


server_t* server_core = nullptr;

int httpsvr_start(int port)
{
	svr_backend_t* bk = get_select_backend();
	server_core = svr_new(bk);
	if (server_core == nullptr)
	{
		return -1;
	}

	int ret = svr_new_listener(server_core, port, on_accept);
	return ret;
}

void httpsvr_stop()
{
	svr_close(server_core);
}

int httpsvr_run()
{
	return svr_run(server_core);
}