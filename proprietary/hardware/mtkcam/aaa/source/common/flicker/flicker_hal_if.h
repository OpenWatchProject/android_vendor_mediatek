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

#pragma once

#include <utils/threads.h>
#include <mtkcam/def/common.h>
#include <aaa_error_code.h>
#include "flicker_hal_type.h"

#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
#include <isp_flicker_config.h>
#endif

using namespace android;


namespace NS3Av3 {

    typedef struct {
        int aeExpTime;
        MVOID *pBuf;
    } FlickerInput;

    typedef struct {
        int flickerResult;
    } FlickerOutput;

    typedef enum {
        FLK_ATTACH_PRIO_LOW = 0,
        FLK_ATTACH_PRIO_MEDIUM,
        FLK_ATTACH_PRIO_HIGH,
        FLK_ATTACH_PRIO_NUM
    } FLK_ATTACH_PRIO_T;

    class IFlickerHal
    {
        protected:
            virtual ~IFlickerHal() = 0;

        public:
            static IFlickerHal* getInstance(int sensorDev);

            virtual MRESULT init(MINT32 const i4SensorIdx) = 0;
            virtual MRESULT uninit() = 0;
            virtual MBOOL config() = 0;
            virtual MBOOL reconfig(MVOID *in, MVOID *out) = 0;
            virtual MBOOL reconfig(MUINT32 *tgWidth, MUINT32 *tgHeight, MUINT32 *binWidth, MUINT32 *binHeight) = 0;
            virtual MBOOL start(FLK_ATTACH_PRIO_T prio) = 0;
            virtual MBOOL stop() = 0;

            virtual MBOOL getInfo() = 0;
            virtual int isAttach() = 0;
            virtual int attach(FLK_ATTACH_PRIO_T prio) = 0;
            virtual int detach() = 0;
            virtual int update(int sensorType, FlickerInput *in, FlickerOutput *out) = 0;

            virtual int setTGInfo(int tgInfo, int width, int height) = 0;
            virtual int setSensorMode(int sensorMode, int width, int height) = 0;

            virtual int setFlickerMode(int mode) = 0;
            virtual int getFlickerState(int &state) = 0;
#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
            virtual MVOID  getHWCfgReg(FLKResultConfig_T *pResultConfig);
#endif
    };
};

