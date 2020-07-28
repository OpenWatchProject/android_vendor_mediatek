/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

/**
* @file ResultPool4Meta.h
* @brief Declarations of Interfance of Result Pool Manager
*/
#ifndef __RESULT_POOL_4META_H__
#define __RESULT_POOL_4META_H__

#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>

namespace NS3Av3
{

/*********************************************************
 *   Result to Meta structure
 *   xxxResultToMeta_T is for metadata use.
 *   If modules member need to update to metadata. Please add here.
 *********************************************************/
struct HALResultToMeta_T : ResultPoolBase_T
{
    MINT32      i4FrmId;
    MBOOL       fgKeep;
    MBOOL       fgFdEnable;
    MBOOL       fgBadPicture;
    MUINT8      u1SceneMode;
    // stereo warning message
    MINT32      i4StereoWarning;
    MINT32      i4ZoomRatio;

    HALResultToMeta_T()
        : i4FrmId(-1)
        , fgKeep(0)
        , fgFdEnable(0)
        , fgBadPicture(0)
        , u1SceneMode(0)
        , i4StereoWarning(0)
        , i4ZoomRatio(100)
    {}

    MVOID clearMember()
    {
        HALResultToMeta_T();
    }

    // copy operator
    virtual HALResultToMeta_T& operator = (const HALResultToMeta_T& ) = default;

    // move operator
    virtual HALResultToMeta_T& operator = (HALResultToMeta_T&& ) = default;

