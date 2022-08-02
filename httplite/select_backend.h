#pragma once

#include "buff_stream.h"
#include <WinSock2.h>


int net_send_package(int fd, char* buff, int len);
