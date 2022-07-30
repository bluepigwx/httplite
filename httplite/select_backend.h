#pragma once

#include "buff_stream.h"
#include <WinSock2.h>


typedef int (*accept_client_cb)(int fd);
typedef void (*close_client_cb)(int fd);
typedef void (*on_client_err_cb)(int fd);
typedef stream_t* (*get_fd_stream_cb)(int fd);
typedef int (*send_fd_buff_cb)(int fd, char* buff, int len);
typedef int (*process_packet_cb)(int fd, stream_t* stream, bool is_overflow);


typedef struct {
	int svrfd;
	fd_set fdsets;

	accept_client_cb accept_cb;
	close_client_cb close_cb;
	on_client_err_cb err_cb;
	get_fd_stream_cb get_stream_cb;
	process_packet_cb process_cb;
}net_backend_t;


int net_init(net_backend_t* backend);
void net_close(net_backend_t* backend);
void net_close_client(net_backend_t* backend, int client_fd);
int net_listen_port(net_backend_t* backend, int port);
int net_loop(net_backend_t* backend);

int net_send_package(int fd, char* buff, int len);
