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
#ifndef __PORTRAIT_H__
#define __PORTRAIT_H__

#include <utils/RefBase.h>
#include <vector>

using namespace android;

#define PORTRAIT_DEFAULT_MODEL  (mtkdl::PortraitNetFactory::M1_LR_MAX_V14_7_90K)
#define PORTRAIT_CPP_API __attribute__((visibility ("default")))

namespace mtkdl
{
    class PORTRAIT_CPP_API PortraitNet;

    class PORTRAIT_CPP_API PortraitNetFactory
    {
    public:
        typedef enum
        {
            M1_LR_MAX_V14_7_90K,
            NUM_MODELS
        } E_MODEL_T;
        static PortraitNetFactory& get();
        sp<PortraitNet> create(E_MODEL_T eModel, bool use_fp32=false);
    private:
        PortraitNetFactory();
        ~PortraitNetFactory(){}
    };

    class PORTRAIT_CPP_API PortraitNet: public virtual RefBase
    {
    public:
        virtual ~PortraitNet(){}
        virtual std::vector<float> forward(bool use_yuv = false) = 0;
        virtual bool run(float thr = 0, bool use_yuv = false) = 0;
        virtual int get_input_width() const = 0;
        virtual int get_input_height() const = 0;
        virtual PortraitNetFactory::E_MODEL_T get_model() const = 0;
        virtual void* map_input_buffer() = 0;
        virtual void unmap_input_buffer() = 0;
    };
}
#endif //__PORTRAIT_H__
