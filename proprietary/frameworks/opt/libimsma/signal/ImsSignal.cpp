
/*****************************************************************************
 * Author:
 * -------
 *   Qian Dong
 *
 ****************************************************************************/
#include <utils/Log.h>
#undef LOG_TAG
#include "ImsSignal.h"


namespace android {

//signal
//===================

Mutex gDefaultImsSignalLock;
sp<ImsSignal> gDefaultImsSignal = NULL;
int32_t  gDefaultImsSignalCnt = 0;

sp<ImsSignal> getDefaultImsSignal(){
    ALOGI("getDefaultImsSignal+++");
    AutoMutex _l(gDefaultImsSignalLock);//multithread or signal thread?

    if (gDefaultImsSignal != NULL){
        gDefaultImsSignalCnt++;
        ALOGI("[VT][ImsSignal]aleady getDefaultImsSignal %p,gDefaultImsSignal %d",gDefaultImsSignal.get(),gDefaultImsSignalCnt);
        return gDefaultImsSignal;
    }

    {
        if (gDefaultImsSignal == NULL) {
		gDefaultImsSignal = new ImsSignal;
		gDefaultImsSignal->start();
		gDefaultImsSignalCnt++;
		ALOGI("[VT][ImsSignal]1th getDefaultImsSignal %p gDefaultImsSignalCnt %d",gDefaultImsSignal.get(),gDefaultImsSignalCnt);
        }
    }

      return gDefaultImsSignal;
}

void deleteDefaultImsSignal()//if no delete it ,we always
{
    AutoMutex _l(gDefaultImsSignalLock);
    gDefaultImsSignalCnt--;
    ALOGI("[deleteDefaultImsSignal]gDefaultImsSignal %p gDefaultImsSignalCnt %d",gDefaultImsSignal.get(),gDefaultImsSignalCnt);
    if (gDefaultImsSignalCnt == 0 &&  gDefaultImsSignal.get() != NULL)   {
        gDefaultImsSignal.clear();
        gDefaultImsSignal = NULL;
    }
}


}
