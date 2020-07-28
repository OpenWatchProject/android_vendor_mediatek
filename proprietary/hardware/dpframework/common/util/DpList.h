#ifndef __DP_LIST_PTR_H__
#define __DP_LIST_PTR_H__

#include <DpDataType.h>

//-----------------------------------------------------------------------------
// Doubly-linked list.  Instantiate with "ListPtr<MyClass> myList".
//
// Objects added to the list are address-assigned.
//-----------------------------------------------------------------------------

template<typename T>
class ListPtr
{
protected:
    /*
     * One element in the list.
     */
    class _Node;

    /*
     * Iterator for walking through the list.
     */
    
    template <typename TYPE>
    struct CONST_ITERATOR
    {
        typedef _Node const *NodePtr;
        typedef const TYPE  Type;
    };
    
    template <typename TYPE>
    struct NON_CONST_ITERATOR
    {
        typedef _Node  *NodePtr;
        typedef TYPE   Type;
    };
    
    template<typename U, template <class> class Constness>
    class _ListIterator
    {
        typedef _ListIterator<U, Constness>     _Iter;
        typedef typename Constness<U>::NodePtr  _NodePtr;
        typedef typename Constness<U>::Type     _Type;

        explicit _ListIterator(_NodePtr pValue)
            : m_pNode(pValue)
        {
        }

    public:
        _ListIterator()
        {
        }

        _ListIterator(const _Iter& rhs)
            : m_pNode(rhs.m_pNode)
        {
        }

        ~_ListIterator()
        {
        }
        
        // this will handle conversions from iterator to const_iterator
        // (and also all convertible iterators)
        // Here, in this implementation, the iterators can be converted
        // if the nodes can be converted
        template<typename V> explicit 
        _ListIterator(const V &rhs)
            : m_pNode(rhs.m_pNode)
        {
        }

        /*
         * Dereference operator.  Used to get at the juicy insides.
         */
        _Type& operator*() const
        {
            return m_pNode->getRef();
        }

        _Type* operator->() const
        {
            return m_pNode->getPtr();
        }

        /*
         * Iterator comparison.
         */
        inline bool operator==(const _Iter& right) const
        { 
            return m_pNode == right.m_pNode;
        }
        
        inline bool operator!=(const _Iter& right) const
        { 
            return m_pNode != right.m_pNode;
        }

        /*
         * handle comparisons between iterator and const_iterator
         */
        template<typename OTHER>
        inline bool operator==(const OTHER& right) const
        { 
            return m_pNode == right.m_pNode;
        }
        
        template<typename OTHER>
        inline bool operator!=(const OTHER& right) const
        {
            return m_pNode != right.m_pNode;
        }

        /*
         * Incr/decr, used to move through the list.
         */
        inline _Iter& operator++()
        {
            // pre-increment
            m_pNode = m_pNode->getNext();
            return *this;
        }

        const _Iter operator++(int)
        {
            // post-increment
            _Iter tmp(*this);

            m_pNode = m_pNode->getNext();
            return tmp;
        }

        inline _Iter& operator--()
        {
            // pre-increment
            m_pNode = m_pNode->getPrev();
            return *this;
        }

        const _Iter operator--(int)
        {
            // post-increment
            _Iter tmp(*this);

            m_pNode = m_pNode->getPrev();
            return tmp;
        }

        inline _NodePtr getNode() const
        {
            return m_pNode;
        }

        _NodePtr m_pNode;  /* Should be private, but older gcc fails */

    private:
        friend class ListPtr;
    };

    /*
     * One element in the list.
     */
    class _Node
    {
    public:
        explicit _Node(T *pValue)
            : m_pValue(pValue)
        {
        }

        ~_Node()
        {
        }

        inline T& getRef()
        {
            return *m_pValue;
        }

        inline T* getPtr()
        {
            return m_pValue;
        }

        inline const T& getRef() const
        {
            return *m_pValue;
        }

        inline const T* getPtr() const
        {
            return m_pValue;
        }

        inline _Node* getPrev() const
        {
            return m_pPrev;
        }

        inline _Node* getNext() const
        {
            return m_pNext;
        }

        inline void setVal(T *pValue)
        {
            m_pValue = pValue;
        }

        inline void setPrev(_Node *pNode)
        {
            m_pPrev = pNode;
        }

        inline void setNext(_Node *pNode)
        {
            m_pNext = pNode;
        }
    private:
        T     *m_pValue;
        _Node *m_pPrev;
        _Node *m_pNext;
    };
public:
    ListPtr()
        : m_pMiddle(0),
          m_newCount(0),
          m_delCount(0)
    {
        prep();
    }

    ListPtr(const ListPtr<T>& src)
    {
        // Copy-constructor
        prep();
        insert(begin(), src.begin(), src.end());
    }

    virtual ~ListPtr()
    {
        clear();
        delete[] (uint8_t*)m_pMiddle;
        m_delCount++;

        assert(m_newCount == m_delCount);
    }

    typedef _ListIterator<T, NON_CONST_ITERATOR> iterator;
    typedef _ListIterator<T, CONST_ITERATOR>     const_iterator;

    ListPtr<T>& operator=(const ListPtr<T> &right);

    /* returns true if the list is empty */
    inline bool empty() const
    {
        return (m_pMiddle->getNext() == m_pMiddle);
    }

