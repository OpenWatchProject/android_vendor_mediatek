/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _SAR_H_
#define _SAR_H_
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <sensorFsm.h>
#include <module.h>
#include <scp_err_info.h>
#include <sensorErrInfo.h>
#include "cache_internal.h"

#define SAR_INTERRUPT_MODE   0
#define SAR_POLLING_MODE     1

enum SarFsmState {
    CHIP_SAR_SAMPLING,
    CHIP_SAR_CONVERT,
    CHIP_SAR_SAMPLING_DONE,
    CHIP_SAR_ENABLE,
    CHIP_SAR_ENABLE_DONE,
    CHIP_SAR_DISABLE,
    CHIP_SAR_DISABLE_DONE,
    CHIP_SAR_RATECHG,
    CHIP_SAR_RATECHG_DONE,
    CHIP_SAR_CALI,
    CHIP_SAR_CALI_DONE,
    CHIP_SAR_CFG,
    CHIP_SAR_CFG_DONE,
    CHIP_INIT_DONE,
    CHIP_IDLE,
    CHIP_RESET,
};

enum SarState {
    SAR_STATE_INIT = -1,
    SAR_STATE_NEAR = 0,
    SAR_STATE_FAR = 1,
};

struct SarData_t {
    uint8_t sensType;
    int32_t sarData[3]; // unit:
    enum SarState sarState;
};

struct SarCntlPacket {
    uint32_t rate;
    uint32_t waterMark;
    uint64_t latency;
    uint32_t magicNum;
};


//void registerSarInterruptMode(uint8_t mode);
void registerSarDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size);

void sarSensorRegister(void);
void delayCallback(uint32_t timerId, void *cookie);
void sarSendCalibrationResult(uint8_t sensorType,
         int32_t bias[3], int8_t status);

 static inline int rxControlInfo(struct SarCntlPacket *recvPacket, void *inBuf,
     uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
 {
     struct SarCntlPacket *packet = (struct SarCntlPacket *)inBuf;

     if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct SarCntlPacket)) {
         return -1;
     }
     recvPacket->rate = packet->rate;
     return 0;
 }
 /*
  * outBuf: driver genarate
  * outSize: driver genarate
  * elemOutSize: struct SarData_t
  */

static inline int rxTransferDataInfo(struct transferDataInfo *dataInfo, void *inBuf, uint8_t inSize,
                                     uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (outBuf == NULL || outSize == NULL || elemOutSize == NULL) {
        return -1;
    }
    dataInfo->inBuf = inBuf;
    dataInfo->inSize = inSize;
    dataInfo->elemInSize = elemInSize;
    dataInfo->outBuf = outBuf;
    dataInfo->outSize = outSize;
    dataInfo->elemOutSize = elemOutSize;
    return 0;
}

static inline void txTransferDataInfo(struct transferDataInfo *dataInfo,
                                      uint8_t outSize, struct SarData_t *data)
{
    *(struct SarData_t **)dataInfo->outBuf =  data;
    *dataInfo->elemOutSize = sizeof(struct SarData_t);
    *dataInfo->outSize = outSize;
}
#endif
