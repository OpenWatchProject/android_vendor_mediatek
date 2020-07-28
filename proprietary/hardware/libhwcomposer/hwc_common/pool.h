#ifndef HWC_COMMON_POOL_H
#define HWC_COMMON_POOL_H

#include <cutils/atomic.h>

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/RefBase.h>

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "utils/debug.h"

using namespace android;

inline uint64_t getUniquePoolId()
{
    static volatile int32_t nextId = 0;
    uint64_t id = static_cast<uint64_t>(getpid()) << 32;
    id |= static_cast<uint32_t>(android_atomic_inc(&nextId));
    return id;
}

template <class T>
class ObjectPool
{
public:
    ObjectPool(String8 name, size_t size);
    ~ObjectPool();

    T* getFreeObject(void);
    bool returnObject(const T* item);

private:
    Mutex m_lock;
    Condition m_cond;
    String8 m_name;
    size_t m_size;
    uint64_t m_pool_id;
    Vector<T*> m_available_pool;
};

template <class T>
class LightPoolBase : public LightRefBase<T>
{
public:
    typedef LightPoolBase<T> basetype;

    inline LightPoolBase() : m_pool_id(0), m_count(0), m_pool(NULL) { }
    inline void incStrong(__attribute__((unused)) const void* id) const {
        android_atomic_inc(&m_count);
    }
    inline void decStrong(__attribute__((unused)) const void* id) const {
        if (android_atomic_dec(&m_count) == 1) {
            m_pool->returnObject(static_cast<const T*>(this));
        }
    }
    //! DEBUGGING ONLY: Get current strong ref count.
    inline int32_t getStrongCount() const {
        return m_count;
    }

    virtual void initData() = 0;
    void setPool(ObjectPool<T>* ptr) { m_pool = ptr; }

protected:
    friend ObjectPool<T>;
    inline virtual ~LightPoolBase() { }
    uint64_t m_pool_id;

private:
    mutable volatile int32_t m_count;
    ObjectPool<T> *m_pool;
};

template <class T>
ObjectPool<T>::ObjectPool(String8 name, size_t size)
    : m_name(name)
    , m_size(size)
{
    m_pool_id = getUniquePoolId();
    for (size_t i = 0; i < m_size; i++)
    {
        T* tmp = new T();
        if (tmp == NULL)
        {
            HWC_LOGE("ObjectPool[%s]: failed to allocate object %d/%d", m_name.string(), i, m_size);
        }
        else
        {
            tmp->m_pool_id = m_pool_id;
            tmp->setPool(this);
            m_available_pool.add(tmp);
        }
    }
}

template <class T>
ObjectPool<T>::~ObjectPool()
{
    Mutex::Autolock l(m_lock);

    int timeout_count = 0;
    while (1)
    {
        if (m_size == m_available_pool.size())
        {
            break;
        }

        if (timeout_count == 1)
        {
            HWC_LOGE("ObjectPool[%s]: resource is still held, wait... (%zu/%zu,cnt:%d)",
                    m_name.string(), m_available_pool.size(), m_size, timeout_count);
        }
        if (timeout_count & 0x01)
        {
            HWC_LOGW("ObjectPool[%s]: resource is still held, wait... (%zu/%zu,cnt:%d)",
                    m_name.string(), m_available_pool.size(), m_size, timeout_count);
        }
        m_cond.waitRelative(m_lock, 1000000000);
        timeout_count++;
    }

    for (size_t i = 0; i < m_available_pool.size(); i++)
    {
        delete m_available_pool[i];
    }
    m_available_pool.clear();
}

template <class T>
T* ObjectPool<T>::getFreeObject(void)
{
    Mutex::Autolock l(m_lock);

    int timeout_count = 0;
    while (1)
    {
        if (!m_available_pool.isEmpty())
        {
            break;
        }

        if (timeout_count == 1)
        {
            HWC_LOGE("ObjectPool[%s]: pool is empty, wait... (cnt:%d)",
                    m_name.string(), timeout_count);
        }
        if (timeout_count & 0x01)
        {
            HWC_LOGW("ObjectPool[%s]: pool is empty, wait... (cnt:%d)",
                    m_name.string(), timeout_count);
        }
        m_cond.waitRelative(m_lock, 1000000000);
        timeout_count++;
    }

    T* tmp = NULL;
    typename Vector<T*>::iterator first = m_available_pool.begin();
    tmp = *first;
    m_available_pool.erase(first);

    return tmp;
}

template <class T>
bool ObjectPool<T>::returnObject(const T* ptr)
{
    bool res = false;
    Mutex::Autolock l(m_lock);
    if (ptr->m_pool_id == m_pool_id)
    {
        res = true;
        m_available_pool.add((T*)ptr);
        m_cond.signal();
    }

    if (!res)
    {
        HWC_LOGE("ObjectPool[%s]: failed to recycle item[%p]", m_name.string(), ptr);
    }

    return res;
}
#endif