    // destructor
    virtual ~HALResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_HAL_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct AEResultToMeta_T : ResultPoolBase_T
{
    MUINT8      u1AeState;
    MUINT8      u1AeMode;
    MBOOL       fgAeBvTrigger;
    MINT32      i4AutoHdrResult;
    // sensor
    MINT64      i8SensorRollingShutterSkew;
    MINT64      i8SensorExposureTime;
    MINT64      i8SensorFrameDuration;
    MINT32      i4SensorSensitivity;
    MINT32      i4SensorGain;
    MINT32      i4IspGain;
    MINT32      i4LuxIndex;

    AEResultToMeta_T()
        : u1AeState(0)
        , u1AeMode(0)
        , fgAeBvTrigger(0)
        , i4AutoHdrResult(0)
        , i8SensorRollingShutterSkew(0)
        , i8SensorExposureTime(0)
        , i8SensorFrameDuration(0)
        , i4SensorSensitivity(0)
        , i4SensorGain(0)
        , i4IspGain(0)
        , i4LuxIndex(0)
    {}

    MVOID clearMember()
    {
        AEResultToMeta_T();
    }

    // copy operator
    virtual AEResultToMeta_T& operator = (const AEResultToMeta_T& ) = default;

    // move operator
    virtual AEResultToMeta_T& operator = (AEResultToMeta_T&& ) = default;

    // destructor
    virtual ~AEResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_AE_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct AFResultToMeta_T : ResultPoolBase_T
{
    MUINT8      u1AfState;
    MUINT8      u1LensState;
    // lens
    MFLOAT      fLensAperture;
    MFLOAT      fLensFilterDensity;
    MFLOAT      fLensFocalLength;
    MFLOAT      fLensFocusDistance;
    MFLOAT      fLensFocusRange[2];
    // focus area result
    MINT32      i4FocusAreaSz[2];

    AFResultToMeta_T()
        : u1AfState(0)
        , u1LensState(0)
        , fLensAperture(0)
        , fLensFilterDensity(0)
        , fLensFocalLength(0)
        , fLensFocusDistance(0)
    {
        ::memset( fLensFocusRange, 0, sizeof(MFLOAT)*2);
        ::memset( i4FocusAreaSz, 0, sizeof(MINT32)*2);
    }

    MVOID clearMember()
    {
        AFResultToMeta_T();
    }
    // copy operator
    virtual AFResultToMeta_T& operator = (const AFResultToMeta_T&  o)
    {
        this->u1AfState = o.u1AfState;
        this->u1LensState = o.u1LensState;
        this->fLensAperture = o.fLensAperture;
        this->fLensFilterDensity = o.fLensFilterDensity;
        this->fLensFocalLength = o.fLensFocalLength;
        this->fLensFocusDistance = o.fLensFocusDistance;
        ::memcpy(this->fLensFocusRange, o.fLensFocusRange, sizeof(MFLOAT)*2);
        ::memcpy(this->i4FocusAreaSz, o.i4FocusAreaSz, sizeof(MINT32)*2);
        return *this;
    }

    // move operator
    virtual AFResultToMeta_T& operator = (AFResultToMeta_T&& ) = default;

    // destructor
    virtual ~AFResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_AF_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct AWBResultToMeta_T : ResultPoolBase_T
{
    MUINT8      u1AwbState;
    // color correction
    MINT32      i4AwbGain[3];
    MINT32      i4AwbGainScaleUint;
    MFLOAT      fColorCorrectGain[4];
    MINT32      i4MWBColorTemperatureMax;
    MINT32      i4MWBColorTemperatureMin;
    MINT32      i4AwbAvailableRange[2];

    AWBResultToMeta_T()
        : u1AwbState(0)
        , i4AwbGainScaleUint(0)
        , i4MWBColorTemperatureMax(0)
        , i4MWBColorTemperatureMin(0)
    {
        ::memset( i4AwbGain, 0, sizeof(MINT32)*3);
        ::memset( fColorCorrectGain, 0, sizeof(MFLOAT)*4);
        ::memset( i4AwbAvailableRange, 0, sizeof(MINT32)*2);
    }

    MVOID clearMember()
    {
        AWBResultToMeta_T();
    }
    // copy operator
    virtual AWBResultToMeta_T& operator = (const AWBResultToMeta_T& o)
    {
        this->u1AwbState = o.u1AwbState;
        this->i4AwbGainScaleUint = o.i4AwbGainScaleUint;
        this->i4MWBColorTemperatureMax = o.i4MWBColorTemperatureMax;
        this->i4MWBColorTemperatureMin = o.i4MWBColorTemperatureMin;
        ::memcpy(this->i4AwbGain, o.i4AwbGain, sizeof(MINT32)*3);
        ::memcpy(this->fColorCorrectGain, o.fColorCorrectGain, sizeof(MFLOAT)*4);
        ::memcpy(this->i4AwbAvailableRange, o.i4AwbAvailableRange, sizeof(MINT32)*2);
        return *this;
    }

    // move operator
    virtual AWBResultToMeta_T& operator = (AWBResultToMeta_T&& ) = default;

    // destructor
    virtual ~AWBResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_AWB_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct ISPResultToMeta_T : ResultPoolBase_T
{
    // cam info
    NSIspTuning::RAWIspCamInfo rCamInfo;

    // copy operator
    virtual ISPResultToMeta_T& operator = (const ISPResultToMeta_T& ) = default;

    // move operator
    virtual ISPResultToMeta_T& operator = (ISPResultToMeta_T&& ) = default;

    // destructor
    virtual ~ISPResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_ISP_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct LCSOResultToMeta_T : ResultPoolBase_T
{
    // cam info
    NSIspTuning::ISP_LCS_OUT_INFO_T rLcsOutInfo;

    LCSOResultToMeta_T()
    {
        ::memset( &rLcsOutInfo, 0, sizeof(rLcsOutInfo));
    }

    MVOID clearMember()
    {
        LCSOResultToMeta_T();
    }

    // copy operator
    virtual LCSOResultToMeta_T& operator = (const LCSOResultToMeta_T& ) = default;

    // move LSCResultToMeta_T
    virtual LCSOResultToMeta_T& operator = (LCSOResultToMeta_T&& ) = default;

    // destructor
    virtual ~LCSOResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_LCSO_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct LSCResultToMeta_T : ResultPoolBase_T
{
    // shading map
    MBOOL       fgShadingMapOn;
    MINT32      i4CurrTblIndex;
    MINT32      i4CurrTsfIndex;

    LSCResultToMeta_T()
        : fgShadingMapOn(0)
        , i4CurrTblIndex(0)
        , i4CurrTsfIndex(0)
    {}

    MVOID clearMember()
    {
        LSCResultToMeta_T();
    }

    // copy operator
    virtual LSCResultToMeta_T& operator = (const LSCResultToMeta_T& ) = default;

    // move LSCResultToMeta_T
    virtual LSCResultToMeta_T& operator = (LSCResultToMeta_T&& ) = default;

    // destructor
    virtual ~LSCResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_LSC_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct FLASHResultToMeta_T : ResultPoolBase_T
{
    MUINT8      u1FlashState;
    MBOOL       bCustEnableFlash; // during touch
    MINT32      i4FlashCalResult;    // flash calibration result
    MUINT8      u1SubFlashState;

    FLASHResultToMeta_T()
        : u1FlashState(0)
        , bCustEnableFlash(0)
        , i4FlashCalResult(0)
        , u1SubFlashState(0)
    {}

    MVOID clearMember()
    {
        FLASHResultToMeta_T();
    }

    // copy operator
    virtual FLASHResultToMeta_T& operator = (const FLASHResultToMeta_T& ) = default;

    // move LSCResultToMeta_T
    virtual FLASHResultToMeta_T& operator = (FLASHResultToMeta_T&& ) = default;

    // destructor
    virtual ~FLASHResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_FLASH_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

struct FLKResultToMeta_T : ResultPoolBase_T
{
    MUINT8      u1SceneFlk;

    FLKResultToMeta_T()
        : u1SceneFlk(0)
    {}

    MVOID clearMember()
    {
        FLKResultToMeta_T();
    }

    // copy operator
    virtual FLKResultToMeta_T& operator = (const FLKResultToMeta_T& ) = default;

    // move LSCResultToMeta_T
    virtual FLKResultToMeta_T& operator = (FLKResultToMeta_T&& ) = default;

    // destructor
    virtual ~FLKResultToMeta_T() = default;

    // Get Id
    virtual int getId() const { return E_FLK_RESULTTOMETA; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of HALResultToMeta_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

} //namespace NS3Av3

#endif  //__RESULT_POOL_4META_H__
