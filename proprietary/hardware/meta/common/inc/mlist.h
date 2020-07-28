#ifndef _M_LIST_
#define _M_LIST_

#include <stddef.h>

template <typename _Tx>
struct mlist_item
{
	_Tx data;
	struct mlist_item<_Tx> *next;	
};

template <typename _Tx>
class mlist_iter
{
public:
	typedef mlist_item<_Tx> nodetype;
	typedef mlist_iter<_Tx> self;
	
	mlist_iter() : node(NULL)
	{
	}
	
	mlist_iter(nodetype *p) : node(p)
	{
	}
	
public:
	self &operator++()
	{
	//	assert(node != NULL);
		node = node->next;
		return *this;
	}
	
	_Tx &operator*()
	{
	//	assert(node != NULL);
		return node->data;
	}
	
	_Tx *operator->()
	{
	//	assert(node != NULL);
		return &(node->data);
	}
	
	bool operator==(const self &ref) const
	{
		return (node == ref.node);
	}
	
	bool operator!=(const self &ref) const
	{
		return (node != ref.node);
	}
	
private:
	nodetype *node;
};

template <typename _Tx>
class mlist
{
public:
	typedef mlist_iter<_Tx> iterator;
	typedef mlist_item<_Tx> nodetype;
	
	mlist()
	{
		head = NULL;
		tail = NULL;
	}
	
	~mlist()
	{
		nodetype *tmp = NULL;
		
		while (head != NULL)
		{
			tmp = head->next;
			delete head;
			head = tmp;
		}
	}
	
public:
	void push_back(const _Tx &x)
	{
		if (tail != NULL)
		{
			tail->next = new nodetype;
			tail = tail->next;
		}
		else
		{
			tail = new nodetype;
			head = tail;
		}
		tail->data = x;
		tail->next = NULL;
	}
	
	void destroy_node(const _Tx &x)
	{
		nodetype *tmp = NULL;
		nodetype *delete_node =NULL;
		
		if(head->data == x)
		{
			delete_node = head;
			head = head->next;
		}
		else
		{
			tmp = head;	
			while(tmp->next->data != x)
			{
				tmp = tmp->next;				
			}
			delete_node = tmp->next;
			tmp->next = tmp->next->next;
		}

		if(head == NULL)
			tail = NULL;

		if(delete_node != NULL)
			delete delete_node;		
	}
	
	iterator begin() const
	{
		return iterator(head);
	}
		
	iterator end() const
	{
		return iterator(NULL);
	}

	void clear()
	{
		nodetype *tmp = NULL;
		while (head != NULL)
		{
			tmp = head->next;
			delete head;
			head = tmp;
		}
		head = NULL;
		tail = NULL;
	}

private:
	nodetype *head;
	nodetype *tail;
};

#endif	// _M_LIST_
