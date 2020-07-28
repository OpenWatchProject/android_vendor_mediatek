/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_VPU_H
#define _MTK_VPU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

typedef enum {
    eDirectionIn,
    eDirectionOut,
    eDirectionInOut
} VpuPortDirection;

typedef enum {
    eImageBuffer,
    eDataBuffer
} VpuPortUsage;

typedef enum {
    eTypeChar,
    eTypeInt32,
    eTypeInt64,
    eTypeFloat,
    eTypeDouble
} VpuPropertyType;

typedef enum {
    eFormatData,
    eFormatImageY8,
    eFormatImageYV12,
    eFormatImageNV21,
    eFormatImageYUY2,
    eFormatReserved1,
    eFormatReserved2,
    eFormatReserved3,
} VpuBufferFormat;

typedef enum {
	eIndex_A = (0x01 << 0),
	eIndex_B = (0x01 << 1),
	eIndex_C = (0x01 << 2),
	eIndex_D = (0x01 << 3),
	eIndex_NONE = 0xFFFF,
} VpuCoreIndex;

typedef struct {
    unsigned int fd;
    unsigned int offset;
    unsigned int stride;
    unsigned int length;
} VpuPlane;

typedef struct {
    unsigned int planeCount;
    VpuPlane planes[3];
    VpuBufferFormat format;
    unsigned int width;
    unsigned int height;
    unsigned int port_id;
} VpuBuffer;

typedef struct {
    VpuPropertyType type;
    unsigned int count;
    union {
        char *c;
        uint8_t *u8;
        int32_t *i32;
        float *f;
        int64_t *i64;
        double *d;
    } data;
} VpuProperty;


typedef struct {
    unsigned int id;
    const char *name;
    VpuPortDirection portDirection;
    VpuPortUsage portUsage;
} VpuPort;

typedef enum {
  ePowerModeDynamic,  // dynamically control by driver, do not support in 2.0 version
  ePowerModeOn        // Always on, pre-open the clock and boot-up
} VpuPowerMode;

typedef enum {
	ePowerOppUnrequest = 0xFF,    // default step
}VpuPowerOpp;

typedef struct {
  VpuPowerMode mode;
  uint8_t      opp;	/* opp level */
}VpuPower;

/* 2.0 */
typedef struct VpuExtraParam{
	VpuExtraParam()
	: opp_step(0xff), freq_step(0xff), bw(0x0)
	{}
  	uint8_t      opp_step;	/* opp level */
  	uint8_t      freq_step;	/* special freq by demand, no use, would be removed */
	uint32_t	 bw;		/* expected used bw */
}VpuExtraParam;


typedef struct {
	VpuCoreIndex core_index;
	bool available;
	int waiting_requests_num;
} VpuStatus;

class VpuAlgo {
public:
	/* 1.0 only */
    virtual bool getProperty(const char *keyName, VpuProperty &prop) = 0;
    virtual VpuPort *getPort(const char *name) = 0;
	/* *** */
    virtual ~VpuAlgo() { };
};


class VpuRequest {
public:
	/* 1.0 only */
    virtual void addBuffer(VpuPort *port, VpuBuffer &buffer) = 0;

    virtual bool getProperty(const char *keyName, VpuProperty &prop) = 0;
    virtual bool setProperty(const char *keyName, const VpuProperty &prop) = 0;
	/* 2.0 */
	virtual bool addBuffer(VpuBuffer &buffer) = 0;
	virtual bool setProperty(void* sett_ptr, int size) = 0;
	virtual bool setExtraParam(VpuExtraParam extra_param) = 0;

    virtual ~VpuRequest() { };
};


class VpuStream {

public:
    static VpuStream *createInstance();
    static VpuStream *createInstance(const char* callername);

    virtual VpuAlgo *getAlgo(char *name) = 0;

    virtual VpuRequest *acquire(VpuAlgo *algo) = 0;

    virtual void release(VpuRequest *request) = 0;

    virtual bool enque(VpuRequest *request, unsigned int core_index = 0xFFFF) = 0;

    virtual VpuRequest *deque() = 0;

    virtual void flush() = 0;

/* powerOnVpu */
    virtual bool setPower(VpuPower& pwr, unsigned int core_index = 0xFFFF) = 0;

	/* 2.0 only */
	virtual bool getVpuStatus(std::vector<VpuStatus>& vpu_status_info) = 0;

    virtual ~VpuStream() { };
};

/* ==================== VPU Utilitis ========================== */
uint32_t vpuMapPhyAddr(int fd);     /* map phyaddr early */

#endif
