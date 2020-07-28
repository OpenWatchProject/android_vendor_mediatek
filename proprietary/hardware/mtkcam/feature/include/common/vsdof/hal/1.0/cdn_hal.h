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
#ifndef CDN_HAL_H_
#define CDN_HAL_H_

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <ui/GraphicBuffer.h>

//For runtime tuning and UT
#define CDN_UNPACK_PROPERTY    "vendor.STEREO.cdn_unpack"
#define CDN_DUMP_PROPERTY      "vendor.STEREO.cdn_dump"

#define CDN_HAL_INIT_CORE      "vendor.STEREO.cdn_hal_init.core"   //1~10
#define CDN_HAL_INIT_ACC       "vendor.STEREO.cdn_hal_init.acc"    //1, 2, 4, 8
#define CDN_HAL_INIT_CONV      "vendor.STEREO.cdn_hal_init.conv"   //0.503900 0.413200 0.564100 (example, 3 floats)
#define CDN_HAL_INIT_SPLIT     "vendor.STEREO.cdn_hal_init.split"  //336 (example)

#define GAIN_IMAGE_SIZE (200)

namespace StereoHAL {
/**
 * \brief Runtime parameteres for CDN algorithm
 * \details isUnPack and outPackSize does not need to set by default
 *
 */
struct CDN_HAL_PARAMS
{
    int isRotate = 0;

    int mode = 0;
    int var = 0;

    float Trans[9];

    int width = 0;
    int height = 0;

    int id = -1;
};

struct CDN_HAL_IO
{
    //unpadded width, height
    int width;
    int height;

    // Input Data
    // Width x Height YUV422 8-bit Mono
    unsigned char*  MonoYUV;
    // Width x Height YUV422 8-bit Bayer
    unsigned char*  BayerYUV;

    // denoise output after CDN
    unsigned char*  output[3];            // Width x Height YUYV 8-bit
};

class CDN_HAL
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
    static CDN_HAL *createInstance();

    /**
     * \brief Destroy instance
     * \details Destroy which is created by createInstance()
     *
     * \param instance instance to destroy
     */
    virtual void destroyInstance() = 0;

    /**
     * \brief Run CDN algorithm
     * \details Run CDN algorithm
     *
     * \param param in input parameter
     * \param buffers in/out buffers
     *
     * \return True if success
     */
    virtual bool CDNHALRun(CDN_HAL_PARAMS &param, CDN_HAL_IO &io) = 0;

protected:
    CDN_HAL() {}
    virtual ~CDN_HAL() {}
};

};  //namespace StereoHAL
#endif
