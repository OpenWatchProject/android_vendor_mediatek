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

#include "flicker_param.h"
#include "camera_custom_flicker_para.h"
#include <mtkcam/def/common.h>
#include <aaa_error_code.h>
#include "flicker_hal_type.h"

/* algo headers*/
#include "Flicker_type.h"
#include "sequential_testing.h"
#include "FlickerDetection.h"


#define FLK_MAX_MODE_NUM    10
#define FLK_MAX_BIN_NUM     2

#include <utils/Mutex.h>
//using namespace android;
typedef struct {
    int32_t m;
    int32_t b_l;
    int32_t b_r;
    int32_t offset;
} FlickerStats;

typedef struct {
    int32_t flicker_freq[9];
    int32_t flicker_grad_threshold;
    int32_t flicker_search_range;
    int32_t min_past_frames;
    int32_t max_past_frames;
    FlickerStats ev50_l50;
    FlickerStats ev50_l60;
    FlickerStats ev60_l50;
    FlickerStats ev60_l60;
    int32_t ev50_thresholds[2];
    int32_t ev60_thresholds[2];
    int32_t freq_feature_index[2];
} FlickerExtPara;

namespace NS3Av3 {

    class FlickerPlatformAdapter
    {
        public:
            FlickerPlatformAdapter(int32_t sensorDev);
            ~FlickerPlatformAdapter();

        public:
            static FlickerPlatformAdapter *getInstance(int32_t sensorDev);

            int32_t init(int32_t const i4SensorIdx);

            int32_t uninit();

            //int runFunc(EFlickerFunc type, void *inputPara, void *outputPara);



            int32_t getSensorType(int32_t hdrType, int32_t *sensorType);

            int32_t getInfo(MBOOL *flkInfo);

            int32_t setTGInfo(int32_t tgInfo);

            int32_t setWindowInfo(uint32_t *imgW, uint32_t *imgH,
                    uint32_t *winW, uint32_t *winH, uint32_t u4TgW, uint32_t u4TgH);

            int32_t setWindowInfoReconfig(uint32_t *imgW, uint32_t *imgH,
                    uint32_t *winW, uint32_t *winH, uint32_t *tgWidth,
                    uint32_t *tgHeight, uint32_t *binWidth, uint32_t *binHeight);

            int32_t getSensorMode(int32_t sensorScenario, int32_t sensorType, int32_t *sensorMode);

            int32_t getFLKResultConfig(MVOID *p_sFLKResultConfig);

            int32_t init_algo(uint32_t pixelsLine, uint32_t winH, uint32_t winW, uint32_t pixelClkFreq, int32_t sensorType);

            int32_t uninit_algo();

            int32_t setExtPara(FlickerExtPara para);

            int32_t setBuf(int32_t sensorType, MVOID *buf, int32_t currentCycle, int32_t *dataLen, int32_t *imgW, int32_t *imgH);

            int32_t getWindowInfo(int32_t imgW, int32_t imgH, int32_t *winW, int32_t *winH);

            int32_t detectFlicker(int32_t sensorType, int32_t flickerState, int32_t exp);

            int32_t setFlickerState(int32_t state);

            int32_t resetQueue();

            int32_t setLogLevel(int32_t loglevel);
        private:
            int32_t createBuf();
            MVOID releaseBuf();

            //int getSensorMode(FlickerSensorInfo_T *sensorInfo, int *sensorMode);

            MVOID prepareFlickerCfg(int32_t width, int32_t height, FLKWinCFG_T *flkWinCfg);

        private:
            int32_t m_sensorDev;
            int32_t m_sensorId;

            int32_t mTgInfo;

            int32_t *m_pVectorData1;
            int32_t *m_pVectorData2;

            uint32_t m_imgW;
            uint32_t m_imgH;
            uint32_t m_winW;
            uint32_t m_winH;
            int32_t m_dataLen;

            uint32_t m_u4SensorPixelClkFreq;
            uint32_t m_u4PixelsInLine;

            int32_t m_currentCycle;

    };
};
