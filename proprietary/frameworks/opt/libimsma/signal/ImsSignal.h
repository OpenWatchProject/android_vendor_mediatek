#ifndef IMS_Signal_H
#define IMS_Signal_H

#undef LOG_TAG
#define LOG_TAG "[VT][ImsSig]"
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <cutils/properties.h>
#include <stdint.h>
#include <utils/KeyedVector.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>

//qian.dong(mk80366)

namespace android {

/**
 * Class used for check ims Signal
 */

/* CMCC
good
	RSRP -80dBm~ -90dBm
	SINR 15 dB~20dB
medium
	RSRP -90dBm~ -100dBm
	SINR 5dB~10dB
poor
	RSRP -110dBm~ -120dBm
	SINR -3dB~0dB

*/

struct ImsSignal;

struct ImsSignal: public AHandler {
    enum {
        Signal_STRENGTH_NONE_OR_UNKNOWN = 0,
        Signal_STRENGTH_POOR,
        Signal_STRENGTH_MODERATE ,
        Signal_STRENGTH_GOOD  ,
        Signal_STRENGTH_GREAT ,
    };
    enum {
        kWhatImsSignalPooling = 1,
    };



    ImsSignal() {
        ALOGI("[VT][ImsSignal]create ImsSignal ");
        mTimedMsSignal1.clear();
        mTimedMsSignal2.clear();
        mTimedMsSignal3.clear();
        mTimedMsSignal4.clear();

        mTimedMsSignalValidCount = 3;
        mTimedGapThresholdMs = 2000ll * mTimedMsSignalValidCount;
        ALOGI("[VT][ImsSignal]create ImsSignal  mTimedMsSignalValidCount =3,mTimedGapThresholdMs =3s");
        char value[PROPERTY_VALUE_MAX];

        if(property_get("persist.vendor.vt.Signal", value, NULL)) {
            mTimedMsSignalValidCount= atoi(value);
            ALOGI("[VT][ImsSignal]reset ImsSignal  mTimedMsSignalValidCount =%d,mTimedGapThresholdMs  = %lld s" ,
                  mTimedMsSignalValidCount,mTimedGapThresholdMs/1000ll);
        }
    }
    const char *signalToString(int32_t signal) {
        switch(signal) {
        case Signal_STRENGTH_GREAT:
            return "Signal_STRENGTH_GREAT";
        case Signal_STRENGTH_GOOD:
            return "Signal_STRENGTH_GOOD";
        case Signal_STRENGTH_MODERATE:
            return "Signal_STRENGTH_MODERATE";
        case Signal_STRENGTH_POOR:
            return "Signal_STRENGTH_POOR";
        case Signal_STRENGTH_NONE_OR_UNKNOWN:
            return "Signal_STRENGTH_NONE_OR_UNKNOWN";
        default:
            return "Unknown signal";
        }
    }
    int32_t signalToBitrateRatio(int32_t signal) {
        switch(signal) {
        case Signal_STRENGTH_GREAT:
            return 100;
        case Signal_STRENGTH_GOOD:
            return 75;
        case Signal_STRENGTH_MODERATE:
            return 50;
        case Signal_STRENGTH_POOR:
            return 25;
        case Signal_STRENGTH_NONE_OR_UNKNOWN:
            return 100;
        default:
            return 100;
        }
    }
    void start(void) {
        mLooper= new ALooper;
        mLooper->setName("ImsSignal_looper");
        mLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
        mLooper->registerHandler(this);
        //post signal
        sp<AMessage> signalMsg = new AMessage(kWhatImsSignalPooling, this);
        signalMsg->post(1000000ll);
    }


