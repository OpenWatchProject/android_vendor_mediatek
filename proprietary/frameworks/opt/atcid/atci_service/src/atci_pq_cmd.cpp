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

#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

// If AAL support read/write fields, following 2 macros should be defined in AALClient.h
#ifndef AALClient_readField
#define AALClient_readField(field, value_ptr) (*(value_ptr) = 0, -1)
#endif
#ifndef AALClient_writeField
#define AALClient_writeField(field, value) (-1)
#endif

#include "ddp_drv.h"
#if defined(MTK_PQ_SERVICE)
#include <PQServiceCommon.h>
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
using vendor::mediatek::hardware::pq::V2_0::ioctlRequest;
using vendor::mediatek::hardware::pq::V2_0::dispPQIoctlParams;
using ::android::sp;
#endif
extern "C" {
#include "atci_pq_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include "at_tok.h"
}

// We don't rely on STL
template <typename _T>
class ManagedArray {
private:
    _T *array;
    int size;

public:
    ManagedArray() : array(NULL), size(0) { }

    ~ManagedArray() {
        if (array != NULL)
            delete [] array;
    }

    int getSize() const {
        return size;
    }

    void setSize(int n) {
        if (n <= 0)
            n = 1;
        if (size == n)
            return;
        if (array != NULL)
            delete [] array;
        size = n;
        array = new _T[size];
    }

    _T* getArray() {
        return array;
    }

    _T& operator[] (int index) {
        if (index < 0)
            index = 0;
        else if (index >= size)
            index = size - 1;
        return array[index];
    }
};


// AAL framework adaptation
#define AAL_INTERNAL_FIELDS_END        0x10000
#define AAL_ADAPT_VIRTUAL_ADDRESS_BASE 0x10000
#define AAL_ADAPT_VIRTUAL_ADDRESS_END  0x20000

#define AAL_ADAPT_VARIABLE_BASE        0x0000
#define AAL_ADAPT_VARIABLE_END         0x1000
#define AAL_ADAPT_CURVE_ADDRESS_BASE   0x1000
#define AAL_ADAPT_CURVE_ADDRESS_END    0x2000

#define __UNUSED(expr) do { (void)(expr); } while (0)

typedef unsigned int address_t;


static ManagedArray<int> gAli2BliCurve;

static unsigned int readAALAdaptVariable(address_t vaddr)
{
    __UNUSED(vaddr);

#ifdef MTK_AAL_RUNTIME_TUNING_SUPPORT
    unsigned int value = 0;

    vaddr -= AAL_ADAPT_VIRTUAL_ADDRESS_BASE;

    AALClient &client = AALClient::getInstance();
    if (AAL_ADAPT_VARIABLE_BASE <= vaddr && vaddr < AAL_ADAPT_VARIABLE_END) {
        vaddr -= AAL_ADAPT_VARIABLE_BASE;

        status_t ret;
        uint32_t serial;

        IAALService::AdaptFieldId adaptField = static_cast<IAALService::AdaptFieldId>(vaddr);
        ret = client.getAdaptField(adaptField, &value, sizeof(value), &serial);
        if (ret != NO_ERROR)
            value = 0;

        ALOGD("[AAL_CMD] ret = %d", ret);
    } else if (AAL_ADAPT_CURVE_ADDRESS_BASE <= vaddr &&
            vaddr < AAL_ADAPT_CURVE_ADDRESS_END)
    {
        status_t ret;
        uint32_t serial;
        int index = (vaddr - AAL_ADAPT_CURVE_ADDRESS_BASE) / sizeof(int);
        if (index == 0) {
            int len;
            ret = client.getAdaptField(IAALService::ALI2BLI_CURVE_LENGTH, &len, sizeof(len), &serial);
            if (ret != NO_ERROR) {
                ALOGE("[AAL_CMD] Read curve length failed: ret = %d", ret);
                return 0;
            }

            if (len > (AAL_ADAPT_CURVE_ADDRESS_END - AAL_ADAPT_CURVE_ADDRESS_BASE) / 2) {
                ALOGE("[AAL_CMD] Length range check failed: length = %d", len);
                return 0;
            }

            gAli2BliCurve.setSize(len * 2);
            ret = client.getAdaptField(IAALService::ALI2BLI_CURVE,
                    gAli2BliCurve.getArray(), gAli2BliCurve.getSize() * sizeof(int), &serial);
            if (ret != NO_ERROR) {
                ALOGE("[AAL_CMD] Read curve failed: ret = %d", ret);
                memset(gAli2BliCurve.getArray(), 0, gAli2BliCurve.getSize() * sizeof(int));
            }
        }

        value = gAli2BliCurve[index];
    }

    return value;
#else
    return 0;
#endif
}


