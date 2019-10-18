#pragma once

// LinkedList.h

// A doubly-linked list

#ifndef		LINKEDLIST_H
#define		LINKEDLIST_H
#pragma		once

/*  <advc.003s> Only use this when it's obvious that the body of the loop won't delete
	a node through some side-effect. */
// Not yet sure about this. The list objects are often wrapped and thus inaccessible.
/*#define FOR_EACH_NODE(Type, list, pName) \
	for (CLLNode<Type> const* pName = list.head(); pName != NULL; pName = list.next(pName))*/
// </advc.003s>

template <class tVARTYPE> class CLinkList;

template <class tVARTYPE> class CLLNode
{

friend class CLinkList<tVARTYPE>;

public:

    CLLNode(const tVARTYPE& val)
          {
	          m_data = val;

	          m_pNext = NULL;
	          m_pPrev = NULL;
          }
	virtual ~CLLNode() {}

	tVARTYPE	m_data;		//list of vartype

protected:

	CLLNode<tVARTYPE>*	m_pNext;
	CLLNode<tVARTYPE>*	m_pPrev;

};


template <class tVARTYPE> class CLinkList
{

public:

	CLinkList();
	virtual ~CLinkList();

	void clear();
	void swap(CLinkList<tVARTYPE>& list); // K-Mod
	void concatenate(CLinkList<tVARTYPE>& list); // K-Mod

	void insertAtBeginning(const tVARTYPE& val);
	void insertAtEnd(const tVARTYPE& val);
	void insertBefore(const tVARTYPE& val, CLLNode<tVARTYPE>* pThisNode);
	void insertAfter(const tVARTYPE& val, CLLNode<tVARTYPE>* pThisNode);
	CLLNode<tVARTYPE>* deleteNode(CLLNode<tVARTYPE>* pNode);
	void moveToEnd(CLLNode<tVARTYPE>* pThisNode);

	CLLNode<tVARTYPE>* next(CLLNode<tVARTYPE>* pNode) const;
	CLLNode<tVARTYPE>* prev(CLLNode<tVARTYPE>* pNode) const;
	// <advc>
	CLLNode<tVARTYPE> const* next(CLLNode<tVARTYPE> const* pNode) const;
	CLLNode<tVARTYPE> const* prev(CLLNode<tVARTYPE> const* pNode) const;
	static CLLNode<tVARTYPE> const* static_next(CLLNode<tVARTYPE> const* pNode);
	static CLLNode<tVARTYPE>* static_next(CLLNode<tVARTYPE>* pNode); // </advc>

	CLLNode<tVARTYPE>* nodeNum(int iNum) const;

	void Read( FDataStreamBase* pStream );
	void Write( FDataStreamBase* pStream ) const;

	int getLength() const
	{
		return m_iLength;
	}

	CLLNode<tVARTYPE>* head() const
	{
		return m_pHead;
	}

	CLLNode<tVARTYPE>* tail() const
	{
		return m_pTail;
	}

protected:
	int m_iLength;

	CLLNode<tVARTYPE>* m_pHead;
	CLLNode<tVARTYPE>* m_pTail;
};



//constructor
//resets local vars
template <class tVARTYPE>
inline CLinkList<tVARTYPE>::CLinkList()
{
	m_iLength = 0;

	m_pHead = NULL;
	m_pTail = NULL;
}


//Destructor
//resets local vars
template <class tVARTYPE>
inline CLinkList<tVARTYPE>::~CLinkList()
{
  clear();
}


template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::clear()
{
	CLLNode<tVARTYPE>* pCurrNode;
	CLLNode<tVARTYPE>* pNextNode;

	pCurrNode = m_pHead;
	while (pCurrNode != NULL)
	{
		pNextNode = pCurrNode->m_pNext;
		SAFE_DELETE(pCurrNode);
		pCurrNode = pNextNode;
	}

	m_iLength = 0;

	m_pHead = NULL;
	m_pTail = NULL;
}

// K-Mod. (I wish they had just used the STL...)
// swap the contents of two lists
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::swap(CLinkList<tVARTYPE>& list)
{
	std::swap(m_pHead, list.m_pHead);
	std::swap(m_pTail, list.m_pTail);
	std::swap(m_iLength, list.m_iLength);
}

