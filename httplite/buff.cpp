#include <stdlib.h>
#include <string.h>
#include "buff.h"


// 默认分配buff长队
#define DEFAULT_BUFF_LEN 1024



szbuff* buff_new()
{
	szbuff* buff = (szbuff*)calloc(1, sizeof(szbuff));
	if (nullptr == buff)
	{
		return nullptr;
	}

	buff->org = (char*)malloc(DEFAULT_BUFF_LEN);
	if (buff->org == nullptr)
	{
		free(buff);
		return nullptr;
	}

	buff->buff = buff->org;
	buff->total = DEFAULT_BUFF_LEN;

	return buff;
}

void buff_delete(szbuff* buff)
{
	if (buff->org)
	{
		free(buff->org);
	}

	free(buff);
}

// 将buff扩展到maxlen
int buff_expand(szbuff* buff, int maxlen)
{
	return 0;
}

// 将buff前部len长度的数据置为无效，从而避免每次读取都要移动内存
int buff_drain(szbuff* buff, int drainlen)
{	
	int len = drainlen;
	// 一把读完了，直接归零，相当于这个buff可用内存已经耗尽，下次无论如何也要expand了
	if (drainlen > buff->off)
	{
		len = buff->off;
		buff->buff = buff->org;
		buff->align = buff->off = 0;
		return len;
	}

	buff->buff += len;
	buff->align += len;
	buff->off -= len;

	return len;
}

// 从buff中读取len长度的数据到data中并前移数据指针
int buff_add_data(szbuff* buff, char* indata, int len)
{
	// 计算用掉了多少
	int used = buff->align + buff->off;
	if (buff->total - used < len)
	{
		// 容量不够先扩容
		if (buff_expand(buff, len) != 0)
		{
			return -1;
		}
	}
	// 拷贝数据
	char* begin = buff->buff + buff->off;
	memcpy(begin, indata, len);
	buff->off += len;

	return 0;
}

// 将data长度的数据从buff中取出
int buff_get_data(szbuff* buff, char* outdata, int len)
{
	int getlen = len;
	if (len > buff->off)
	{
		getlen = buff->off;
	}

	memcpy(outdata, buff->buff, getlen);
	buff_drain(buff, getlen);

	return getlen;
}

// 将一个fd的内容读入到buff中
int buff_read_fd(szbuff* buff, int fd, int len)
{
	return 0;
}