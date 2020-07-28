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
#ifndef MASK_COMPRESSOR_H_
#define MASK_COMPRESSOR_H_

#include <vector>
#include <mtkcam/def/common.h>

namespace StereoHAL {

struct RUN_LENGTH_DATA
{
    //Data is always 0/255
    MUINT32 offset;
    MUINT32 len;

    RUN_LENGTH_DATA(MUINT32 o, MUINT32 l) {
        offset = o;
        len = l;
    }
};

class MaskCompressor
{
public:
    static void compressMask(MUINT8 *mask, NSCam::MSize MASK_SIZE, std::vector<RUN_LENGTH_DATA> &compressedMask)
    {
        compressedMask.clear();
        const int IMAGE_SIZE = MASK_SIZE.w * MASK_SIZE.h;

        if(NULL == mask ||
           0 == IMAGE_SIZE)
        {
            return;
        }

        MUINT32 len = 0;
        MUINT32 offset = 0;

        const int CMP_LEN = 128;
        MUINT8 *FF_MASK = new MUINT8[CMP_LEN];
        ::memset(FF_MASK, 0xFF, CMP_LEN);

        for(int i = 0; i < IMAGE_SIZE; i += CMP_LEN) {
            if(0 == memcmp(mask, FF_MASK, CMP_LEN)) {
                if(0 == len) {
                    offset = i;
                }

                len += CMP_LEN;
                mask += CMP_LEN;
            } else {
                for(int j = 0; j < CMP_LEN; j++, mask++) {
                    if(0 != *mask) {
                        if(0 != len) {
                            ++len;
                        } else {
                            len = 1;
                            offset = i+j;
                        }
                    } else {
                        if(0 != len) {
                            compressedMask.push_back(RUN_LENGTH_DATA(offset, len));
                            len = 0;
                        }
                    }
                }
            }
        }

        if(0 != len) {
            compressedMask.push_back(RUN_LENGTH_DATA(offset, len));
        }

        delete [] FF_MASK;
    }
};

};  //namespace StereoHAL
#endif