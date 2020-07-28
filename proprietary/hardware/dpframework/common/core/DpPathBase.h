#ifndef __DP_PATH_BASE_H__
#define __DP_PATH_BASE_H__

#include "DpEngineBase.h"

/*
 * This is the base class for the path object.
 */
class DpPathBase
{
protected:
    template <class _Type>
    class _PathIterator
    {
        typedef _PathIterator<_Type> _Iterator;

    public:
        _PathIterator()
            : m_pObjects(NULL)
        {
        }

        _PathIterator(_Type **pObjects)
            : m_pObjects(pObjects)
        {
        }

        _PathIterator(_Type *pObjects)
            : m_pObjects(pObjects)
        {
        }

        _PathIterator(const _Iterator& rhs)
            : m_pObjects(rhs.m_pObjects)
        {
        }

        ~_PathIterator()
        {
        }

        _Type* operator->() const
        {
            return m_pObjects[0];
        }

        _Type& operator*() const
        {
            return *m_pObjects[0];
        }

        /*
         * Iterator comparison.
         */
        inline bool operator==(const _Iterator& right) const
        {
            if (m_pObjects[0] == right.m_pObjects[0])
            {
                return true;
            }

            return false;
        }

        inline bool operator!=(const _Iterator& right) const
        {
            if (m_pObjects[0] != right.m_pObjects[0])
            {
                return true;
            }

            return false;
        }

        /*
         * Incr/decr, used to move through the list.
         */
        inline _Iterator& operator++()
        {
            // pre-increment
            m_pObjects++;
            return *this;
        }

        const _Iterator operator++(int32_t)
        {
            // post-increment
            _Iterator temp(*this);
            m_pObjects++;
            return temp;
        }

        inline _Iterator& operator--()
        {
            // pre-increment
            m_pObjects--;
            return *this;
        }

        const _Iterator operator--(int32_t)
        {
            // post-increment
            _Iterator temp(*this);
            m_pObjects--;
            return temp;
        }

    private:
        _Type   **m_pObjects;
    };

public:
    typedef _PathIterator<DpEngineBase> iterator;

    DpPathBase()
    {
    }

    virtual ~DpPathBase()
    {
    }

    virtual uint32_t size() = 0;

    virtual iterator begin() = 0;

    virtual iterator end() = 0;

    DP_STATUS_ENUM getScenario(STREAM_TYPE_ENUM *pType)
    {
        if (NULL == pType)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetScenario(pType);
    }

    DP_STATUS_ENUM getPMQOS(mdp_pmqos *pPMQOS)
    {
        if (NULL == pPMQOS)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetPMQOS(pPMQOS);
    }

    DP_STATUS_ENUM getPQReadback(bool *pReadback)
    {
        if (NULL == pReadback)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetPQReadback(pReadback);
    }

    DP_STATUS_ENUM getHDRReadback(bool *pReadback)
    {
        if (NULL == pReadback)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetHDRReadback(pReadback);
    }

    DP_STATUS_ENUM getDREReadback(int32_t *pReadback)
    {
        if (NULL == pReadback)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetDREReadback(pReadback);
    }

    DP_STATUS_ENUM getPriority(int32_t *pPriority)
    {
        if (NULL == pPriority)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetPriority(pPriority);
    }

    DP_STATUS_ENUM getPathFlag(int64_t *pFlag)
    {
        if (NULL == pFlag)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetPathFlag(pFlag);
    }

    const uint32_t* getReadbackValues(uint32_t& numValues)
    {
        return onGetReadbackValues(numValues);
    }

    DP_STATUS_ENUM getCommander(DpCommand **commander)
    {
        return onGetCommander(commander);
    }

    DP_STATUS_ENUM initPath()
    {
        DP_TRACE_CALL();
        return onInitPath();
    }

    DP_STATUS_ENUM startPath()
    {
        DP_TRACE_CALL();
        return onStartPath();
    }

    DP_STATUS_ENUM waitPath()
    {
        DP_TRACE_CALL();
        return onWaitPath();
    }

    DP_STATUS_ENUM stopPath()
    {
        DP_TRACE_CALL();
        return onStopPath();
    }

    DP_STATUS_ENUM abortPath()
    {
        DP_TRACE_CALL();
        return onAbortPath();
    }

    DP_STATUS_ENUM destroyPath()
    {
        DP_TRACE_CALL();
        return onDestroyPath();
    }

    DP_STATUS_ENUM dumpDebugPath()
    {
        return onDumpDebugPath();
    }

    DP_STATUS_ENUM getSyncMode(bool *pSyncMode)
    {
        if (NULL == pSyncMode)
        {
            return DP_STATUS_INVALID_PARAX;
        }

        return onGetSyncMode(pSyncMode);
    }

    uint32_t* getReadbackPABuffer(uint32_t& readbackPABufferIndex)
    {
        return onGetReadbackPABuffer(readbackPABufferIndex);
    }

    DP_STATUS_ENUM setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID)
    {
        return onSetNumReadbackPABuffer(numReadbackPABuffer, readbackEngineID);
    }

protected:
    virtual DP_STATUS_ENUM onGetScenario(STREAM_TYPE_ENUM *pType) = 0;

    virtual DP_STATUS_ENUM onGetPQReadback(bool *pReadback) = 0;

    virtual DP_STATUS_ENUM onGetHDRReadback(bool *pReadback) = 0;

    virtual DP_STATUS_ENUM onGetDREReadback(int32_t *pReadback) = 0;

    virtual DP_STATUS_ENUM onGetPriority(int32_t *pPriority) = 0;

    virtual DP_STATUS_ENUM onGetPMQOS(mdp_pmqos *pPMQOS) = 0;

    virtual DP_STATUS_ENUM onGetPathFlag(int64_t *pFlag) = 0;

    virtual const uint32_t* onGetReadbackValues(uint32_t& numValues) = 0;

    virtual DP_STATUS_ENUM onGetCommander(DpCommand **commander) = 0;

    virtual DP_STATUS_ENUM onInitPath() = 0;

    virtual DP_STATUS_ENUM onStartPath() = 0;

    virtual DP_STATUS_ENUM onWaitPath() = 0;

    virtual DP_STATUS_ENUM onStopPath() = 0;

    virtual DP_STATUS_ENUM onAbortPath() = 0;

    virtual DP_STATUS_ENUM onDestroyPath() = 0;

    virtual DP_STATUS_ENUM onDumpDebugPath() = 0;

    virtual DP_STATUS_ENUM onGetSyncMode(bool *pSyncMode) = 0;

    virtual uint32_t* onGetReadbackPABuffer(uint32_t& readbackPABufferIndex) = 0;

    virtual DP_STATUS_ENUM onSetNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID) = 0;
};

#endif  // __DP_PATH_BASE_H__
