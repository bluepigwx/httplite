#pragma once

/*
* 当初始化时buff地址==org地址，off==align
* 内存分为三部分 1、ore+align：已经读出部分
*				2、buff+off：已经读入部分
*				3、buff+off直到total末尾：可写入部分
* buff内存结构本质上是一个先进先出队列，当有新数据进入时，调整off偏移量，表名在队列中我们写入了多少内存
* 当从头部取出数据时，调整buff和align偏移，保证buff指针每次指向可用内存的起始地址，而align偏移量则指明了从org开始读出了多少偏移的内存，
* 在重新对齐内存之前，从org开始到align之间的内存为已读出且不可用状态，从buff开始到off直接为已经写入状态，从off+align偏移到total之间的内存为空闲可写入状态
* |----------------------------------------------|
* ^org      align^^buff				^off		 ^ total
* 
* org：原始地址
* buff：实际已经使用内存的首地址
* off：从buff地址开始算起，实际使用了多少内存的偏移
* align：从org地址开始算起，实际排出的内存偏移
*/

struct szbuff {
	// 内存维护
	char* org;
	char* buff;
	// 使用量维护
	int total;
	int off;
	int align;
};


szbuff* buff_new();
void buff_delete(szbuff* buff);

// 将buff扩展到maxlen
int buff_expand(szbuff* buff, int maxlen);
// 将buff前部len长度的数据置为无效，从而避免每次读取都要移动内存
int buff_drain(szbuff* buff, int drainlen);
// 将已写入内存重新归位到org位置
void buff_realign(szbuff* buff);
// 将data数据放入到buff中
int buff_add_data(szbuff* buff, const char* indata, int len);
// 将data长度的数据从buff中取出
int buff_get_data(szbuff* buff, char* outdata, int len);
// 将一个fd的内容读入到buff中
int buff_read_fd(szbuff* buff, int fd, int len);

