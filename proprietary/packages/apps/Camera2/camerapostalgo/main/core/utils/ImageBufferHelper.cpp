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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#include <utils/ImageBufferHelper.h>
#include <sys/stat.h>

#define LOG_TAG "ImageBufferHelper"

sp<IImageBuffer> NSPA::convertImageBufferFromGraphicBuffer(const sp<GraphicBuffer> gb) {
    ANativeWindowBuffer * pANativeWindowBuffer = gb->getNativeBuffer();
    ALOGD("convertImageBufferFromGraphicBuffer usage %ju",
            pANativeWindowBuffer->usage);
    sp<IGraphicImageBufferHeap> pIGraphicImageBufferHeap =
            IGraphicImageBufferHeap::create("PostAlgo",
                    pANativeWindowBuffer);
    return pIGraphicImageBufferHeap->createImageBuffer();
}

void NSPA::dumpBuffer(const sp<android::GraphicBuffer> gb, const char* fileNamePrefix) {
    static int dumpCount = 0;
    sp<IImageBuffer> buffer = convertImageBufferFromGraphicBuffer(gb);
    dumpBuffer(buffer.get(), fileNamePrefix);
}

void NSPA::dumpBuffer(const IImageBuffer* buffer, const char* fileNamePrefix) {
    static int dumpCount = 0;

    char path[128] = "/sdcard/pas_dump";
    int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    if ((result == -1) && (errno != EEXIST)) {
        ALOGE("mkdir fail, error %d, return", errno);
        return;
    }

    char fileName[128];
    MSize size = buffer->getImgSize();
    sprintf(fileName, "%s/%s_f%d_w%d_h%d_s%d_%d.dump", path, fileNamePrefix,
            buffer->getImgFormat(), size.w, size.h, size.w, ++dumpCount);
    FILE *fp = fopen(fileName, "wb");
    if (NULL == fp) {
        ALOGE("fail to open file %s", fileName);
    } else {
        size_t planecount = buffer->getPlaneCount();
        for (size_t i = 0; i < planecount; i++) {
            unsigned char* planeBuffer = (unsigned char *)buffer->getBufVA(i);
            int planeBufferSize = buffer->getBufSizeInBytes(i);
            int total_write = 0;
            while(total_write < planeBufferSize) {
                int write_size = fwrite(planeBuffer+total_write, 1, planeBufferSize-total_write, fp);
                if (write_size <= 0) {
                    ALOGE("write_size = %d, fileName = %s", write_size, fileName);
                }
                total_write += write_size;
            }
        }
        fclose(fp);
    }
}

void NSPA::dumpJpegBuffer(const sp<GraphicBuffer> buffer, const char* fileNamePrefix) {

}
