#define MTK_LOG_ENABLE 1
#include "BpMtkCodec.h"
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <cutils/log.h>
#define LOG_TAG "BpMtkCodec"

#define LOGD SLOGD
#define LOGE SLOGE
#define LOGV SLOGV

void DeathNotifier::binderDied(const wp < IBinder > & who)
{
    LOGD("mtk.codecservice died");
}
//...>>>
DeathNotifier::~DeathNotifier()
{
    LOGD("~DeathNotifier+");
    if (pmtkCodec != 0)
    {
        //pmtkCodec->asBinder()->unlinkToDeath(this);
		IInterface::asBinder(pmtkCodec)->unlinkToDeath(this);
        pmtkCodec = NULL;
    }
    LOGD("~DeathNotifier-");
}

BpMtkCodec::~BpMtkCodec()
{
    LOGD("~BpMtkCodec+");
    
    if (mNotifier != NULL)
    {
        LOGV("~BpMtkCodec:1");
        mNotifier.clear();
        mNotifier = NULL;    
    }

    if (mBinder != NULL)
    {
        LOGV("~BpMtkCodec:2");
        mBinder.clear();
        mBinder = NULL;
    }
    
    LOGD("~BpMtkCodec-");    
}

BpMtkCodec::BpMtkCodec()
{
    LOGD("ctor");
    mNotifier = NULL;
    sp<IServiceManager> sm = defaultServiceManager();
    mBinder = sm->getService(String16("mtk.codecservice"));
    if (mBinder.get() == NULL)
    {
        mSrvReady = false;
        LOGE("mtk.codecservice is NULL");
    }
    else
    {
        mSrvReady = true;
        mNotifier = new DeathNotifier(this);
        mBinder->linkToDeath(mNotifier);
        LOGD("mtk.codecservice success");
    }
}

status_t BpMtkCodec::Create(const Parcel &para, Parcel *reply)
{
    LOGD("create");
    if (mSrvReady)
    {
        mBinder->transact(CREATE, para, reply);
        mCodecId = reply->readInt32();
        return OK;
    }
    else
    {
        LOGE("create:service error:%d",DEAD_OBJECT);
        return DEAD_OBJECT;
    }
    
}

status_t BpMtkCodec::Destroy(const Parcel &para)
{
    if (mSrvReady)
    {
        Parcel reply;
        mBinder->transact(DESTROY, para, &reply);
        status_t iresult = reply.readInt32();
        if (iresult != OK)
        {
            LOGE("Destroy error");
            return iresult;
        }
        else
        {
            return OK;
        }
    }
    else
    {
        LOGE("destroy:service error:%d",DEAD_OBJECT);
        return DEAD_OBJECT;
    }
}

status_t BpMtkCodec::Init(const Parcel &para)
{
    Parcel reply;
    mBinder->transact(INIT, para, &reply);
    status_t iresult = reply.readInt32();
    if (iresult != OK)
    {
        LOGE("Init error");
        return iresult;
    }
    else
    {
        return OK;
    }
}

status_t BpMtkCodec::DeInit(const Parcel &para)
{
    Parcel reply;    
    mBinder->transact(DEINIT, para, &reply);
    status_t iresult = reply.readInt32();
    if (iresult != OK)
    {
        LOGE("DeInit error");
        return iresult;
    }
    else
    {
        return OK;
    }
}

status_t BpMtkCodec::DoCodec(const Parcel &para, Parcel *reply)
{
    status_t iresult = mBinder->transact(DOCODEC, para, reply);
    return iresult;
}

status_t BpMtkCodec::Reset(const Parcel &para)
{
    Parcel reply;    
    mBinder->transact(RESET, para, &reply);
    status_t iresult = reply.readInt32();
    if (iresult != OK)
    {
        LOGE("Reset error");
        return iresult;
    }
    else
    {
        return OK;
    }
}

IBinder* BpMtkCodec::onAsBinder()
{
    if (mBinder != NULL)
    {
        LOGD("onAsBinder:binder.get");
        return mBinder.get();
    }
    else
    {
        LOGD("onAsBinder:NULL");
        return NULL;
    }
}