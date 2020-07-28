#ifndef __STT_BUF_QUEUE_H__
#define __STT_BUF_QUEUE_H__

#include <utils/Vector.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>
#include <mtkcam/def/BuiltinTypes.h>

namespace NSCam {

class ISttBufQ : public virtual android::RefBase
{
public:
    struct DATA : public virtual android::RefBase
    {
        MINT32 MagicNumberStt;
        MINT32 MagicNumberRequest;
        android::Vector<char> AAO;
    };

public:
    static MVOID createSingleton(MINT32 SensorDev, const char *UserName);
    static MVOID destroySingleton(MINT32 SensorDev, const char *UserName);
    static android::sp<ISttBufQ> getInstance(MINT32 SensorDev);

    virtual ~ISttBufQ() {};

    virtual android::sp<DATA> deque() = 0;
    virtual android::sp<DATA> deque_last() = 0;
    virtual MBOOL enque_first(android::sp<DATA> pData) = 0;
    virtual MBOOL enque(android::sp<DATA> pData) = 0;
    virtual android::sp<DATA> deque_byMagicNumberRequest(MINT32 MagicNumberRequest) = 0;
    virtual android::sp<DATA> deque_byMagicNumberStt(MINT32 MagicNumberStt) = 0;
};

}; //namespace NSCam

#endif // #ifndef __STT_BUF_QUEUE_H__

