#include <stdlib.h>
#include <string.h>
#include "buff.h"


// Ĭ�Ϸ���buff����
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

// ��buff�����ڴ���չ��maxlen
int buff_expand(szbuff* buff, int maxlen)
{
	// 1. off��ʼ��total��ָ���ڴ�ĩβ�����ڴ滹��װ��maxlen
	int used = buff->align + buff->off;
	if (buff->total - used >= maxlen)
	{
		return 0;
	}
	// 2. �Ѷ����ڴ�+����1.�Ŀ����ڴ滹��װ��maxlen��Ҳ������β���˵Ŀ����ڴ���װ��maxlen����ô�ں�һ��
	if (buff->total - buff->off > maxlen)
	{
		// ���¶���
		buff_realign(buff);
		return 0;
	}
	// 3. ��β�����ڴ�֮���Ѿ��޷�װ��maxlen����Ҫ���·����ڴ�

	return 0;
}

// ��buffǰ��len���ȵ�������Ϊ��Ч���Ӷ�����ÿ�ζ�ȡ��Ҫ�ƶ��ڴ�
int buff_drain(szbuff* buff, int drainlen)
{	
	int len = drainlen;
	// һ�Ѷ����ˣ�ֱ�ӹ��㣬�൱�����buff�����ڴ��Ѿ��ľ����´��������ҲҪexpand��
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

// ����д���ڴ����¹�λ��orgλ��
void buff_realign(szbuff* buff)
{
	memmove(buff->org, buff->buff, buff->off);
	buff->buff = buff->org;
	buff->align = 0;
}

// ��buff�ж�ȡlen���ȵ����ݵ�data�в�ǰ������ָ��
int buff_add_data(szbuff* buff, char* indata, int len)
{
	// �����õ��˶���
	int used = buff->align + buff->off;
	if (buff->total - used < len)
	{
		// ��������������
		if (buff_expand(buff, len) != 0)
		{
			return -1;
		}
	}
	// ��������
	char* begin = buff->buff + buff->off;
	memcpy(begin, indata, len);
	buff->off += len;

	return 0;
}

// ��data���ȵ����ݴ�buff��ȡ��
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

// ��һ��fd�����ݶ��뵽buff��
int buff_read_fd(szbuff* buff, int fd, int len)
{
	return 0;
}