static void writeAALAdaptVariable(address_t vaddr, unsigned int value)
{
    __UNUSED(vaddr);
    __UNUSED(value);

#ifdef MTK_AAL_RUNTIME_TUNING_SUPPORT

    vaddr -= AAL_ADAPT_VIRTUAL_ADDRESS_BASE;

    AALClient &client = AALClient::getInstance();
    if (AAL_ADAPT_VARIABLE_BASE <= vaddr && vaddr < AAL_ADAPT_VARIABLE_END) {
        vaddr -= AAL_ADAPT_VARIABLE_BASE;

        if (vaddr == IAALService::ALI2BLI_CURVE_LENGTH) {
            if (value <= (AAL_ADAPT_CURVE_ADDRESS_END - AAL_ADAPT_CURVE_ADDRESS_BASE) / 2) {
                gAli2BliCurve.setSize(value * 2);
                ALOGD("[AAL_CMD] Initial curve length = %u", value);
            } else {
                ALOGE("[AAL_CMD] ERROR: invalid curve length = %u", value);
            }
        } else {
            status_t ret;
            uint32_t serial;

            IAALService::AdaptFieldId adaptField = static_cast<IAALService::AdaptFieldId>(vaddr);
            ret = client.setAdaptField(adaptField, &value, sizeof(value), &serial);
            ALOGD("[AAL_CMD] ret = %d", ret);
        }
    } else if (AAL_ADAPT_CURVE_ADDRESS_BASE <= vaddr &&
            vaddr < AAL_ADAPT_CURVE_ADDRESS_END)
    {
        uint32_t serial;
        int index = (vaddr - AAL_ADAPT_CURVE_ADDRESS_BASE) / sizeof(int);
        if (index < gAli2BliCurve.getSize()) {
            gAli2BliCurve[index] = value;
            ALOGD("[AAL_CMD] curve[%d] = %d", index, value);
        }
        if (index == gAli2BliCurve.getSize() - 1) {
            status_t ret;
            ret = client.setAdaptField(IAALService::ALI2BLI_CURVE, gAli2BliCurve.getArray(),
                    gAli2BliCurve.getSize() * sizeof(int), &serial);
            ALOGD("[AAL_CMD] Write curve to service, ret = %d", ret);
        }
    }
#endif
}



//4 types of operation codes: ACTION / TEST(=?) / READ(?) / SET(=)

extern "C"
int pq_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    ALOGD("[PQ_CMD] pq cmd handler handles cmdline:%s", cmdline);

    int actionID=0, RegBase = 0, RegValue = 0;
    struct timespec tt1, tt2;
#if defined(MTK_PQ_SERVICE)
    dispPQIoctlParams arg;
    memset(&arg, 0, sizeof(arg));
    sp<IPictureQuality> service = IPictureQuality::getService();
