#include "TestDefine.h"
#include "DpBufferPool.h"

class TestListener: public DpBufferListener
{
public:
    TestListener()
        : m_queuedCount(0),
          m_releasedCount(0)
    {
    }

    ~TestListener()
    {
    }

    void onBufferQueued(uint32_t userToken)
    {
        m_queuedCount++;
    }

    int32_t getQueuedCount()
    {
        return m_queuedCount;
    }

    void onDequeueTimeOut(uint32_t userToken)
    {
    }

    void onBufferReleased(uint32_t userToken)
    {
        m_releasedCount++;
    }

    int32_t getReleasedCount()
    {
        return m_releasedCount;
    }

    void onAcquireTimeOut(uint32_t userToken)
    {
    }

private:
    int32_t m_queuedCount;
    int32_t m_releasedCount;
};


/*
 * Basic Buffer Test #0: basic operation test
 */
void basicBufferCase0(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    int32_t           bufID;
    uint8_t           *pBuf;
    uint32_t          base[3];
    DpBasicBufferPool pool;
    TestListener      listen;
    
    pBuf = (uint8_t*)malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pBuf), "Can't allocate buffer!");

    bufID = pool.registerBuffer(pBuf,
                                640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (bufID >= 0), "Register buffer failed");

    status = pool.registerListener(&listen, 0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register listener failed");

    status = pool.dequeueBuffer(&bufID,
                                base);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue buffer failed");

    REPORTER_ASSERT_MESSAGE(pReporter, (pBuf == (uint8_t*)base[0]), "Incorrect buffer base");
    
    status = pool.queueBuffer(bufID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue buffer failed");

    status = pool.cancelBuffer(bufID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Cancel buffer failed");

    status = pool.unregisterBuffer(bufID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Unregister buffer failed");

    if (NULL != pBuf)
    {
        free(pBuf);
    }
}

DEFINE_TEST_CASE("BasicBuffer0", BasicBuffer0, basicBufferCase0)