    /* return #of elements in list */
    int size() const
    {
        return int(distance(begin(), end()));
    }

    /*
     * Return the first element or one past the last element.  The
     * _Node* we're returning is converted to an "iterator" by a
     * constructor in _ListIterator.
     */
    inline iterator begin()
    {
        return iterator(m_pMiddle->getNext());
    }

    inline const_iterator begin() const
    {
        return const_iterator(const_cast<_Node const*>(m_pMiddle->getNext()));
    }

    inline iterator end()
    {
        return iterator(m_pMiddle);
    }

    inline const_iterator end() const
    {
        return const_iterator(const_cast<_Node const*>(m_pMiddle));
    }

    /* add the object to the head or tail of the list */
    void push_front(T &value)
    {
        insert(begin(), &value);
    }

    void push_back(T &value)
    {
        insert(end(), &value);
    }

    void push_front(T *pValue)
    {
        insert(begin(), pValue);
    }

    void push_back(T *pValue)
    {
        insert(end(), pValue);
    }

    T* remove(iterator posn)
    {
        T* pRetVal;

        pRetVal = posn.getNode()->getPtr();
        erase(posn);
        return pRetVal;
    }

    /* insert before the current node; returns iterator at new node */
    iterator insert(iterator posn, T *pValue)
    {
        _Node *pNewNode;
        
        pNewNode = new _Node(pValue);  // Alloc & copy-construct
        m_newCount++;

        pNewNode->setNext(posn.getNode());
        pNewNode->setPrev(posn.getNode()->getPrev());
        posn.getNode()->getPrev()->setNext(pNewNode);
        posn.getNode()->setPrev(pNewNode);

        return iterator(pNewNode);
    }

    /* Insert a range of elements before the current node */
    void insert(iterator posn, const_iterator first, const_iterator last)
    {
        for (; first != last; ++first)
        {
            insert(posn, const_cast<T*>(&(*first)));
        }
    }

    /* remove one entry; returns iterator at next node */
    iterator erase(iterator posn)
    {
        _Node *pNext;
        _Node *pPrev;

        pNext = posn.getNode()->getNext();
        pPrev = posn.getNode()->getPrev();

        pPrev->setNext(pNext);
        pNext->setPrev(pPrev);

        delete posn.getNode();
        m_delCount++;

        return iterator(pNext);
    }

    /* remove a range of elements */
    iterator erase(iterator first, iterator last)
    {
        while (first != last)
        {
            erase(first++);  // Don't erase than incr later!
        }

        return iterator(last);
    }

    /* remove all contents of the list */
    void clear()
    {
        _Node *pCurrent;
        _Node *pNext;        

        pCurrent = m_pMiddle->getNext();
        while (pCurrent != m_pMiddle)
        {
            pNext = pCurrent->getNext();
            delete pCurrent;

            m_delCount++;

            pCurrent = pNext;
        }

        m_pMiddle->setPrev(m_pMiddle);
        m_pMiddle->setNext(m_pMiddle);
    }

    /*
     * Measure the distance between two iterators.  On exist, "first"
     * will be equal to "last".  The iterators must refer to the same
     * list.
     *
     * FIXME: This is actually a generic iterator function. It should be a
     * template function at the top-level with specializations for things like
     * vector<>, which can just do pointer math). Here we limit it to
     * _ListIterator of the same type but different constness.
     */
    template<typename U, template <class> class CL, template <class> class CR>
    int distance(_ListIterator<U, CL> first, _ListIterator<U, CR> last) const
    {
        int32_t count;
        
        count = 0;
        while (first != last)
        {
            ++first;
            ++count;
        }

        return count;
    }

private:
    /*
     * I want a _Node but don't need it to hold valid data.  More
     * to the point, I don't want T's constructor to fire, since it
     * might have side-effects or require arguments.  So, we do this
     * slightly uncouth storage alloc.
     */
    void prep()
    {
        m_pMiddle = (_Node*)new uint8_t[sizeof(_Node)];
        m_newCount++;

        m_pMiddle->setPrev(m_pMiddle);
        m_pMiddle->setNext(m_pMiddle);
    }

    /*
     * This node plays the role of "pointer to head" and "pointer to tail".
     * It sits in the middle of a circular list of nodes.  The iterator
     * runs around the circle until it encounters this one.
     */
    _Node*      m_pMiddle;
    int32_t     m_newCount;
    int32_t     m_delCount;
};

/*
 * Assignment operator.
 *
 * The simplest way to do this would be to clear out the target list and
 * fill it with the source.  However, we can speed things along by
 * re-using existing elements.
 */
template<class T>
ListPtr<T>& ListPtr<T>::operator=(const ListPtr<T>& right)
{
    if (this == &right)
    {
        return *this;       // self-assignment
    }

    iterator firstDst = begin();
    iterator lastDst  = end();
 
    const_iterator firstSrc = right.begin();
    const_iterator lastSrc  = right.end();

    while (firstSrc != lastSrc && firstDst != lastDst)
    {
        *firstDst++ = *firstSrc++;
    }

    if (firstSrc == lastSrc)        // Ran out of elements in source?
    {
        erase(firstDst, lastDst);   // yes, erase any extras
    }
    else
    {
        insert(lastDst, firstSrc, lastSrc);     // copy remaining over
    }

    return *this;
}

#endif  // __DP_LIST_PTR_H__
