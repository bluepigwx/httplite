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



//=============================================================
// �Ƚ��ȳ�����
// �������
#define QUEUE_HEAD(type)	\
struct {	\
	type* first;	\
	type** last;		\
}

#define QUEUE_ENTRY(type, name)	\
struct {	\
	type* name##next;	\
}


// ��ʼ��
#define QUEUE_INIT(head)	\
do {	\
	(head)->first = nullptr;	\
	(head)->last = &((head)->first);	\
} while (0)

// ����
#define QUEUE_POP(head, name)	\
do{	\
	if ((head)->first)	\
		(head)->first = (head)->first->name##next;	\
} while (0)

// ѹ��
#define QUEUE_PUSH(head, item, name)	\
do {	\
	(item)->name##next = nullptr;	\
	if (*(head)->last != nullptr)	\
		(head)->first->name##next = (item);	\
	(head)->last = &(item);	\
} while(0)

// �����Ԫ��
#define QUEUE_GET_FIRST(head)	\
	(head)->first;

// ��������
#define QUEUE_FOREACH(var, head, name)	\
	for ((var)=(head)->first;	\
		(var)!= nullptr;	\
		(var)=(var)->next)
		