/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef _3DNR_CUSTOM_BASE_H_
#define _3DNR_CUSTOM_BASE_H_

#include <cutils/properties.h>
#include <math.h>

typedef int           MINT32;
typedef unsigned char MUINT8;
typedef unsigned int  MUINT32;
typedef float         MFLOAT;
typedef int           MBOOL;

enum NR3D_MODE
{
    NR3D_MODE_OFF = 0,
    NR3D_MODE_3DNR_10,
    NR3D_MODE_3DNR_20,
    NR3D_MODE_3DNR_30,
    NR3D_MODE_3DNR_40,
};

#define NR3D_MODE_ENABLE_3DNR_10(x)           (x)|=(1<<NR3D_MODE_3DNR_10)
#define NR3D_MODE_IS_3DNR_10_ENABLED(x)       ((x& (1<<NR3D_MODE_3DNR_10))?true:false)

#define NR3D_MODE_ENABLE_3DNR_20(x)           (x)|=(1<<NR3D_MODE_3DNR_20)
#define NR3D_MODE_IS_3DNR_20_ENABLED(x)       ((x& (1<<NR3D_MODE_3DNR_20))?true:false)

#define NR3D_MODE_ENABLE_3DNR_30(x)           (x)|=(1<<NR3D_MODE_3DNR_30)
#define NR3D_MODE_IS_3DNR_30_ENABLED(x)       ((x& (1<<NR3D_MODE_3DNR_30))?true:false)

#define NR3D_MODE_ENABLE_3DNR_40(x)           (x)|=(1<<NR3D_MODE_3DNR_40)
#define NR3D_MODE_IS_3DNR_40_ENABLED(x)       ((x& (1<<NR3D_MODE_3DNR_40))?true:false)
#define isNR3DUsageMaskEnable(x, mask)        ((x) & (mask))

// ISO value must higher then threshold to turn on 3DNR
#define DEFAULT_NR3D_OFF_ISO_THRESHOLD          400

// gmv ConfThreshold
#define NR3D_GMVX_CONF_LOW_THRESHOLD              20
#define NR3D_GMVX_CONF_HIGH_THRESHOLD             30

#define NR3D_GMVY_CONF_LOW_THRESHOLD              20
#define NR3D_GMVY_CONF_HIGH_THRESHOLD             30

#define NR3D_GYRO_CONF_THRESHOLD                  0

class NR3DCustomBase
{
public:
    enum USAGE_MASK
    {
        USAGE_MASK_NONE         = 0,
        USAGE_MASK_DUAL_ZOOM    = 1 << 0,
        USAGE_MASK_MULTIUSER    = 1 << 1,
        USAGE_MASK_HIGHSPEED    = 1 << 2,
    };
    struct AdjustmentInput
    {
        bool force3DNR;
        int confX, confY;
        int gmvX, gmvY;
        bool isGyroValid;
        float gyroX, gyroY, gyroZ;

        // RSC info
        intptr_t pMV;
        intptr_t pBV; // size is rssoSize
        int  rrzoW,rrzoH;
        int  rssoW,rssoH;
        unsigned int staGMV; // gmv value of RSC
        bool   isRscValid;

        AdjustmentInput() :
            force3DNR(false),
            confX(0), confY(0), gmvX(0), gmvY(0),
            isGyroValid(false), gyroX(0), gyroY(0), gyroZ(0),
            pMV(NULL), pBV(NULL), rrzoW(0), rrzoH(0), rssoW(0), rssoH(0), staGMV(0), isRscValid(false)
        {
        }

        void setGmv(int confX, int confY, int gmvX, int gmvY)
        {
            this->confX = confX;
            this->confY = confY;
            this->gmvX = gmvX;
            this->gmvY = gmvY;
        }

        void setGyro(bool isGyroValid, float gyroX, float gyroY, float gyroZ)
        {
            this->isGyroValid = isGyroValid;
            this->gyroX = gyroX;
            this->gyroY = gyroY;
            this->gyroZ = gyroZ;
        }

        void setRsc(bool isRscValid, intptr_t pMV, intptr_t pBV, int rrzoW, int rrzoH, int rssoW, int rssoH, unsigned int staGMV)
        {
            this->isRscValid = isRscValid;
            this->pMV = pMV;
            this->pBV = pBV;
            this->rrzoW = rrzoW;
            this->rrzoH = rrzoH;
            this->rssoW = rssoW;
            this->rssoH = rssoH;
            this->staGMV = staGMV;
        }
    };

    struct AdjustmentOutput
    {
        bool isGmvOverwritten;
        int gmvX, gmvY;

        AdjustmentOutput() :
            isGmvOverwritten(0),
            gmvX(0), gmvY(0)
        {
        }

        void setGmv(bool isGmvOverwritten, int gmvX, int gmvY)
        {
            this->isGmvOverwritten = isGmvOverwritten;
            this->gmvX = gmvX;
            this->gmvY = gmvY;
        }
    };

    struct AdjustmentState
    {
        // If any state added in the future, store here
        unsigned int unused; // to be removed
    };

protected:
    // DO NOT create instance
    NR3DCustomBase()
    {
    }


