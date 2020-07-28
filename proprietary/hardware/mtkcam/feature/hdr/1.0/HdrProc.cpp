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

#include <mtkcam/feature/hdr/1.0/IHdrProc.h>
#include "MyHdr.h"
#include "Hdr.h"
#include <mtkcam/hal/aaa_hal_base.h>

using namespace android;
using namespace NS3A;

/******************************************************************************
 *
 ******************************************************************************/
using namespace NSCam;

class HDRProc : public IHdrProc
{
public:
    HDRProc() { }
    ~HDRProc() { }

    /**
     * @brief Destroy the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID destroyInstance();

    /**
     * @brief Init the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL init();

    /**
     * @brief Uninit the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL uninit();

    MBOOL setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2);
    MBOOL getParam(MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2);

    MBOOL setShotParam(
            MUINT32 w, MUINT32 h, MUINT32 transform,
            MRect& cropRegion, MUINT32 zoomratio);

    MBOOL setJpegParam(
            MUINT32 main_w, MUINT32 main_h, MUINT32 th_w, MUINT32 th_h,
            MUINT32 quality);

    MBOOL prepare();

    MBOOL addInputFrame(MINT32 frame_index,
            const android::sp<IImageBuffer> imgbuf);

    MBOOL addOutputFrame(MINT32 frame_index,
            android::sp<IImageBuffer> imgbuf);

    MBOOL start();

    MBOOL release();

    MBOOL GetBuf(
            IImageBuffer** src0, IImageBuffer** small0,
            IImageBuffer** src1, IImageBuffer** small1);

    MBOOL getHDRCapInfo(MINT32& i4FrameNum,
            Vector<MUINT32>& vu4Eposuretime,
            Vector<MUINT32>& vu4SensorGain,
            Vector<MUINT32>& vu4FlareOffset);

    MVOID setCompleteCallback(HDRProcCompleteCallback_t complete_cb,
            MVOID* user);

private:
    HDRProc_ShotParam mShotParam;
    HDRProc_JpegParam mJpegParam;

/*
    //
    MUINT32 nrtype = queryCapNRType( getCaptureIso(), isMfbShot);
    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);
           );
*/
};


/******************************************************************************
 *
 ******************************************************************************/
IHdrProc* IHdrProc::createInstance()
{
    return new HDRProc();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID HDRProc::destroyInstance()
{
    delete this;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HDRProc::init()
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    HDRProcInit();

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::uninit()
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    HDRProcUnInit();

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    if(paramId <= HDRProcParam_Begin || paramId >= HDRProcParam_Num){
        ALOGE("[HDRProc::setParam] invalid paramId:%d \n",paramId);
        return MFALSE;
    }

    HDRProcSetParam(paramId,iArg1,iArg2);

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::getParam(MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    if(paramId <= HDRProcParam_Begin || paramId >= HDRProcParam_Num){
        ALOGE("[HDRProc::getParam] invalid paramId:%d \n",paramId);
        return MFALSE;
    }

    HDRProcGetParam(paramId,rArg1,rArg2);

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::setShotParam(
        MUINT32 w, MUINT32 h, MUINT32 transform __attribute__((unused)),
        MRect& cropRegion, MUINT32 zoomratio)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    mShotParam.mi4PictureWidth = w;
    mShotParam.mi4PictureHeight = h;
    //mShotParam.u4PictureTransform = transform;
    mShotParam.mScalerCropRegion = cropRegion;
    mShotParam.mu4ZoomRatio = zoomratio;

    ALOGD("[HDRProc::setShotParam] size(%dx%d) crop(%d,%d,%dx%d) zoomratio(%d)",
            mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight,
            cropRegion.leftTop().x, cropRegion.leftTop().y,
            cropRegion.width(), cropRegion.height(),
            mShotParam.mu4ZoomRatio);
    HDRProcSetShotParam((void *)&mShotParam);

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::setJpegParam(MUINT32 main_w, MUINT32 main_h, MUINT32 th_w, MUINT32 th_h,MUINT32 quality)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;

    mJpegParam.mi4JpsWidth = main_w;
    mJpegParam.mi4JpsHeight = main_h;
    mJpegParam.mi4JpegThumbWidth = th_w;
    mJpegParam.mi4JpegThumbHeight = th_h;
    mJpegParam.mu4JpegQuality = quality;

    ALOGD("[HDRProc::setJpegParam] main wxh(%dx%d), tb wxh(%dx%d),Quality:%d\n", mJpegParam.mi4JpsWidth, mJpegParam.mi4JpsHeight, mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight, mJpegParam.mu4JpegQuality);

    HDRProcSetJpegParam((void *)&mJpegParam);

    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::prepare()
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcPrepare();
    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::addInputFrame(MINT32 frame_index, const android::sp<IImageBuffer> imgbuf)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcAddInputFrame(frame_index, imgbuf);
    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL
HDRProc::addOutputFrame(MINT32 frame_index,android::sp<IImageBuffer> imgbuf)
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcAddOutFrame(frame_index,imgbuf);
    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL
HDRProc::start()
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcStart();
    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::release()
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcRelease();
    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HDRProc::GetBuf(IImageBuffer** src0, IImageBuffer** small0, IImageBuffer** src1, IImageBuffer** small1 )
{
	MBOOL ret = MTRUE;
	FUNCTION_LOG_START;
    HDRProcGetBuf( src0,  small0,  src1,  small1 );
    FUNCTION_LOG_END
    return ret;

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL HDRProc::getHDRCapInfo(
        MINT32& i4FrameNum,
        Vector<MUINT32>& vu4Eposuretime,
        Vector<MUINT32>& vu4SensorGain,
        Vector<MUINT32>& vu4FlareOffset)
{
    MBOOL ret = true;
    Vector<CaptureParam_T> rCap3AParam;

    HDRProcGetCapInfo(rCap3AParam, i4FrameNum);

    ALOGD("[HDRProc::getHDRCapInfo] hdrFrameNum(%d)", i4FrameNum);

    ALOGD("[HDRProc::getHDRCapInfo] vCap3AParam.size(%zu)", rCap3AParam.size());
    Vector<CaptureParam_T>::iterator it = rCap3AParam.begin();
    while (it != rCap3AParam.end())
    {
        ALOGD("=================");
        ALOGD("[HDRProc::getHDRCapInfo] u4Eposuretime(%d)", it->u4Eposuretime);
        ALOGD("[HDRProc::getHDRCapInfo] u4AfeGain(%d)", it->u4AfeGain);
        ALOGD("[HDRProc::getHDRCapInfo] u4IspGain(%d)", it->u4IspGain);
        ALOGD("[HDRProc::getHDRCapInfo] u4RealISO(%d)", it->u4RealISO);
        ALOGD("[HDRProc::getHDRCapInfo] u4FlareOffset(%d)", it->u4FlareOffset);

        vu4Eposuretime.push_back(it->u4Eposuretime);
        vu4SensorGain.push_back(it->u4AfeGain);
        vu4FlareOffset.push_back(it->u4FlareOffset);

        it++;
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HDRProc::
setCompleteCallback(HDRProcCompleteCallback_t complete_cb, MVOID* user)
{
	FUNCTION_LOG_START;
    HDRProcSetCallBack(complete_cb,user);
    FUNCTION_LOG_END_MUM
    return ;
}

