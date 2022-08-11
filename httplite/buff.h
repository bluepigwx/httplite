#pragma once

/*
* ����ʼ��ʱbuff��ַ==org��ַ��off==align
* �ڴ��Ϊ������ 1��ore+align���Ѿ���������
*				2��buff+off���Ѿ����벿��
*				3��buff+offֱ��totalĩβ����д�벿��
* buff�ڴ�ṹ��������һ���Ƚ��ȳ����У����������ݽ���ʱ������offƫ�����������ڶ���������д���˶����ڴ�
* ����ͷ��ȡ������ʱ������buff��alignƫ�ƣ���֤buffָ��ÿ��ָ������ڴ����ʼ��ַ����alignƫ������ָ���˴�org��ʼ�����˶���ƫ�Ƶ��ڴ棬
* �����¶����ڴ�֮ǰ����org��ʼ��align֮����ڴ�Ϊ�Ѷ����Ҳ�����״̬����buff��ʼ��offֱ��Ϊ�Ѿ�д��״̬����off+alignƫ�Ƶ�total֮����ڴ�Ϊ���п�д��״̬
* |----------------------------------------------|
* ^org      align^^buff				^off		 ^ total
* 
* org��ԭʼ��ַ
* buff��ʵ���Ѿ�ʹ���ڴ���׵�ַ
* off����buff��ַ��ʼ����ʵ��ʹ���˶����ڴ��ƫ��
* align����org��ַ��ʼ����ʵ���ų����ڴ�ƫ��
*/

struct szbuff {
	// �ڴ�ά��
	char* org;
	char* buff;
	// ʹ����ά��
	int total;
	int off;
	int align;
};


szbuff* buff_new();
void buff_delete(szbuff* buff);

// ��buff��չ��maxlen
int buff_expand(szbuff* buff, int maxlen);
// ��buffǰ��len���ȵ�������Ϊ��Ч���Ӷ�����ÿ�ζ�ȡ��Ҫ�ƶ��ڴ�
int buff_drain(szbuff* buff, int drainlen);
// ����д���ڴ����¹�λ��orgλ��
void buff_realign(szbuff* buff);
// ��data���ݷ��뵽buff��
int buff_add_data(szbuff* buff, const char* indata, int len);
// ��data���ȵ����ݴ�buff��ȡ��
int buff_get_data(szbuff* buff, char* outdata, int len);
// ��һ��fd�����ݶ��뵽buff��
int buff_read_fd(szbuff* buff, int fd, int len);

