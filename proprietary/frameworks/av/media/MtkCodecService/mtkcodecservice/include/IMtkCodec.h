#ifndef MTKCODEC_H
#define MTKCODEC_H

#include <utils/RefBase.h>
//#include <binder/IInterface.h>
//#include <binder/Parcel.h>
//#include <binder/IBinder.h>
#include <utils/Errors.h>
#include <hwbinder/Parcel.h>


using namespace android;
using ::android::hardware::Parcel;

class IMtkCodec
{
public:
	
    virtual status_t Init(const Parcel &para) = 0;
    virtual status_t DeInit(const Parcel &para) = 0;
    virtual status_t DoCodec(const Parcel &para, Parcel *reply) = 0;
    virtual status_t Reset(const Parcel &para) = 0;
    virtual status_t Create(const Parcel &para, Parcel *reply=NULL) = 0;
    virtual status_t Destroy(const Parcel &para) = 0;	
    virtual status_t Command(const Parcel &para __unused, Parcel *reply __unused)
    {
        return 0;
    }
    virtual ~IMtkCodec()
   {
   }
};

enum OPERATION 
{  
    INIT= 1,
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

#endif