// move the contents from the argument list onto the end of this list
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::concatenate(CLinkList<tVARTYPE>& list)
{
	if (list.m_pHead == NULL)
		return;

	if (m_pTail)
	{
		m_pTail->m_pNext = list.m_pHead;
		list.m_pHead->m_pPrev = m_pTail;
	}
	else
	{
		assert(m_pHead == NULL && m_iLength == 0);
		m_pHead = list.m_pHead;
	}
	assert(list.m_pTail != NULL);
	m_pTail = list.m_pTail;
	m_iLength += list.m_iLength;

	list.m_iLength = 0;
	list.m_pHead = 0;
	list.m_pTail = 0;
}

// K-Mod end

//inserts at the tail of the list
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::insertAtBeginning(const tVARTYPE& val)
{
	CLLNode<tVARTYPE>* pNode;

	assert((m_pHead == NULL) || (m_iLength > 0));

	pNode = new CLLNode<tVARTYPE>(val);

	if (m_pHead != NULL)
	{
		m_pHead->m_pPrev = pNode;
		pNode->m_pNext = m_pHead;
		m_pHead = pNode;
	}
	else
	{
		m_pHead = pNode;
		m_pTail = pNode;
	}

	m_iLength++;
}


//inserts at the tail of the list
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::insertAtEnd(const tVARTYPE& val)
{
	CLLNode<tVARTYPE>* pNode;

	assert((m_pHead == NULL) || (m_iLength > 0));

	pNode = new CLLNode<tVARTYPE>(val);

	if (m_pTail != NULL)
	{
		m_pTail->m_pNext = pNode;
		pNode->m_pPrev = m_pTail;
		m_pTail = pNode;
	}
	else
	{
		m_pHead = pNode;
		m_pTail = pNode;
	}

	m_iLength++;
}


//inserts before the specified node
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::insertBefore(const tVARTYPE& val, CLLNode<tVARTYPE>* pThisNode)
{
	CLLNode<tVARTYPE>* pNode;

	assert((m_pHead == NULL) || (m_iLength > 0));

	if ((pThisNode == NULL) || (pThisNode->m_pPrev == NULL))
	{
		insertAtBeginning(val);
		return;
	}

	pNode = new CLLNode<tVARTYPE>(val);

	pThisNode->m_pPrev->m_pNext = pNode;
	pNode->m_pPrev = pThisNode->m_pPrev;
	pThisNode->m_pPrev = pNode;
	pNode->m_pNext = pThisNode;

	m_iLength++;
}


//inserts after the specified node
template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::insertAfter(const tVARTYPE& val, CLLNode<tVARTYPE>* pThisNode)
{
	CLLNode<tVARTYPE>* pNode;

	assert((m_pHead == NULL) || (m_iLength > 0));

	if ((pThisNode == NULL) || (pThisNode->m_pNext == NULL))
	{
		insertAtEnd(val);
		return;
	}

	pNode = new CLLNode<tVARTYPE>(val);

	pThisNode->m_pNext->m_pPrev = pNode;
	pNode->m_pNext              = pThisNode->m_pNext;
	pThisNode->m_pNext          = pNode;
	pNode->m_pPrev			        = pThisNode;

	m_iLength++;
}


template <class tVARTYPE>
inline CLLNode<tVARTYPE>* CLinkList<tVARTYPE>::deleteNode(CLLNode<tVARTYPE>* pNode)
{
	CLLNode<tVARTYPE>* pPrevNode;
	CLLNode<tVARTYPE>* pNextNode;

	assert(pNode != NULL);

	pPrevNode = pNode->m_pPrev;
	pNextNode = pNode->m_pNext;

	if ((pPrevNode != NULL) && (pNextNode != NULL))
	{
		pPrevNode->m_pNext = pNextNode;
		pNextNode->m_pPrev = pPrevNode;
	}
	else if (pPrevNode != NULL)
	{
		pPrevNode->m_pNext = NULL;
		m_pTail = pPrevNode;
	}
	else if (pNextNode != NULL)
	{
		pNextNode->m_pPrev = NULL;
		m_pHead = pNextNode;
	}
	else
	{
		m_pHead = NULL;
		m_pTail = NULL;
	}

	SAFE_DELETE(pNode);

	m_iLength--;

	return pNextNode;
}


