
#include "MtkOmxVencCmdQueue.h"

MtkOmxCmdQueue::MtkOmxCmdQueue()
{
    INIT_MUTEX(mCmdMutex);
    INIT_SEMAPHORE(mCmdSem);
    memset(mCmdQueueItems, 0, sizeof(MTK_OMX_CMD_QUEUE_ITEM)*MAX_CMD_ITEM);

#if 0
    for (int i=0; i < MAX_CMD_ITEM; i++)
    {
        mCmdQueueItems[i].Used = OMX_FALSE;
    }
#endif

    char EnableCmdItemLogValue[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk.omx.cmditem.log", EnableCmdItemLogValue, "0");
    mEnableCmdItemLog = (OMX_BOOL) atoi(EnableCmdItemLogValue);
}

MtkOmxCmdQueue::~MtkOmxCmdQueue()
{
    DESTROY_MUTEX(mCmdMutex);
    DESTROY_SEMAPHORE(mCmdSem);
}

void MtkOmxCmdQueue::NewCmdItem(MTK_OMX_CMD_QUEUE_ITEM** ppItem)
{
    LOCK(mCmdMutex);

    for (int i=0; i < MAX_CMD_ITEM; i++)
    {
        if (mCmdQueueItems[i].Used == OMX_FALSE)
        {
            mCmdQueueItems[i].Used = OMX_TRUE;
            mCmdQueueItems[i].nItemIndex = i;
            UNLOCK(mCmdMutex);

            MTK_OMX_CMDITEM_LOGD(this, "New Cmd Item: 0x%x", &mCmdQueueItems[i]);
            *ppItem = &mCmdQueueItems[i];
            return;
        }
    }

    UNLOCK(mCmdMutex);

    ALOGE("[Error] Run out all cmd items. &mCmdQueue=0x%x, mCmdQuque.size=%d", &mCmdQueue, mCmdQueue.size());
    for (int i=0; i<MAX_CMD_ITEM;i++)
    {
        ALOGE("NewCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
    }
    abort();
    return;
}

void MtkOmxCmdQueue::PutCmd(MTK_OMX_CMD_QUEUE_ITEM* pItem)
{
    if (pItem == NULL || pItem->nItemIndex < 0 || pItem->nItemIndex > MAX_CMD_ITEM)
    {
        if (pItem != NULL)
            ALOGE("PutCmd(). Abnormal Item: 0x%x, nCmdItemIndex:%d", pItem, pItem->nItemIndex);
        else
            ALOGE("PutCmd(). NULL pItem");

        abort();
    }
    else
    {
        LOCK(mCmdMutex);

        mCmdQueue.push(pItem);
        SIGNAL(mCmdSem);
        MTK_OMX_CMDITEM_LOGD(this, "PutCmd(). pItem=0x%x, mCmdQuque.size=%d, SemValue:%d", pItem, mCmdQueue.size(), get_sem_value(&mCmdSem));
        UNLOCK(mCmdMutex);
    }
}

int MtkOmxCmdQueue::GetCmdSize()
{
    int nCmdSize =  mCmdQueue.size();
    MTK_OMX_CMDITEM_LOGD(this, "GetCmdSize(). mCmdQuque.size=%d, Sem value:%d", mCmdQueue.size(), get_sem_value(&mCmdSem));
    return nCmdSize;
}

void MtkOmxCmdQueue::GetCmd(MTK_OMX_CMD_QUEUE_ITEM** ppItem)
{
    MTK_OMX_CMDITEM_LOGD(this, "GetCmd(). &mCmdQueue=0x%x, mCmdQuque.size=%d, Sem value:%d", &mCmdQueue, mCmdQueue.size(), get_sem_value(&mCmdSem));

    WAIT(mCmdSem);
    LOCK(mCmdMutex);
    //pItem = mCmdList[0];
    *ppItem = mCmdQueue.front();
    mCmdQueue.pop();

    if (OMX_TRUE != (*ppItem)->Used)
    {
        ALOGE("GetCmd(). Abnormal Cmd, Not been used:0x%x", *ppItem);
        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("GetCmd(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }

        abort();
    }
    else
    {
        MTK_OMX_CMDITEM_LOGD(this, "GetCmd(). got Cmd: 0x%x, Index: %d, bUsed:%d", *ppItem, (*ppItem)->nItemIndex, (*ppItem)->Used);
    }

    //mCmdList.erase(0);

    UNLOCK(mCmdMutex);
}

void MtkOmxCmdQueue::FreeCmdItem(MTK_OMX_CMD_QUEUE_ITEM* pItem)
{
    LOCK(mCmdMutex);

    if (pItem == NULL || pItem->Used != OMX_TRUE)
    {
        if (pItem != NULL)
            ALOGE("FreeCmdItam(). Abnormal pItem:0x%x, Index:%d, Used:%d", pItem, pItem->nItemIndex, pItem->Used);
        else
            ALOGE("FreeCmdItam(). NULL pItem");

        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("FreeCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }
        abort();
    }
    else
    {
        for (int i=0; i < MAX_CMD_ITEM; i++)
        {
            if (&mCmdQueueItems[i] == pItem)
            {
                MTK_OMX_CMDITEM_LOGD(this, "FreeCmdItem() &mCmdQueueItems[%d]:0x%x", i, &mCmdQueueItems[i]);
                mCmdQueueItems[i].Used = OMX_FALSE;
                UNLOCK(mCmdMutex);
                return;
            }
        }

        UNLOCK(mCmdMutex);

        ALOGE("[Error] FreeCmdItem(). Can't match cmd item");
        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("FreeCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }
    }
}