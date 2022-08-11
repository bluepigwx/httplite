#pragma once


//=============================================================
// ˫������

// ����˫������Ԫ��
#define DLIST_HEAD(type) \
struct {	\
	type* first;	\
}

#define DLIST_ENTRY(type, name)	\
	type* name##pre;		\
	type* name##next;


// ���뵽����ͷǰ
#define DLIST_INSERT_HEAD(head, item, name) \
do {	\
		if ((head)->first)	\
			(head)->first->name##pre = (item);	\
		(head)->first->name##pre = (item);	\
		(item)->name##next = (head)->first;	\
		(head)->first = (item);			\
}while(0);

// ��ȡ��Ԫ��
#define DLIST_GET_FIRST(head)	\
	(head)->first;

// �������Ƴ�
#define DLIST_REMOVE(item, name) \
do {			\
	if ((item)->name##pre)	\
		(item)->name##pre->name##next = (item)->name##next;	\
	if ((item)->name##next)	\
		(item)->name##next->name##pre = (item)->name##pre;	\
}while(0);

// ��������
#define DLIST_FOREEACH(var, head, name) \
	for ((var) = (head)->first;	\
		(var) != nullptr;		\
		(var) = (var)->name##next)