/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "MediaTypes.h"
#include "mhal_jpeg.h"
#include "jpeg_hal.h"

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}

#include <cutils/log.h>

#define xlog(...) \
        do { \
            ALOGW(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "mHalJpgEnc"

int mHalJpgEncStart(MHAL_JPEG_ENC_START_IN *encInParams)
{
    JpgEncHal *jHalEncoder = new JpgEncHal();
    if(!jHalEncoder->lock())
    {
        xlog("mHalJpgEnc:: can't lock jpeg encode resource");
        delete jHalEncoder;
        return JPEG_ERROR_INVALID_DRIVER;
    }

    if (encInParams->encFormat != JPEG_ENC_FORMAT_YUY2 && encInParams->encFormat != JPEG_ENC_FORMAT_UYVY &&
        encInParams->encFormat != JPEG_ENC_FORMAT_NV12 && encInParams->encFormat != JPEG_ENC_FORMAT_NV21 &&
        encInParams->encFormat != JPEG_ENC_FORMAT_YV12)
    {
        jHalEncoder->unlock();
        xlog("mHalJpgEnc:: unsupport format(%u)\n", encInParams->encFormat);
        delete jHalEncoder;
        return JPEG_ERROR_INVALID_FORMAT;
    }
    jHalEncoder->setEncSize(encInParams->imgWidth, encInParams->imgHeight, (JpgEncHal::EncFormat)encInParams->encFormat);

    if(encInParams->quality > 100)
    {
        jHalEncoder->unlock();
        xlog("mHalJpgEnc:: unsupport quality(%u)\n", encInParams->quality);
        delete jHalEncoder;
        return JPEG_ERROR_INVALID_PARA;
    }
    jHalEncoder->setQuality(encInParams->quality);
    jHalEncoder->enableSOI(encInParams->enableSOI);

    if (encInParams->srcFD != -1 && encInParams->dstFD != -1)
    {
        jHalEncoder->setIonMode(true);
    }

    jHalEncoder->setSrcAddr(encInParams->srcAddr, encInParams->srcChromaAddr);
    jHalEncoder->setSrcFD(encInParams->srcFD, encInParams->srcChromaFD);
    jHalEncoder->setSrcBufSize(encInParams->imgStride, encInParams->srcBufferSize, encInParams->srcChromaBufferSize);
    jHalEncoder->setDstAddr(encInParams->dstAddr);
    jHalEncoder->setDstFD(encInParams->dstFD);
    jHalEncoder->setDstSize(encInParams->dstBufferSize);

    if(!jHalEncoder->start(encInParams->encSize))
    {
        xlog("mHalJpgEnc:: trigger Encoder Fail L:%d!!\n", __LINE__);
        jHalEncoder->unlock();
        delete jHalEncoder;
        return JPEG_ERROR_INVALID_DRIVER;
    }

    jHalEncoder->unlock();
    delete jHalEncoder;
    jHalEncoder = NULL;

    return JPEG_ERROR_NONE;
}

