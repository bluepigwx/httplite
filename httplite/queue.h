#pragma once


//=============================================================
// 双向链表
// 定义双向链表元素
#define DLIST_HEAD(type) \
struct {	\
	type* first;	\
}

#define DLIST_ENTRY(type, name)	\
	type* name##pre;		\
	type* name##next;


// 插入到链表头前
#define DLIST_INSERT_HEAD(head, item, name) \
do {	\
		if ((head)->first)	\
			(head)->first->name##pre = (item);	\
		(head)->first->name##pre = (item);	\
		(item)->name##next = (head)->first;	\
		(head)->first = (item);			\
}while(0);

// 获取首元素
#define DLIST_GET_FIRST(head)	\
	(head)->first;

// 从链表移除
#define DLIST_REMOVE(item, name) \
do {			\
	if ((item)->name##pre)	\
		(item)->name##pre->name##next = (item)->name##next;	\
	if ((item)->name##next)	\
		(item)->name##next->name##pre = (item)->name##pre;	\
}while(0);

// 遍历链表
#define DLIST_FOREEACH(var, head, name) \
	for ((var) = (head)->first;	\
		(var) != nullptr;		\
		(var) = (var)->name##next)



//=============================================================
// 先进先出队列
// 定义队列
#define QUEUE_HEAD(type)	\
struct {	\
	type* first;	\
	type** last;		\
}

#define QUEUE_ENTRY(type, name)	\
struct {	\
	type* name##next;	\
}


// 初始化
#define QUEUE_INIT(head)	\
do {	\
	(head)->first = nullptr;	\
	(head)->last = &((head)->first);	\
} while (0)

// 弹出
#define QUEUE_POP(head, name)	\
do{	\
	if ((head)->first)	\
		(head)->first = (head)->first->name##next;	\
} while (0)

// 压入
#define QUEUE_PUSH(head, item, name)	\
do {	\
	(item)->name##next = nullptr;	\
	if (*(head)->last != nullptr)	\
		(head)->first->name##next = (item);	\
	(head)->last = &(item);	\
} while(0)

// 获得首元素
#define QUEUE_GET_FIRST(head)	\
	(head)->first;

// 遍历队列
#define QUEUE_FOREACH(var, head, name)	\
	for ((var)=(head)->first;	\
		(var)!= nullptr;	\
		(var)=(var)->next)
		