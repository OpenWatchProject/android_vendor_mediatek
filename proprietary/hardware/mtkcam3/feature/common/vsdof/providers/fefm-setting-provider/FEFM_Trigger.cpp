/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FEFM_Trigger"
#include <FEFM_Trigger.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <cstring>

CAM_ULOG_DECLARE_MODULE_ID(MOD_MULTICAM_PROVIDER);

using namespace android;

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

Mutex MTKFEFMTrigger::__instanceLock;
Mutex MTKFEFMTrigger::__runLock;
MTKFEFMTrigger* MTKFEFMTrigger::__FEFMTrigger = NULL;

MTKFEFMTrigger*
MTKFEFMTrigger::getInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if(NULL == __FEFMTrigger) {
        __FEFMTrigger = new MTKFEFMTrigger();
    }

    return  __FEFMTrigger;
}

void
MTKFEFMTrigger::destroyInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if( __FEFMTrigger ) {
        delete __FEFMTrigger ;
        __FEFMTrigger = NULL;
    }
}

MTKFEFMTrigger::MTKFEFMTrigger()
{
}

MTKFEFMTrigger::~MTKFEFMTrigger()
{
}

void
MTKFEFMTrigger::FEFMTrigger_init( bool isMainAF, bool isAuxiAF )
{
    focus_type_main = (isMainAF) ? FEFM_TRIGGER_FOCUS_AF : FEFM_TRIGGER_FOCUS_FF;
    focus_type_auxi = (isAuxiAF) ? FEFM_TRIGGER_FOCUS_AF : FEFM_TRIGGER_FOCUS_FF;

    ::memset(__afQueue, 0, sizeof(__afQueue));
    ::memset(__ffQueue, 0, sizeof(__ffQueue));
}

void
MTKFEFMTrigger::FEFMTrigger_set_reqular_trigger( int count )
{
    para_reqular_trigger_count = count ;
}

// entrie function
int
MTKFEFMTrigger::FEFMTrigger_query( int main1MagicNumber, int main2MagicNumber, int af_valid_main, int af_valid_auxi )
{
    Mutex::Autolock lock(__runLock);

    if( focus_type_main == FEFM_TRIGGER_FOCUS_FF ) // FF+FF
    {
        return __queryFF(main1MagicNumber);
    }
    else
    {
        return __queryAF(main1MagicNumber, main2MagicNumber, af_valid_main, af_valid_auxi);
    }
}

int
MTKFEFMTrigger::FEFMTrigger_get_debug_info( )
{
    int tmp = 0 ;
    tmp = (tmp<<1) + ((focus_type_main == 0)?(0):(1)) ; // bit 1
    tmp = (tmp<<1) + ((focus_type_auxi == 0)?(0):(1)) ; // bit 2
    tmp = (tmp<<5) + ((para_reqular_trigger_count>31)?(31):(para_reqular_trigger_count)) ; // bit 3~7
    tmp = (tmp<<5) + ((reqular_trigger_count>31)?(31):(reqular_trigger_count)) ; // bit 8~12
    tmp = (tmp<<1) + ((af_valid_main_now == 0)?(0):(1)) ; // bit 13
    tmp = (tmp<<1) + ((af_valid_main_pre == 0)?(0):(1)) ; // bit 14
    tmp = (tmp<<1) + ((af_valid_auxi_now == 0)?(0):(1)) ; // bit 15
    tmp = (tmp<<1) + ((af_valid_auxi_pre == 0)?(0):(1)) ; // bit 16

    return tmp ;
}

int
MTKFEFMTrigger::__queryAF( int main1MagicNumber, int main2MagicNumber, int af_valid_main, int af_valid_auxi )
{
    int currentIndex;
    if(main1MagicNumber > __afQueue[__curIndex].magicNumber[0]) {
        //Insert mode
        if(__curIndex + 1 < CHECK_QUEUE_SIZE) {
            __curIndex++;
        } else {
            __curIndex = 0;
        }

        CheckQueueElement &elm = __afQueue[__curIndex];
        elm.magicNumber[0] = main1MagicNumber;
        elm.magicNumber[1] = main2MagicNumber;
        elm.isAFValid[0]   = af_valid_main;
        elm.isAFValid[1]   = af_valid_auxi;

        currentIndex = __curIndex;
    } else {
        //Search mode
        currentIndex = __curIndex;
        bool isFound = false;
        for(int si = CHECK_QUEUE_SIZE-1; si >= 0; --si) {
            if(__afQueue[currentIndex].magicNumber[0] == main1MagicNumber &&
               __afQueue[currentIndex].magicNumber[1] == main2MagicNumber)
            {
                isFound = true;
                break;
            }

            currentIndex--;
            if(currentIndex < 0) {
                currentIndex = CHECK_QUEUE_SIZE - 1;
            }
        }

        MY_LOGE_IF(!isFound, "Cannot find magicNumber %d %d in check list", main1MagicNumber, main2MagicNumber);
    }

    int lastIndex = currentIndex - 1;
    if(lastIndex < 0) {
        lastIndex = CHECK_QUEUE_SIZE - 1;
    }

    af_valid_main_pre = __afQueue[lastIndex].isAFValid[0];
    af_valid_main_now = __afQueue[currentIndex].isAFValid[0];

    af_valid_auxi_pre = __afQueue[lastIndex].isAFValid[1];
    af_valid_auxi_now = __afQueue[currentIndex].isAFValid[1];

    if( focus_type_auxi == FEFM_TRIGGER_FOCUS_FF ) // AF+FF
    {
        if( af_valid_main_pre == 0 && af_valid_main_now == 1 ) {    // just focused
            return FEFM_TRIGGER_YES ;
        } else {
            return FEFM_TRIGGER_NO ;
        }
    }
    else // AF+AF
    {
        if( (af_valid_main_pre == 0 && af_valid_main_now == 1 && af_valid_auxi_now == 1) ||
            (af_valid_auxi_pre == 0 && af_valid_auxi_now == 1 && af_valid_main_now == 1) ) // both just focused
        {
            return FEFM_TRIGGER_YES ;
        } else {
            return FEFM_TRIGGER_NO ;
        }
    }
}

int
MTKFEFMTrigger::__queryFF( int main1MagicNumber )
{
    int currentIndex;
    if(main1MagicNumber > __ffQueue[__curIndex].magicNumber) {
        //Insert mode
        if(__curIndex + 1 < CHECK_QUEUE_SIZE) {
            __curIndex++;
        } else {
            __curIndex = 0;
        }

        CheckQueueElement_FF &elm = __ffQueue[__curIndex];
        elm.magicNumber = main1MagicNumber;
        reqular_trigger_count++;
        if(reqular_trigger_count >= para_reqular_trigger_count) {
            elm.isTrigger = true;
            reqular_trigger_count = 0;
        } else {
            elm.isTrigger = false;
        }

        currentIndex = __curIndex;
    } else {
        //Search mode
        currentIndex = __curIndex;
        bool isFound = false;
        for(int si = CHECK_QUEUE_SIZE-1; si >= 0; --si) {
            if(__ffQueue[currentIndex].magicNumber == main1MagicNumber) {
                isFound = true;
                break;
            }

            currentIndex--;
            if(currentIndex < 0) {
                currentIndex = CHECK_QUEUE_SIZE - 1;
            }
        }

        MY_LOGE_IF(!isFound, "Cannot find magicNumber %d in check list", main1MagicNumber);
    }

    return __ffQueue[currentIndex].isTrigger;
}
