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
#ifndef BWDN_HAL_H_
#define BWDN_HAL_H_

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <ui/GraphicBuffer.h>

//For runtime tuning and UT
#define BWDN_UNPACK_PROPERTY    "debug.STEREO.bwdn_unpack"
#define BWDN_DUMP_PROPERTY      "debug.STEREO.bwdn_dump"

#define BWDN_HAL_INIT_CORE      "debug.STEREO.bwdn_hal_init.core"   //1~10
#define BWDN_HAL_INIT_ACC       "debug.STEREO.bwdn_hal_init.acc"    //1, 2, 4, 8
#define BWDN_HAL_INIT_CONV      "debug.STEREO.bwdn_hal_init.conv"   //0.503900 0.413200 0.564100 (example, 3 floats)
#define BWDN_HAL_INIT_SPLIT     "debug.STEREO.bwdn_hal_init.split"  //336 (example)

#define GAIN_IMAGE_SIZE (200)

namespace StereoHAL {

/**
 * \brief BWDN HAL init parameters, does not need to set by default
 * \details User can set for UT or manual tuning
 *
 */
struct BWDN_HAL_INIT
{
    MINT32 coreNum;
    MINT32 splitSize;
    MFLOAT warpingMatrix[3];
    MINT32 acc; //value: 1, 2, 4, 8
};

/**
 * \brief Runtime parameteres for BWDN algorithm
 * \details isUnPack and outPackSize does not need to set by default
 *
 */
struct BWDN_HAL_PARAMS
{
    //ISP Info
    int OBOffsetBayer[4]    = {0};
    int OBOffsetMono[4]     = {0};
    int sensorGainBayer     = 0;
    int sensorGainMono      = 0;
    int ispGainBayer        = 0;
    int ispGainMono         = 0;
    int preGainBayer[3]     = {0};
    int isRotate            = 0;
    int bayerOrder          = 0;
    int RA                  = 0;
    int BitMode             = 2;

    //ratio crop offset
    int OffsetX             = 0;
    int OffsetY             = 0;

    //ISO dependent
    int BW_SingleRange      = 0;
    int BW_OccRange         = 0;
    int BW_Range            = 0;
    int BW_Kernel           = 0;
    int B_Range             = 0;
    int B_Kernel            = 0;
    int W_Range             = 0;
    int W_Kernel            = 0;

    //Sensor dependent
    int VSL                 = 0;
    int VOFT                = 0;
    int VGAIN               = 0;

    //information from N3D
    float Trans[9]          = {0};
    int dPadding[2]         = {0};

    //Performance tuning
    int FPREPROC            = 0;
    int FSSTEP              = 0;
    int DblkRto             = 0;
    int DblkTH              = 0;
    int QSch                = 6;

    int isUnPack            = 1;    //no need to set
    int outPackSize         = 0;    //no need to set

    //Buffers
    int dsH                 = 0;
    int dsV                 = 0;

    android::sp<android::GraphicBuffer> *monoProcessedRaw   = nullptr;
    android::sp<android::GraphicBuffer> *bayerProcessedRaw  = nullptr;
    android::sp<android::GraphicBuffer> *bayerW             = nullptr;
    android::sp<android::GraphicBuffer> *depth              = nullptr;    //disparity from Main to Auxi
    float*  bayerGain                                       = NULL;
    float*  monoGain                                        = NULL;
};

struct BWDN_HAL_OUTPUT
{
    android::sp<android::GraphicBuffer> *outBuffer = nullptr;
};

class BWDN_HAL
{
public:
    /**

     * \return
     */
    /**
     * \brief Create a new instance
     * \details Create instance, callers should call destroyInstance() after using it
     *
     * \param initParams Mauanl init parameter, no need to set by default
     * \return A new instance
     */
    static BWDN_HAL *createInstance(BWDN_HAL_INIT *initParams = NULL);

    /**
     * \brief Destroy instance
     * \details Destroy which is created by createInstance()
     *
     * \param instance instance to destroy
     */
    virtual void destroyInstance() = 0;

    /**
     * \brief Run BWDN algorithm
     * \details Run BWDN algorithm
     *
     * \param input in input parameter
     * \param output out output parameter
     *
     * \return True if success
     */
    virtual bool BWDNHALRun(BWDN_HAL_PARAMS &in, BWDN_HAL_OUTPUT &out) = 0;

protected:
    BWDN_HAL() {}
    virtual ~BWDN_HAL() {}
};

};  //namespace StereoHAL
#endif