#endif
    clock_gettime(CLOCK_REALTIME, &tt1);

    switch(at_op){
        case AT_ACTION_OP:
        case AT_TEST_OP:
        case AT_READ_OP:
            sprintf(response,"\r\nPQ OK\r\n");
            break;

        case AT_SET_OP:

            at_tok_nexthexint(&cmdline, &actionID);
#if defined(MTK_PQ_SERVICE)
            /* check if PQservice is available for PQ related Action*/
            if (service == nullptr && actionID >= HW_READ && actionID < PQ_SW_WRITE)
            {
                ALOGE("[PQ_CMD] failed to get HW PQservice");
                return 0;
            }
#endif
            if (actionID == HW_READ)
            {
                DISP_READ_REG params;

                at_tok_nexthexint(&cmdline, &RegBase);

                params.reg = RegBase;
                params.val = 0;
                params.mask = 0xFFFFFFFF;
#if defined(MTK_PQ_SERVICE)
                arg.request = ioctlRequest::IOCTL_READ_REG;
                arg.reg = params.reg;
                arg.val = params.val;
                arg.mask = params.mask;

                android::hardware::Return<void> ret = service->execIoctl(arg,
                    [&] (Result retval, const dispPQIoctlParams &retParam)
                {
                    if (retval == Result::OK)
                    {
                        params.val = retParam.val;
                    }
                    else
                    {
                        ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                    }
                });
                if (!ret.isOk()) {
                    ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                }

#else
                ALOGE("[PQ_CMD] Not support tuning");
#endif
                sprintf(response,"%08x\r\n", params.val);
                ALOGD("[PQ_CMD] R: 0x%08lx = %08x", (unsigned long)params.reg, params.val);
            }
            else if (actionID == HW_WRITE)
            {
                DISP_WRITE_REG params;

                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                if((unsigned int)RegBase == 0xEEEEEEEE)
                {
                    if(RegValue == 0)
                    {
                        ALOGD("PQ_DEBUG SCE program start...\n");
#if defined(MTK_PQ_SERVICE)
                        arg.request = ioctlRequest::IOCTL_MUTEX_CONTROL;
                        arg.val = 1;

                        android::hardware::Return<void> ret = service->execIoctl(arg,
                            [&] (Result retval, const dispPQIoctlParams &retParam)
                        {
                            if (retval != Result::OK)
                            {
                                __UNUSED(retParam);
                                ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                        }
#else
                        ALOGE("[PQ_CMD] Not support tuning");
#endif
                    }
                    else
                    {
                        ALOGD("PQ_DEBUG SCE program done!\n");
#if defined(MTK_PQ_SERVICE)
                        arg.val = 2;
                        arg.request = ioctlRequest::IOCTL_MUTEX_CONTROL;

                        android::hardware::Return<void> ret = service->execIoctl(arg,
                            [&] (Result retval, const dispPQIoctlParams &retParam)
                        {
                            if (retval != Result::OK)
                            {
                                __UNUSED(retParam);
                                ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                        }
#else
                        ALOGE("[PQ_CMD] Not support tuning");
#endif
                    }
                }
                else
                {
                    params.reg = RegBase;
                    params.val = RegValue;
                    params.mask = 0xFFFFFFFF;
#if defined(MTK_PQ_SERVICE)
                    arg.request = ioctlRequest::IOCTL_WRITE_REG;
                    arg.reg = params.reg;
                    arg.val = params.val;
                    arg.mask = params.mask;

                    android::hardware::Return<void> ret = service->execIoctl(arg,
                        [&] (Result retval, const dispPQIoctlParams &retParam)
                    {
                        if (retval != Result::OK)
                        {
                            __UNUSED(retParam);
                            ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                        }
                    });
                    if (!ret.isOk()) {
                        ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                    }
#else
                    ALOGE("[PQ_CMD] Not support tuning");
#endif
                    ALOGD("[PQ_CMD] W: 0x%08x = %08x", params.reg, params.val);
                }
#if defined(MTK_PQ_SERVICE)
                sprintf(response,"%08x\r\n", arg.val);
#else
                sprintf(response,"%08x\r\n", 0);
#endif
            }
            else if (actionID == PQ_SW_READ)
            {
                at_tok_nexthexint(&cmdline, &RegBase);

                if ((unsigned int)RegBase < 0xFFFF0000) {
#if defined(MTK_PQ_SERVICE)
                        int32_t module, field;
                        int32_t getvalue = 0;

                        module = (RegBase >> 16) & 0xffff;
                        field = (RegBase & 0xffff);

                        android::hardware::Return<void> ret = service->getTuningField(module, field,
                            [&] (Result retval, int32_t value) {
                            if (retval == Result::OK) {
                                getvalue = value;
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::getTuningField failed!");
                            sprintf(response, "%08x\r\n", 0);
                        }
                        else {
                            sprintf(response, "%08x\r\n", getvalue);
                            ALOGD("[PQ_CMD] SR: %x:%x = 0x%x(%d)", module, field, (unsigned int)(getvalue), getvalue);
                        }
#else
                        sprintf(response, "%08x\r\n", 0);
                        ALOGE("[PQ_CMD] Not support tuning");
#endif
                } else {
                    DISP_READ_REG params;
                    bool isOk_flag = true;

                    params.reg = (RegBase); // For compatible
                    params.val = 0;
                    params.mask = 0xFFFFFFFF;
#if defined(MTK_PQ_SERVICE)
                    int32_t module, field;
                    int32_t getvalue = 0;

                    module = (RegBase >> 16) & 0xffff;
                    field = (RegBase & 0xffff);

                    if (params.reg >= SWREG_PQDS_DS_EN && params.reg < SWREG_PQDS_END) {
                        android::hardware::Return<void> ret =
                            service->getTuningField(MOD_DYNAMIC_SHARPNESS, RegBase - SWREG_PQDS_DS_EN,
                            [&] (Result retval, int32_t value) {
                            if (retval == Result::OK) {
                                getvalue = value;
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::getTuningField failed!");
                            isOk_flag = false;
                        }
                        params.val = getvalue;
                    }
                    else if (params.reg >= SWREG_TDSHP_REG_GAIN_MID && params.reg < SWREG_TDSHP_REG_END) {
                        android::hardware::Return<void> ret =
                            service->getTuningField(MOD_TDSHP_REG, RegBase - SWREG_TDSHP_REG_GAIN_MID,
                            [&] (Result retval, int32_t value) {
                            if (retval == Result::OK) {
                                getvalue = value;
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::getTuningField failed!");
                            isOk_flag = false;
                        }
                        params.val = getvalue;
                    }
                    else if (params.reg == SWREG_TDSHP_TUNING_MODE) {
                        android::hardware::Return<void> ret = service->getTDSHPFlag(
                            [&] (Result retval, int32_t value) {
                            if (retval == Result::OK) {
                                getvalue = value;
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::getTDSHPFlag failed!");
                            isOk_flag = false;
                        }
                        params.val = getvalue;
                    }
                    else if (params.reg >= SWREG_BLACK_EFFECT_ENABLE && params.reg < SWREG_DC_END) {
                        android::hardware::Return<void> ret =
                            service->getTuningField(MOD_DYNAMIC_CONTRAST, RegBase - SWREG_BLACK_EFFECT_ENABLE,
                            [&] (Result retval, int32_t value) {
                            if (retval == Result::OK) {
                                getvalue = value;
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::getTuningField failed!");
                            isOk_flag = false;
                        }
                        params.val = getvalue;
                    }
                    else
                    {
                        arg.request = ioctlRequest::IOCTL_READ_SW_REG;
                        arg.reg = params.reg;
                        arg.val = params.val;
                        arg.mask = params.mask;

                        android::hardware::Return<void> ret = service->execIoctl(arg,
                            [&] (Result retval, const dispPQIoctlParams &retParam)
                        {
                            if (retval == Result::OK)
                            {
                                params.val = retParam.val;
                            }
                            else
                            {
                                ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                            isOk_flag = false;
                        }
                    }

                    if (isOk_flag) {
                        sprintf(response, "%08x\r\n", params.val);
                        ALOGD("[PQ_CMD] SR: 0x%08x = %08x", params.reg, params.val);
                    }
                    else {
                        sprintf(response, "%08x\r\n", 0);
                    }
#else
                    sprintf(response, "%08x\r\n", 0);
                    ALOGE("[PQ_CMD] Not support tuning");
#endif
                }
            }
            else if (actionID == PQ_SW_WRITE)
            {
                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                if ((unsigned int)RegBase < 0xFFFF0000) {
#if defined(MTK_PQ_SERVICE)
                        int32_t module, field, value;

                        module = (RegBase >> 16) & 0xffff;
                        field = (RegBase & 0xffff);
                        value = RegValue;

                        android::hardware::Return<Result> ret = service->setTuningField(module, field, value);
                        if (!ret.isOk() || ret != Result::OK) {
                            ALOGE("[PQ_CMD] IPictureQuality::setTuningField failed!");
                            sprintf(response, "%08x\r\n", 0);
                        }
                        else {
                            sprintf(response, "%08x\r\n", value);
                            ALOGD("[PQ_CMD] SW: %x:%x = 0x%x(%d)", module, field, (unsigned int)(value), value);
                        }
#else
                        sprintf(response, "%08x\r\n", 0);
                        ALOGE("[PQ_CMD] Not support tuning");
#endif
                } else {
                    DISP_WRITE_REG params;

                    params.reg = (RegBase); // For compatible
                    params.val = RegValue;
                    params.mask = 0xFFFFFFFF;
#if defined(MTK_PQ_SERVICE)
                    int32_t module, field, value;

                    module = (RegBase >> 16) & 0xffff;
                    field = (RegBase & 0xffff);
                    value = RegValue;

                    if (params.reg >= SWREG_PQDS_DS_EN && params.reg < SWREG_PQDS_END) {
                        android::hardware::Return<Result> ret = service->setTuningField(
                            MOD_DYNAMIC_SHARPNESS, RegBase - SWREG_PQDS_DS_EN, value);
                        if (!ret.isOk() || ret != Result::OK) {
                            ALOGE("[PQ_CMD] IPictureQuality::setTuningField failed!");
                        }
                    }
                    else if (params.reg >= SWREG_TDSHP_REG_GAIN_MID && params.reg < SWREG_TDSHP_REG_END) {
                        android::hardware::Return<Result> ret = service->setTuningField(MOD_TDSHP_REG, RegBase - SWREG_TDSHP_REG_GAIN_MID, value);
                        if (!ret.isOk() || ret != Result::OK) {
                            ALOGE("[PQ_CMD] IPictureQuality::setTuningField failed!");
                        }
                    }
                    else if (params.reg == SWREG_TDSHP_TUNING_MODE) {
                        android::hardware::Return<Result> ret = service->setTDSHPFlag(params.val);
                        if (!ret.isOk() || ret != Result::OK) {
                            ALOGE("[PQ_CMD] IPictureQuality::setTDSHPFlag failed!");
                        }
                    }
                    else if (params.reg >= SWREG_BLACK_EFFECT_ENABLE && params.reg < SWREG_DC_END) {
                        android::hardware::Return<Result> ret = service->setTuningField(MOD_DYNAMIC_CONTRAST, RegBase - SWREG_BLACK_EFFECT_ENABLE, value);
                        if (!ret.isOk() || ret != Result::OK) {
                            ALOGE("[PQ_CMD] IPictureQuality::setTuningField failed!");
                        }
                    }
                    else
                    {
                        arg.request = ioctlRequest::IOCTL_WRITE_SW_REG;
                        arg.reg = params.reg;
                        arg.val = params.val;
                        arg.mask = params.mask;

                        android::hardware::Return<void> ret = service->execIoctl(arg,
                            [&] (Result retval, const dispPQIoctlParams &retParam)
                        {
                            if (retval != Result::OK)
                            {
                                __UNUSED(retParam);
                                ALOGE("[PQ_CMD] service->execIoctl [%d] failed", arg.request);
                            }
                        });
                        if (!ret.isOk()) {
                            ALOGE("[PQ_CMD] IPictureQuality::execIoctl failed!");
                        }
                    }

                    ALOGD("[PQ_CMD] SW: 0x%08x = %08x", params.reg, params.val);

                    sprintf(response,"%08x\r\n", params.val);
#else
                    sprintf(response, "%08x\r\n", 0);
                    ALOGE("[PQ_CMD] Not support tuning");
#endif
                }
            }
            else if (actionID == AAL_SW_READ)
            {
                unsigned int value = 0;

                at_tok_nexthexint(&cmdline, &RegBase);

                if (RegBase < AAL_INTERNAL_FIELDS_END) {
                    if (AALClient_readField((unsigned int)RegBase, &value) == 0) {
                        ALOGD("[AAL_CMD] SR: %u = %d(0x%08x)", RegBase, (int)value, value);
                    } else {
                        ALOGD("[AAL_CMD] SR: %u read error", RegBase);
                    }
                } else if (AAL_ADAPT_VIRTUAL_ADDRESS_BASE <= RegBase && RegBase < AAL_ADAPT_VIRTUAL_ADDRESS_END) {
                    value = readAALAdaptVariable(RegBase);
                    ALOGD("[AAL_CMD] SR: %u = %d(0x%08x)", RegBase, (int)value, value);
                }

                sprintf(response,"%08x\r\n", value);
            }
            else if (actionID == AAL_SW_WRITE)
            {
                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                if (RegBase < AAL_INTERNAL_FIELDS_END) {
                    if (AALClient_writeField((unsigned int)RegBase, (unsigned int)RegValue) == 0) {
                        ALOGD("[AAL_CMD] SW: %u = %d(0x%08x)", RegBase, RegValue, (unsigned int)RegValue);
                    } else {
                        ALOGD("[AAL_CMD] SW: %u = %d(0x%08x) write error", RegBase, RegValue, (unsigned int)RegValue);
                    }
                } else if (AAL_ADAPT_VIRTUAL_ADDRESS_BASE <= RegBase && RegBase < AAL_ADAPT_VIRTUAL_ADDRESS_END) {
                    ALOGD("[AAL_CMD] SW: %u = %d(0x%08x)", RegBase, RegValue, (unsigned int)RegValue);
                    writeAALAdaptVariable(RegBase, RegValue);
                }

                sprintf(response,"%08x\r\n", RegValue);
            }
            else if (actionID == IDLEMGR_CONTROL)
            {
                int32_t fd;
                size_t s;
                char cmd[22] = "echo enable_idlemgr:0";

                fd = open("/d/mtkfb", O_WRONLY);

                if (fd < 0)
                {
                    ALOGE("fail to open: %s\n", "/d/mtkfb");
                    break;
                }

                s = write(fd, (void *)&cmd, sizeof(cmd) - 1);
                close(fd);

                if (s <= 0)
                {
                    ALOGE("could not write /d/mtkfb");
                    break;
                }

                ALOGD("[PQ_CMD] Disable IdleMgr");
            }
            break;
        default:
            break;
    }

    clock_gettime(CLOCK_REALTIME, &tt2);
    ALOGD("[PQ_CMD] pq cmd handler end... comsumes [%ld] nano-sec!", tt2.tv_nsec - tt1.tv_nsec);

    return 0;
}



