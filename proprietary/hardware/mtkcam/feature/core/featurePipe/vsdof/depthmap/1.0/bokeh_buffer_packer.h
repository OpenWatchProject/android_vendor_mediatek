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
#ifndef BOKEH_BUFFER_PACKER_H_
#define BOKEH_BUFFER_PACKER_H_

#include <vector>
#include <thread>
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

namespace StereoHAL
{

struct PackBufferParam
{
    MINT32          captureOrientation;

    //Input buffers
    MUINT8          *depthMap;
    MUINT8          *occMap;
    MUINT8          *nocMap;

    //Output buffer, need to alloc by user, size: DMW*4
    MUINT8          *packedBuffer;
};

class CaptureBufferPacker
{
public:
    /**
     * \brief Pack buffer for bokeh
     * \details Pack as follow format:
     *          +----------+
     *          ¦ WMI      ¦
     *          +----------¦
     *          ¦ WMI      ¦
     *          +----------¦
     *          ¦ VAR      ¦
     *          +----------¦
     *          ¦ OCC      ¦
     *          +----------+
     *
     * \param param parameters to pack
     */
    static void packBuffer(PackBufferParam &param)
    {
        MUINT8 *buffers[] = {param.depthMap, NULL, NULL, param.occMap};
        const int TOTAL_BUFFER = sizeof(buffers)/sizeof(MUINT8*);
        const MSize BUFFER_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMW, eSTEREO_SCENARIO_CAPTURE);
        const size_t PER_BUFFER_SIZE = BUFFER_SIZE.w * BUFFER_SIZE.h;

        int rotation = param.captureOrientation - StereoSettingProvider::getModuleRotation();
        if(rotation < 0) {
            rotation += 360;
        }

        std::vector<std::thread> threads;
        for(int bi = 0; bi < TOTAL_BUFFER; ++bi) {
            if(NULL == buffers[bi]) {
                ::memset(param.packedBuffer + PER_BUFFER_SIZE * bi, 0, PER_BUFFER_SIZE);
                continue;
            }

            threads.push_back(std::thread(
                [&, bi] () {
                    const char *TAG = "CaptureBufferPacker";
                    char logName[100];
                    sprintf(logName, "Pack %d: size %dx%d, rotate %d",
                            bi, BUFFER_SIZE.w, BUFFER_SIZE.h, rotation);
                    AutoProfileUtil profile(TAG, logName);

                    //Rotate buffer
                    {
                        MUINT8 *dstBuffer = param.packedBuffer + PER_BUFFER_SIZE * bi;
                        //Rotate by CPU
                        MINT32 writeRow = 0;
                        MINT32 writeCol = 0;
                        MINT32 writePos = 0;

                        if(eRotate_0 == rotation) {
                            ::memcpy(dstBuffer, buffers[bi], PER_BUFFER_SIZE);
                        } else if(eRotate_180 == rotation) {
                            writeRow = 0;
                            writeCol = 0;
                            writePos = 0;
                            for(int i = PER_BUFFER_SIZE-1; i >= 0; --i) {
                                *(dstBuffer + writePos) = *(buffers[bi] + i);

                                ++writePos;
                                ++writeCol;
                                if(writeCol >= BUFFER_SIZE.w) {
                                    ++writeRow;
                                    writeCol = 0;
                                    writePos = BUFFER_SIZE.w * writeRow;
                                }
                            }
                        } else if(eRotate_270 == rotation) {
                            writeRow = 0;
                            writeCol = BUFFER_SIZE.h - 1;
                            writePos = writeCol;
                            for(int i = PER_BUFFER_SIZE-1; i >= 0; --i) {
                                *(dstBuffer + writePos) = *(buffers[bi] + i);

                                writePos += BUFFER_SIZE.h;
                                ++writeRow;
                                if(writeRow >= BUFFER_SIZE.w) {
                                    writeRow = 0;
                                    --writeCol;
                                    writePos = writeCol;
                                }
                            }
                        } else if(eRotate_90 == rotation) {
                            writeRow = BUFFER_SIZE.w - 1;
                            writeCol = 0;
                            writePos = PER_BUFFER_SIZE - BUFFER_SIZE.h;
                            for(int i = PER_BUFFER_SIZE-1; i >= 0; --i) {
                                *(dstBuffer + writePos) = *(buffers[bi] + i);

                                writePos -= BUFFER_SIZE.h;
                                --writeRow;
                                if(writeRow < 0) {
                                    writeRow = BUFFER_SIZE.w - 1;
                                    ++writeCol;
                                    writePos = PER_BUFFER_SIZE - BUFFER_SIZE.h + writeCol;
                                }
                            }
                        }
                    }
                }
            ));
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
            t.join();
        });

        //index 0 and 1 are both WMI
        ::memcpy(param.packedBuffer + PER_BUFFER_SIZE, param.packedBuffer, PER_BUFFER_SIZE);
    }
private:
};  // class CaptureBufferWarapper
};  //namespace StereoHAL

#endif
