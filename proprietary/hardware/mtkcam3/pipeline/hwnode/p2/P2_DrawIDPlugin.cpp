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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_DrawIDPlugin.h"
#include "P2_DebugControl.h"
#define P2_CLASS_TAG    DrawIDPlugin
#define P2_TRACE        TRACE_P2_DRAWID_PLUGIN
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

#include <algorithm>
#include <string>
#include <cutils/properties.h>

using namespace std;

namespace P2
{

P2DrawIDPlugin::P2DrawIDPlugin()
{
    TRACE_FUNC_ENTER();
    mDrawIDUtil = DebugDrawID::createInstance();

    char property_buf[PROPERTY_VALUE_MAX];
    property_get("debug.cam.draw.ctrl", property_buf, "");
    string property_str(property_buf);
    for_each(property_str.begin(), property_str.end(), [](char &c){ c = toupper(c);});

    mDrawCtrl.en = 0;
    if(property_str.find("MAGIC")!=string::npos) mDrawCtrl.en_bit.magic_num = 1;
    if(property_str.find("ISO")  !=string::npos) mDrawCtrl.en_bit.iso = 1;
    if(property_str.find("LV")   !=string::npos) mDrawCtrl.en_bit.lv  = 1;
    if(property_str.find("DAC")  !=string::npos) mDrawCtrl.en_bit.dac = 1;


    TRACE_FUNC_EXIT();
}

P2DrawIDPlugin::~P2DrawIDPlugin()
{
    TRACE_FUNC_ENTER();
    if( mDrawIDUtil )
    {
        mDrawIDUtil->destroyInstance();
        mDrawIDUtil = NULL;
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2DrawIDPlugin::onPlugin(const P2Img *img)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mDrawIDUtil &&
        isValid(img) &&
        img->getDir() & IO_DIR_OUT &&
        ( img->getID() == OUT_YUV || img->getID() == OUT_JPEG_YUV))
    {
        std::vector<IImageBuffer*> buffers = img->getIImageBufferPtrs();
        MBOOL drawTS = buffers.size() > 1;
        for( IImageBuffer *ptr : buffers )
        {
            if( ptr )
            {
                TRACE_FUNC("draw + img(%s)", img->getHumanName());
                if(mDrawCtrl.en)
                {
                    unsigned offset_x  = 200;
                    unsigned offset_y  = 200;
                    unsigned linePixel = 10;
                    unsigned maskH     = 5; /* Reference : DebugDrawID module */

                    const P2SensorData& sensor_data = img->getSensorData();

                    if(mDrawCtrl.en_bit.magic_num)
                    {
                        mDrawIDUtil->draw("NUM"+to_string(sensor_data.mMagic3A),
                                          offset_x,
                                          offset_y += linePixel*maskH + 10,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0),
                                          linePixel);
                    }

                    if(mDrawCtrl.en_bit.dac)
                    {
                        mDrawIDUtil->draw("DAC"+to_string(sensor_data.mLensPosition),
                                          offset_x,
                                          offset_y += linePixel*maskH + 10,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0),
                                          linePixel);
                    }

                    if(mDrawCtrl.en_bit.lv)
                    {
                        mDrawIDUtil->draw("LV"+to_string(sensor_data.mLV),
                                          offset_x,
                                          offset_y += linePixel*maskH + 10,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0),
                                          linePixel);
                    }

                    if(mDrawCtrl.en_bit.iso)
                    {

                        mDrawIDUtil->draw("ISO"+to_string(sensor_data.mISO),
                                          offset_x,
                                          offset_y += linePixel*maskH + 10,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0),
                                          linePixel);
                    }

                    if( drawTS )
                    {
                        mDrawIDUtil->draw((MUINT32)(ptr->getTimestamp()/1000000),
                                          5,
                                          offset_x,
                                          offset_y += linePixel*maskH + 10,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0));
                    }

                }
                else
                {
                    mDrawIDUtil->draw(img->getMagic3A(),
                                      (char*)ptr->getBufVA(0),
                                      ptr->getImgSize().w,
                                      ptr->getImgSize().h,
                                      ptr->getBufStridesInBytes(0),
                                      ptr->getBufSizeInBytes(0));

                    if( drawTS )
                    {
                        mDrawIDUtil->draw((MUINT32)(ptr->getTimestamp()/1000000),
                                          5,
                                          100,
                                          100,
                                          (char*)ptr->getBufVA(0),
                                          ptr->getImgSize().w,
                                          ptr->getImgSize().h,
                                          ptr->getBufStridesInBytes(0),
                                          ptr->getBufSizeInBytes(0));
                    }
                }
                TRACE_FUNC("draw -");

            }
            else
            {
                MY_LOGW("invalid ptr=NULL img=%p", img);
            }
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2DrawIDPlugin::isEnabled() const
{
    return (mDrawIDUtil != NULL) ? mDrawIDUtil->needDraw() : MFALSE;
}

} // namespace
