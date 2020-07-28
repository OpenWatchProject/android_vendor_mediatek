#ifndef BPMTKCODEC_H
#define BPMTKCODEC_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
using namespace android;


enum OPERATION 
{  
    INIT= IBinder::FIRST_CALL_TRANSACTION,
    RESET,
    DEINIT,
    DOCODEC,
    CREATE,
    DESTROY,
    COMMAND
};
enum CODECTYPE
{
    ENCODE,
    DECODE
};

class DeathNotifier: public IBinder::DeathRecipient
{
    public:
	    DeathNotifier(IInterface* pCodec) 
	    {
	    	pmtkCodec = pCodec;
	    }
	    virtual ~DeathNotifier();
	    virtual void binderDied(const wp<IBinder>& who);
    private:
        IInterface* pmtkCodec;
};

class BpMtkCodec : public IInterface
{
public:
    BpMtkCodec();
    ~BpMtkCodec();
    status_t Init(const Parcel &para);
    status_t DeInit(const Parcel &para);
    status_t DoCodec(const Parcel &para, Parcel *reply);
    status_t Reset(const Parcel &para);
    status_t Create(const Parcel &para, Parcel *reply=NULL);
    status_t Destroy(const Parcel &para);
    status_t Command(const Parcel &para, Parcel *reply)
    {
        return 0;
    }
    int32_t GetCodecId()
   {
   	return mCodecId;
   }
   bool IsSrvReady()
  {
  	return mSrvReady;
  }

protected:
    virtual IBinder*            onAsBinder();
	
private:
    int32_t mCodecId;
    sp<IBinder> mBinder;
    sp<DeathNotifier> mNotifier;
    bool mSrvReady;
	
};
#endif
