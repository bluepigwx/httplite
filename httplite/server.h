#pragma once

#define MAX_BUFF_LEN 10240
#define BUFF_LEN_256 256

typedef struct {
	int client_fd;
	char buff[MAX_BUFF_LEN];
	char method[BUFF_LEN_256];
	char url[BUFF_LEN_256];
} client_t;


int svr_init();
void svr_close();
int svr_run();