template <class tVARTYPE>
inline void CLinkList<tVARTYPE>::moveToEnd(CLLNode<tVARTYPE>* pNode)
{
	CLLNode<tVARTYPE>* pPrevNode;
	CLLNode<tVARTYPE>* pNextNode;

	assert(pNode != NULL);

	if (getLength() == 1)
	{
	return;
	}

	if (pNode == m_pTail)
	{
		return;
	}

	pPrevNode = pNode->m_pPrev;
	pNextNode = pNode->m_pNext;

	if ((pPrevNode != NULL) && (pNextNode != NULL))
	{
		pPrevNode->m_pNext = pNextNode;
		pNextNode->m_pPrev = pPrevNode;
	}
	else if (pPrevNode != NULL)
	{
		pPrevNode->m_pNext = NULL;
		m_pTail = pPrevNode;
	}
	else if (pNextNode != NULL)
	{
		pNextNode->m_pPrev = NULL;
		m_pHead = pNextNode;
	}
	else
	{
		m_pHead = NULL;
		m_pTail = NULL;
	}

	pNode->m_pNext = NULL;
	m_pTail->m_pNext = pNode;
	pNode->m_pPrev = m_pTail;
	m_pTail = pNode;
}


template <class tVARTYPE>
inline CLLNode<tVARTYPE>* CLinkList<tVARTYPE>::next(CLLNode<tVARTYPE>* pNode) const
{
  //assert(pNode != NULL); // advc.opt: I suspect that this slows assert builds down more than it's worth
  return pNode->m_pNext;
}


template <class tVARTYPE>
inline CLLNode<tVARTYPE>* CLinkList<tVARTYPE>::prev(CLLNode<tVARTYPE>* pNode) const
{
	assert(pNode != NULL);

	return pNode->m_pPrev;
}

// <advc.003s>
// Safer in 'for' loops (those mustn't remove nodes)
template <class tVARTYPE>
inline CLLNode<tVARTYPE> const* CLinkList<tVARTYPE>::next(CLLNode<tVARTYPE> const* pNode) const
{
  return pNode->m_pNext;
}

template <class tVARTYPE>
inline CLLNode<tVARTYPE> const* CLinkList<tVARTYPE>::prev(CLLNode<tVARTYPE> const* pNode) const
{
	return pNode->m_pPrev;
}

/*  Since the next node doesn't depend on the list at all, let's allow
	traversal without a list object. */
template <class tVARTYPE>
inline CLLNode<tVARTYPE>* CLinkList<tVARTYPE>::static_next(CLLNode<tVARTYPE>* pNode)
{
	//assert(pNode != NULL);
	return pNode->m_pNext;
}

template <class tVARTYPE>
inline CLLNode<tVARTYPE> const* CLinkList<tVARTYPE>::static_next(CLLNode<tVARTYPE> const* pNode)
{
	return pNode->m_pNext;
}
// </advc.003s>

template <class tVARTYPE>
inline CLLNode<tVARTYPE>* CLinkList<tVARTYPE>::nodeNum(int iNum) const
{
	CLLNode<tVARTYPE>* pNode;
	int iCount;

	iCount = 0;
	pNode = m_pHead;

	while (pNode != NULL)
	{
		if (iCount == iNum)
		{
			return pNode;
		}

		iCount++;
		pNode = pNode->m_pNext;
	}

	return NULL;
}

//
// use when linked list contains non-streamable types
//
template < class T >
inline void CLinkList< T >::Read( FDataStreamBase* pStream )
{
	int iLength;
	pStream->Read( &iLength );
	clear();

	if ( iLength )
	{
		T* pData = new T;
		for ( int i = 0; i < iLength; i++ )
		{
			pStream->Read( sizeof ( T ), ( byte* )pData );
			insertAtEnd( *pData );
		}
		SAFE_DELETE( pData );
	}
}

template < class T >
inline void CLinkList< T >::Write( FDataStreamBase* pStream ) const
{
	int iLength = getLength();
	pStream->Write( iLength );
	CLLNode< T >* pNode = head();
	while ( pNode )
	{
		pStream->Write( sizeof ( T ), ( byte* )&pNode->m_data );
		pNode = next( pNode );
	}
}

//-------------------------------
// Serialization helper templates:
//-------------------------------

//
// use when linked list contains streamable types
//
template < class T >
inline void ReadStreamableLinkList( CLinkList< T >& llist, FDataStreamBase* pStream )
{
	int iLength;
	pStream->Read( &iLength );
	llist.init();

	if ( iLength )
	{
		T* pData = new T;
		for ( int i = 0; i < iLength; i++ )
		{
			pData->read( pStream );
			llist.insertAtEnd( *pData );
		}
		SAFE_DELETE( pData );
	}
}

template < class T >
inline void WriteStreamableLinkList( CLinkList< T >& llist, FDataStreamBase* pStream )
{
	int iLength = llist.getLength();
	pStream->Write( iLength );
	CLLNode< T >* pNode = llist.head();
	while ( pNode )
	{
		pNode->m_data.write( pStream );
		pNode = llist.next( pNode );
	}
}

#endif	//LINKEDLIST_H
