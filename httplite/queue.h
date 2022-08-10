#pragma once


//=============================================================
// ˫������
// ����˫������ͷ���������˷�һ��preָ��Ŀռ�
#define DLIST_HEAD(type)	\
struct {	\
	struct type* pre;		\
	struct type* next;		\
}

// ����˫������Ԫ��
#define DLIST_ENTRY(type)	\
struct {	\
	type* pre;		\
	type* next;		\
}


// ���뵽����ͷǰ
#define DLIST_INSERT_HEAD(head, item) \
do {			\
		if ((*head) == nullptr) \
			((*head) = item)	\
		else                    \
		{						\
			(item)->next = (*head);	\
			(item)->pre = nullptr;	\
			(*head)->pre = item;	\
			(*head) = item;			\
		}						\
		if ((head)->next)	\
			

}while(0);


// �������Ƴ�
#define DLIST_REMOVE(head, item) \
do {			\
		if 	((*head) == (item))		\
			(*head) = nullptr;		\
		else                        \
		{							\
			if ((item)->next)		\
				(item)->next->pre = (item)->pre;	\
			if ((item)->pre)		\
				(item)->pre->next = (item)->next;	\
		}							\
}while(0);