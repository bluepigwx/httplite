#include "string.h"
#include "test.h"
#include "buff.h"


int test_buff_add_data(szbuff* buff)
{
	char sz[] = "hello world";
	buff_add_data(buff, sz, sizeof(sz));

	return 0;
}

int test_buff_get_data(szbuff* buff)
{
	char sz[1024] = {0};
	buff_get_data(buff, sz, sizeof(sz));
	return 0;
}


int test_start()
{
	szbuff* buff = buff_new();

	test_buff_add_data(buff);

	test_buff_get_data(buff);

	buff_delete(buff);

	return 0;
}