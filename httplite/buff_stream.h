#pragma once


#define MAX_BUFF_LEN 10240

typedef struct {
	char buff[MAX_BUFF_LEN];
	unsigned short cap;
	unsigned short len;
} stream_t;

