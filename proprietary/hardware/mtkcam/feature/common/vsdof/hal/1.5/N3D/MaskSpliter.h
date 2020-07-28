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
#ifndef _MASM_SPLITER_H_
#define _MASM_SPLITER_H_

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <vsdof/hal/ProfileUtil.h>

//Since graphic buffer is non-cacheable, which means direct access to gbuffer is very slow.
//So we use cache to read and get mask
//Performance enhancement is about 5x faster
const MUINT32 CACHE_SIZE = 256; //experimental result, faster than 64, almost the same as 256, CACHE_SIZE % 8 = 0
const MUINT32 COPY_SIZE  = CACHE_SIZE * sizeof(MUINT32);

class MaskSpliter
{
public:
    //====================================================================
    //  SPLITER: Split and rotate mask according to module orientation
    //  Result is stored in outputBuffer(2176x1152)
    //====================================================================
    static bool splitMask(IImageBuffer *inputBuffer, MUINT8 *outputBuffer, ENUM_ROTATION rotation=eRotate_0)
    {
        if(NULL == inputBuffer ||
           NULL == outputBuffer)
        {
            return false;
        }

        if( 90 == rotation ||
           270 == rotation)
        {
            return _splitByMDP(inputBuffer, outputBuffer, rotation);
        }

        const int WIDTH  = inputBuffer->getImgSize().w;
        const int HEIGHT = inputBuffer->getImgSize().h;
        const int IMAGE_SIZE = WIDTH * HEIGHT;
        const int PIXEL_SIZE = sizeof(MUINT32);

        //Get mask from graphic buffer & rotate at the same time
        //Mask is 8-bit image, value: 0 or 0xFF

        //No rotation
        int nCol = 0;
        int nRow = 0;
        int nWritePos = 0;

        if(180 == rotation) {
            nWritePos = IMAGE_SIZE - 1;
        }

        //Since graphic buffer is non-cacheable, which means direct access to gbuffer is very slow.
        //So we use cache to read and get mask
        //Performance enhancement is about 300ms -> 66 ms
        const MUINT32 CACHE_SIZE = 256; //experimental result, faster than 64, almost the same as 256, CACHE_SIZE % 8 = 0
        const MUINT32 COPY_SIZE = CACHE_SIZE * PIXEL_SIZE;

        MUINT32* pImgCache = new MUINT32[CACHE_SIZE];
        MUINT32* pGraphicBuffer = (MUINT32*)inputBuffer->getBufVA(0);
        MUINT32* pCachePos = NULL;

        int nIndex = 0;
        for(int nRound = IMAGE_SIZE/CACHE_SIZE - 1; nRound >= 0; nRound--)
        {
            //Cache graphic buffer
            ::memcpy(pImgCache, pGraphicBuffer, COPY_SIZE);
            pGraphicBuffer += CACHE_SIZE;
            pCachePos = pImgCache;

            //Get mask from alpha channel and rotate at a time
            for(nIndex = CACHE_SIZE-1; nIndex >= 0; nIndex--) {
                *(outputBuffer + nWritePos) = (MUINT8)((*pCachePos)>>24);
                ++pCachePos;

                if(0 == rotation) {
                    ++nWritePos;
                } else {
                    --nWritePos;
                }
            }
        }

        //Last Round
        int leftSize = IMAGE_SIZE % CACHE_SIZE;
        if(leftSize != 0) {
            ::memcpy(pImgCache, pGraphicBuffer, leftSize * PIXEL_SIZE);
            pCachePos = pImgCache;

            //Get mask from alpha channel and rotate at a time
            for(nIndex = leftSize - 1; nIndex >= 0; nIndex--) {
                *(outputBuffer + nWritePos) = (MUINT8)((*pCachePos)>>24);
                ++pCachePos;

                if(0 == rotation) {
                    ++nWritePos;
                } else {
                    --nWritePos;
                }
            }
        }

        delete [] pImgCache;
        return true;
    }

private:
    static bool _splitByMDP(IImageBuffer *inputBuffer, MUINT8 *outputBuffer, ENUM_ROTATION rotation)
    {
        bool ret = true;
        sp<IImageBuffer> newImg;
        const int IMAGE_SIZE = inputBuffer->getImgSize().w * inputBuffer->getImgSize().h;
        if(StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_RGBA8888, inputBuffer->getImgSize(), !IS_ALLOC_GB, newImg)) {
            if(StereoDpUtil::transformImage(inputBuffer, newImg.get(), rotation)) {
                MUINT32 *pBuffer = (MUINT32 *)inputBuffer->getBufVA(0);
                for(int i = IMAGE_SIZE-1; i >= 0; i--) {
                    outputBuffer[i] = (MUINT8)(pBuffer[i]>>24);
                }
            } else {
                ALOGD("Transfer image failed");
                ret = false;
            }
        } else {
            ALOGE("Cannot alloc image buffer");
            ret = false;
        }

        return ret;
    }
};

#endif