    template <typename _T>
    static inline _T __MAX(const _T &a, const _T &b)
    {
        return (a < b ? b : a);
    }

public:
    /* Get 3DNR Mode
     */
    static MUINT32 get3DNRMode(MUINT32 mask)
    {
        (void)mask;
        return NR3D_MODE_OFF;
    }

    // === Feature support ===
    static MBOOL isEnabled3DNR()
    {
        return true;
    }

    static MBOOL isSecure3DNRSupported()
    {
        return false;
    }

    static MBOOL is3DNRSmvrBatchSupported()
    {
        return false;
    }

    static MBOOL isUFBCSupported()
    {
        return false;
    }

    static MINT32 get_3dnr_off_iso_threshold(MUINT8 ispProfile = 0, MBOOL useAdbValue = 0)
    {
        (void)ispProfile; // unused
        (void)useAdbValue; // unused
        return DEFAULT_NR3D_OFF_ISO_THRESHOLD;
    }

    // === Motion related ===
    static void adjust_parameters(
        const AdjustmentInput &input,
        AdjustmentOutput &output,
        AdjustmentState *) // state may be NULL
    {
        output.setGmv(false, input.gmvX, input.gmvY);

        int confXThrL = NR3D_GMVX_CONF_LOW_THRESHOLD;
        int confXThrH = NR3D_GMVX_CONF_HIGH_THRESHOLD;
        int confYThrL = NR3D_GMVY_CONF_LOW_THRESHOLD;
        int confYThrH = NR3D_GMVY_CONF_HIGH_THRESHOLD;
        int confGyro  = NR3D_GYRO_CONF_THRESHOLD;

        if (input.force3DNR)
        {
            confXThrL = ::property_get_int32("vendor.debug.nr3d.confXL", NR3D_GMVX_CONF_LOW_THRESHOLD);
            confXThrH = ::property_get_int32("vendor.debug.nr3d.confXH", NR3D_GMVX_CONF_HIGH_THRESHOLD);
            confYThrL = ::property_get_int32("vendor.debug.nr3d.confYL", NR3D_GMVY_CONF_LOW_THRESHOLD);
            confYThrH = ::property_get_int32("vendor.debug.nr3d.confYH", NR3D_GMVY_CONF_HIGH_THRESHOLD);
            confGyro = ::property_get_int32("vendor.debug.nr3d.confGyro", NR3D_GYRO_CONF_THRESHOLD);
        }

        if ((input.confX <= confXThrL && input.confY <= confYThrH) ||
            (input.confY <= confYThrL && input.confX <= confXThrH))
        {
            output.setGmv(true, 0, 0);
        }
        if (input.isGyroValid)
        {
            float absX = fabs(input.gyroX);
            float absY = fabs(input.gyroY);
            float absZ = fabs(input.gyroZ);
            float gyromax = __MAX(absX, absY);
            gyromax = __MAX(gyromax, absZ);
            float confGyroF = (float)confGyro / 1000.0f; //0.2
            if (gyromax < confGyroF)
                output.setGmv(true, 0, 0);
        }
        if (input.isRscValid)
        {
            // TODO: add calculation
        }
    }

    // DEPRECATED
    // return value: 0 (no adjustment), 1 (adjusted)
    // New chip SHALL use adjust_parameters() instead
    // This function is reserved only for backward-compatible
    static int adjust_3dnr_gmv_by_conf(int force3DNR, const int confX, const int confY, int &outGmvX, int &outGmvY)
    {
        AdjustmentInput adjInput;
        adjInput.force3DNR = force3DNR ? true : false;
        adjInput.setGmv(confX, confY, outGmvX, outGmvY);
        adjInput.isGyroValid = false;

        AdjustmentOutput adjOutput;
        NR3DCustomBase::adjust_parameters(adjInput, adjOutput, NULL);

        if (adjOutput.isGmvOverwritten)
        {
            outGmvX = adjOutput.gmvX;
            outGmvY = adjOutput.gmvY;
        }

        return (adjOutput.isGmvOverwritten ? 1 : 0);
    }
// === mktodo: to be obsoleted ===

    /* EIS version support
     */
    // mktodo: obsolete
    static MBOOL isEnabled3DNR10()
    {
        return false;
    }

    // mktodo: obsolete
    static MBOOL isEnabled3DNR20()
    {
        return false;
    }

    // mktodo: obsolete
    static MBOOL isEnabled3DNR30()
    {
        return false;
    }

    // mktodo: obsolete
    static MBOOL isEnabled3DNR35()
    {
        return false;
    }

    // mktodo: obsolete
    static MBOOL isEnabled3DNR40()
    {
        return false;
    }

    static MBOOL isSupportRSC()
    {
        // return MTRUE if platform has RSC HW and NR3D wants to use RSC HW
        return false;
    }

    static MBOOL isEnabledRSC(MUINT32 mask)
    {
        // return MTRUE if NR3D can enable RSC in usage of mask
        (void)mask;
        return false;
    }

};
#endif /* _3DNR_CUSTOM_BASE_H_ */

