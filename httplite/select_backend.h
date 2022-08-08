#pragma once

#include <WinSock2.h>


int net_send_package(int fd, char* buff, int len);

struct svr_backend_t* get_select_backend();
