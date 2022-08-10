#pragma once


//=============================================================
// 双向链表
// 定义双向链表头，这里会多浪费一个pre指针的空间
#define DLIST_HEAD(type)	\
struct {	\
	struct type* pre;		\
	struct type* next;		\
}

// 定义双向链表元素
#define DLIST_ENTRY(type)	\
struct {	\
	type* pre;		\
	type* next;		\
}


// 插入到链表头前
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


// 从链表移除
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