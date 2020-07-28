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
#ifndef _MTKCAM_PIPELINE_EXTENSION_THIRDPARTYFEATURES_H_
#define _MTKCAM_PIPELINE_EXTENSION_THIRDPARTYFEATURES_H_

#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <errno.h>
#include <utils/RefBase.h>


using android::sp;
using android::wp;
using android::IParamsManager;


namespace MTK3rdParty {

// Set IParamsManager to a weak pointer for MTK3rdParty usage.
//  @param param                    The IParamsManager instance.
//  @note This method is thread-safe.
void                                setParamsManager(
                                        const sp<IParamsManager>& param
                                    );


// Try to get IParamsManager. If it doesn't exist, returns nullptr.
//  @return                         IParamsManager or nullptr if not exists.
//  @note This method is thread-safe.
sp<IParamsManager>                  getParamsManager();


class FaceBeauty {
public:
    // Excute 3rd party face beauty (in place buffer)
    //  @param img [in/out]         The input image and also an output image.
    //                              This buffer is supposed to be locked by
    //                              SW read/write.
    //  @return                     POSIX error code, returns 0 if no error.
    static int                      run(
                                        NSCam::IImageBuffer*    pImg,
                                        NSCam::IMetadata*       pAppMeta,
                                        NSCam::IMetadata*       pHalMeta
                                    );

    // Returns if it's necessary to run.
    //  @param pAppMeta             Metadata of app.
    //  @param pHalMeta             Metadata of hal.
    //  @return                     Return 0 for no, otherwise yes.
    static int                      isEnable(
                                        const NSCam::IMetadata*     pAppMeta,
                                        const NSCam::IMetadata*     pHalMeta
                                    );
};


};
#endif//_MTKCAM_PIPELINE_EXTENSION_THIRDPARTYFEATURES_H_
