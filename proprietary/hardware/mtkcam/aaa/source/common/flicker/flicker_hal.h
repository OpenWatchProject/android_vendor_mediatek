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

#include "flicker_hal_if.h"
#include "flicker_param.h"
#include "camera_custom_flicker.h"
#include "camera_custom_flicker_para.h"
#include <aaa_hal_sttCtrl.h>

#define FLK_MAX_BIN_NUM     2

typedef signed long long MMINT64;

#include <utils/Mutex.h>
using namespace android;

namespace NS3Av3 {

    class FlickerHal : public IFlickerHal
    {
        public:
            FlickerHal(int sensorDev);
            ~FlickerHal();

        public:
            static FlickerHal* getInstance(int sensorDev);

            MRESULT init(MINT32 const i4SensorIdx);
            MRESULT uninit();
            MBOOL config();
            MBOOL reconfig(MVOID *in, MVOID *out);
            MBOOL reconfig(MUINT32 *tgWidth, MUINT32 *tgHeight, MUINT32 *binWidth, MUINT32 *binHeight);
            MBOOL start(FLK_ATTACH_PRIO_T prio);
            MBOOL stop();

            MBOOL getInfo();
            int isAttach();
            int attach(FLK_ATTACH_PRIO_T prio);
            int detach();
            int update(int sensorType, FlickerInput *in, FlickerOutput *out);

            int setTGInfo(int tgInfo, int width, int height);
            int setSensorMode(int sensorMode, int width, int height);
            int setFlickerMode(int mode);
            int getFlickerState(int &state);
#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
            MVOID getHWCfgReg(FLKResultConfig_T *pResultConfig);
#endif


        private:
            MINT32 init_algo(int sensorType);
            MINT32 uninit_algo();

            MINT32 setWindowInfo(MUINT32 *imgW, MUINT32 *imgH,
                    MUINT32 *winW, MUINT32 *winH);

            MINT32 setWindowInfoReconfig(MUINT32 *imgW, MUINT32 *imgH,
                    MUINT32 *winW, MUINT32 *winH, MUINT32 *tgWidth, MUINT32 *tgHeight,
                    MUINT32 *binWidth, MUINT32 *binHeight);


            MINT32 analyzeFlickerFrequency(int sensorType, int exp, MVOID *buf);

            int setFlickerState(int state);

            MVOID getTargetParameter(int sensorMode, int ratio, void *para);
            int getFlickerParametersAll(int sensorModeTotal, int binRatioTotal);

        private:
            static Mutex m_attachLock;
            static int m_attachCount;
            static int m_flickerState;

            mutable Mutex m_lock;
            volatile int mUsers;

            MINT32 m_sensorDev;
            int m_sensorIdx;

            int m_isAttach;
            int m_maxAttachNum;

            int mTgInfo;
            MUINT32 m_u4TgWReal;
            MUINT32 m_u4TgHReal;

            int mSensorMode;
            MUINT32 m_u4TgW;
            MUINT32 m_u4TgH;

            MUINT32 m_imgW;
            MUINT32 m_imgH;
            MUINT32 m_winW;
            MUINT32 m_winH;

            MUINT32 m_u4SensorPixelClkFreq;
            MUINT32 m_u4PixelsInLine;

            int m_flickerMode;

            int m_maxDetExpUs;
            int m_currentFrame;
            int m_detectCycle;
            int m_currentCycle;
            int m_alreadyGetFlickerPara;
            Hal3ASttCtrl*          m_p3ASttCtrl;
            FLICKER_CUST_PARA **m_flkParaArray;
    };
};

