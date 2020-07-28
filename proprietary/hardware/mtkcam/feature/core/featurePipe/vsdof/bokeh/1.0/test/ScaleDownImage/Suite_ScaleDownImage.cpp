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
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)
#define LOG_TAG "MDP_TEST"

#include <gtest/gtest.h>
#include <featurePipe/core/include/BufferPool.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <mtkcam/def/ImageFormat.h>
//
#include "imgi_640x480_yuy2.h"
//
#include <DpBlitStream.h>
//

using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
// sava file utility
static bool do_mkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
        {
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        return  false;
    }
    //
    return  true;
}
//
bool
makePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    char*copypath = strdup(path);
    char*pp = copypath;
    char*sp;
    while ( ret && 0 != (sp = strchr(pp, '/')) )
    {
        if (sp != pp)
        {
            *sp = '\0';
            ret = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (ret)
        ret = do_mkdir(path, mode);
    free(copypath);
    return  ret;
}
//
// test subject
IImageBuffer* createImgBufUsingExample()
{
#warning "Re-write here, do not use IMEM module"
#if 0
    IMemDrv* mpImemDrv=NULL;
	mpImemDrv=IMemDrv::createInstance();
	mpImemDrv->init();
    IMEM_BUF_INFO imgiBuf;
	MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
	IImageBuffer* srcBuffer;
	MUINT32 bufStridesInBytes[3] = {1280, 0, 0};
    imgiBuf.size=sizeof(g_imgi_array_640x480_yuy2);
	mpImemDrv->allocVirtBuf(&imgiBuf);
    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_yuy2), imgiBuf.size);
     //imem buffer 2 image heap
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
	srcBuffer = pHeap->createImageBuffer();
    srcBuffer->incStrong(srcBuffer);
    srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    return srcBuffer;
#else
    return NULL;