    bool  feedSignal(uint32_t simID,int32_t *itsSignal) {
        int32_t rsrpIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;
        int32_t snrIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;

        CHECK(simID <= 4);
        char getSignalString[256];
        sprintf(getSignalString,"vendor.ril.nw.signalstrength.lte.%d",simID);
        ALOGV("[VT][ImsSignal]feedSignal  simID %d,getSignalString %s",simID,getSignalString);
        int32_t RSRP = -140 ;
        int32_t SINR = -200 ;


        //sprintf(getSignalString,"ril.nw.signalstrength.lte" );//just for test

        char value[PROPERTY_VALUE_MAX];

        if(property_get(getSignalString, value, NULL)) {
            char* SINRStr = NULL;
            char* RSRPStr = NULL;
            //ALOGE("[VT][ImsSignal]feedSignal  value %s",value );

            RSRPStr = strtok(value, ",");

            if(RSRPStr != NULL) {
                SINRStr = strtok(NULL, ",");
            } else {
                ALOGE("[VT][ImsSignal][feedSignal]wrong1 simID %d value %s",simID,value);
                return false;
            }

            //ALOGE("[VT][ImsSignal]SINRStr %s  value %s RSRPStr  %s",SINRStr,value,RSRPStr );

            RSRP = atoi(RSRPStr);
            SINR = atoi(SINRStr);

        } else {
            ALOGW("[VT][ImsSignal][feedSignal]not get simID %d",simID);
            return false;
        }

        //INVALID value 0x7FFFFFFF
        if(RSRP > -44) {
            rsrpIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;//too good to believe or invalid value set
        } else if(RSRP >= -85) {
            rsrpIconLevel = Signal_STRENGTH_GREAT;
        } else if(RSRP >= -95) {
            rsrpIconLevel = Signal_STRENGTH_GOOD;
        } else if(RSRP >= -105) {
            rsrpIconLevel = Signal_STRENGTH_MODERATE;
        } else if(RSRP >= -115) {
            rsrpIconLevel = Signal_STRENGTH_POOR;
        } else if(RSRP >= -140) { //too bad to use
            rsrpIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;
        }

        if(SINR > 300) {
            snrIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;//too good to believe or invalid value set
        } else if(SINR >= 130) {
            snrIconLevel = Signal_STRENGTH_GREAT;
        } else if(SINR >= 45) {
            snrIconLevel = Signal_STRENGTH_GOOD;
        } else if(SINR >= 10) {
            snrIconLevel = Signal_STRENGTH_MODERATE;
        } else if(SINR >= -30) {
            snrIconLevel = Signal_STRENGTH_POOR;
        } else if(SINR >= -200) {
            snrIconLevel = Signal_STRENGTH_NONE_OR_UNKNOWN;
        }

        //use the worse one as result
        int32_t nowSignal = (rsrpIconLevel>snrIconLevel)?rsrpIconLevel:snrIconLevel;
        *itsSignal = nowSignal;

        ALOGI("[VT][ImsSignal][feedSignal]simID=%d,RSRP=%d,SINR=%d,nowSignal=%s ",simID,RSRP,SINR,signalToString(nowSignal));
        return true;
    }
    //return worse case
    bool getSignal(uint32_t simID,int32_t *longSignal,int32_t *shortSignal) {
        bool stable = false;
        *longSignal = Signal_STRENGTH_NONE_OR_UNKNOWN;
        *shortSignal = Signal_STRENGTH_NONE_OR_UNKNOWN;

        Mutex::Autolock autoLock(mLock);
        KeyedVector<int64_t,int32_t> tempTimedMsSignal;

        if(simID == 0) {
            tempTimedMsSignal = mTimedMsSignal1;
        } else if(simID == 1) {
            tempTimedMsSignal = mTimedMsSignal2;
        } else if(simID == 2) {
            tempTimedMsSignal = mTimedMsSignal3;
        } else if(simID == 3) {
            tempTimedMsSignal = mTimedMsSignal4;
        } else {
            ALOGE("[VT][ImsSignal]getSignal fail-simID %d ",simID);
        }




        if(tempTimedMsSignal.size() == 0) {
            ALOGI("[VT][ImsSignal]getSignal size 0 ");
            return false;
        } else if(tempTimedMsSignal.size() < (size_t)mTimedMsSignalValidCount) {
            *shortSignal = (tempTimedMsSignal.editValueAt(tempTimedMsSignal.size()-1));
            ALOGI("[VT][ImsSignal]getSignal simID %d,size %zu < %d",simID,tempTimedMsSignal.size() ,mTimedMsSignalValidCount);
            ALOGI("[VT][ImsSignal]getSignal simID %d,longSignal [%d][%s]  shortSignal [%d][%s] stable %d",
                  simID,*longSignal,signalToString(*longSignal),*shortSignal,signalToString(*shortSignal),stable);
            return false;
        }

        stable = true;
        *longSignal = Signal_STRENGTH_GREAT;

        //scan all signal and time to find worse case and check if stable case
        for(size_t i = 0; i < tempTimedMsSignal.size()-1; i++) {
            int32_t Signal1 = (tempTimedMsSignal.editValueAt(i));
            int32_t Signal2 = (tempTimedMsSignal.editValueAt(i+1));

            if(Signal1  != Signal2) {
                stable = false;
            }

            if(Signal1  <  *longSignal) {
                *longSignal  = Signal1;
            }

            if(Signal2  <  *longSignal) {
                *longSignal  = Signal2;
            }
        }

        *shortSignal = (tempTimedMsSignal.editValueAt(tempTimedMsSignal.size()-1));

        int64_t time1 = (tempTimedMsSignal.keyAt(0));
        int64_t time2 = (tempTimedMsSignal.keyAt(tempTimedMsSignal.size()-1));

        if((time2 - time1) > mTimedGapThresholdMs) {
            ALOGE("[VT][ImsSignal]getSignal simID %d,GapMs %lld s ",simID,(time2 - time1) /1000ll);
        }

        ALOGI("[VT][ImsSignal]getSignal simID %d,longSignal [%d][%s]  shortSignal [%d][%s] stable %d",
              simID,*longSignal,signalToString(*longSignal),*shortSignal,signalToString(*shortSignal),stable);
        return stable;
    }

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg) {
        switch(msg->what()) {
        case kWhatImsSignalPooling: {
            /*//just for test
            int32_t sigLong,sigShort;
            bool stable ;
            stable = getSignal(0,&sigLong,&sigShort);
            stable = getSignal(1,&sigLong,&sigShort);
            stable = getSignal(2,&sigLong,&sigShort);
            stable = getSignal(3,&sigLong,&sigShort);
            */

            int32_t tempSignal = 0;

            int64_t nowMs = ALooper::GetNowUs()/1000ll;
            {
                Mutex::Autolock autoLock(mLock);

                if(feedSignal(1,&tempSignal)) {
                    mTimedMsSignal1.add(nowMs,tempSignal);

                    if(mTimedMsSignal1.size() > (size_t)mTimedMsSignalValidCount) {
                        mTimedMsSignal1.removeItemsAt(0);
                    }
                }

                if(feedSignal(2,&tempSignal)) {
                    mTimedMsSignal2.add(nowMs,tempSignal);

                    if(mTimedMsSignal2.size() > (size_t)mTimedMsSignalValidCount) {
                        mTimedMsSignal2.removeItemsAt(0);
                    }
                }

                if(feedSignal(3,&tempSignal)) {
                    mTimedMsSignal3.add(nowMs,tempSignal);

                    if(mTimedMsSignal3.size() > (size_t)mTimedMsSignalValidCount) {
                        mTimedMsSignal3.removeItemsAt(0);
                    }
                }

                if(feedSignal(4,&tempSignal)) {
                    mTimedMsSignal4.add(nowMs,tempSignal);

                    if(mTimedMsSignal4.size() > (size_t)mTimedMsSignalValidCount) {
                        mTimedMsSignal4.removeItemsAt(0);
                    }
                }
            }


            msg->post(1000000ll);// 1s
        }
        }
    }
    virtual ~ImsSignal() {
        ALOGI("[VT][~ImsSignal]");
    }

private:
    mutable     Mutex     mLock;
    sp<ALooper>           mLooper;
    KeyedVector<int64_t,int32_t> mTimedMsSignal1;
    KeyedVector<int64_t,int32_t> mTimedMsSignal2;
    KeyedVector<int64_t,int32_t> mTimedMsSignal3;
    KeyedVector<int64_t,int32_t> mTimedMsSignal4;

    int32_t mTimedMsSignalValidCount;
    int64_t mTimedGapThresholdMs;
};


sp<ImsSignal> getDefaultImsSignal();
void deleteDefaultImsSignal();

}  // namespace android

#endif // IMS_Signal_H