#endif
}
//
TEST(TestBufferPool, Alloc)
{
    // create src image
    IImageBuffer* srcImg = createImgBufUsingExample();
    makePath("/sdcard/bokehPipe/", 0660);
    srcImg->saveToFile("/sdcard/bokehPipe/setBufferVal_128.yuv");
    // create dst buffer from buffer pool
    android::sp<ImageBufferPool> pPool = nullptr;
    pPool = ImageBufferPool::create("SetBufferVal", 240, 136, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    pPool->allocate();
    SmartImageBuffer sbImage = pPool->request();
    // scale down
    DpBlitStream* pDpBlitStream = new DpBlitStream();
    //***************************src YUY2****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    int plane_num = 1;
    src_addr_list[0] = srcImg->getBufVA(0);
    src_size_list[0] = srcImg->getBufSizeInBytes(0);
    pDpBlitStream->setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    pDpBlitStream->setSrcConfig(srcImg->getImgSize().w,
                                srcImg->getImgSize().h,
                                1280,
                                0,
                                DP_COLOR_YUYV,
                                DP_PROFILE_FULL_BT601,
                                eInterlace_None,
                                0,
                                DP_SECURE_NONE,
                                false);
    //***************************dst YV12********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    plane_num = 3;
    dst_addr_list[0] = sbImage->mImageBuffer->getBufVA(0);
    dst_addr_list[1] = sbImage->mImageBuffer->getBufVA(1);
    dst_addr_list[2] = sbImage->mImageBuffer->getBufVA(2);

    dst_size_list[0] = sbImage->mImageBuffer->getBufSizeInBytes(0);
    dst_size_list[1] = sbImage->mImageBuffer->getBufSizeInBytes(1);
    dst_size_list[2] = sbImage->mImageBuffer->getBufSizeInBytes(2);
    pDpBlitStream->setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    pDpBlitStream->setDstConfig(sbImage->mImageBuffer->getImgSize().w,
                        sbImage->mImageBuffer->getImgSize().h,
                        sbImage->mImageBuffer->getBufStridesInBytes(0),
                        sbImage->mImageBuffer->getBufStridesInBytes(1),
                        DP_COLOR_YV12,
                        DP_PROFILE_FULL_BT601,
                        eInterlace_None,
                        0,
                        DP_SECURE_NONE,
                        false);
    pDpBlitStream->setRotate(0);
    //*******************************************************************//
    if (pDpBlitStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
    }
    delete pDpBlitStream;
    // save scale down image
    sbImage->mImageBuffer->saveToFile("sdcard/bokehPipe/scaleDown.yuv");
    // create dst buffer from buffer pool
    android::sp<ImageBufferPool> pRotPool = nullptr;
    pRotPool = ImageBufferPool::create("RotBufferVal", 136, 240, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    pRotPool->allocate();
    SmartImageBuffer sbRotImage = pRotPool->request();
    LOG_DBG("CKH: MDP init +");
    pDpBlitStream = new DpBlitStream();
    //***************************src YUY2****************************//
    plane_num = 3;
    src_addr_list[0] = sbImage->mImageBuffer->getBufVA(0);
    src_addr_list[1] = sbImage->mImageBuffer->getBufVA(1);
    src_addr_list[2] = sbImage->mImageBuffer->getBufVA(2);
    src_size_list[0] = sbImage->mImageBuffer->getBufSizeInBytes(0);
    src_size_list[1] = sbImage->mImageBuffer->getBufSizeInBytes(1);
    src_size_list[2] = sbImage->mImageBuffer->getBufSizeInBytes(2);
    pDpBlitStream->setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    pDpBlitStream->setSrcConfig(sbImage->mImageBuffer->getImgSize().w,
                                sbImage->mImageBuffer->getImgSize().h,
                                sbImage->mImageBuffer->getBufStridesInBytes(0),
                                sbImage->mImageBuffer->getBufStridesInBytes(1),
                                DP_COLOR_YV12,
                                DP_PROFILE_FULL_BT601,
                                eInterlace_None,
                                0,
                                DP_SECURE_NONE,
                                false);
    LOG_DBG("CKH: sbImage->mImageBuffer->getBufStridesInBytes(0) %d", sbImage->mImageBuffer->getBufStridesInBytes(0));
    LOG_DBG("CKH: sbImage->mImageBuffer->getBufStridesInBytes(1) %d", sbImage->mImageBuffer->getBufStridesInBytes(1));
    //***************************dst YV12********************************//
    plane_num = 3;
    dst_addr_list[0] = sbRotImage->mImageBuffer->getBufVA(0);
    dst_addr_list[1] = sbRotImage->mImageBuffer->getBufVA(1);
    dst_addr_list[2] = sbRotImage->mImageBuffer->getBufVA(2);

    dst_size_list[0] = sbRotImage->mImageBuffer->getBufSizeInBytes(0);
    dst_size_list[1] = sbRotImage->mImageBuffer->getBufSizeInBytes(1);
    dst_size_list[2] = sbRotImage->mImageBuffer->getBufSizeInBytes(2);
    pDpBlitStream->setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    pDpBlitStream->setDstConfig(sbRotImage->mImageBuffer->getImgSize().w,
                        sbRotImage->mImageBuffer->getImgSize().h,
                        sbRotImage->mImageBuffer->getBufStridesInBytes(0),
                        sbRotImage->mImageBuffer->getBufStridesInBytes(1),
                        DP_COLOR_YV12,
                        DP_PROFILE_FULL_BT601,
                        eInterlace_None,
                        0,
                        DP_SECURE_NONE,
                        false);
    pDpBlitStream->setRotate(90);
    //*******************************************************************//
    LOG_DBG("CKH: MDP start +");
    if (pDpBlitStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
    }
    LOG_DBG("CKH: MDP start -");
    delete pDpBlitStream;
    LOG_DBG("CKH: MDP init -");
    sbRotImage->mImageBuffer->saveToFile("sdcard/bokehPipe/scaleDownRot.yuv");
}
