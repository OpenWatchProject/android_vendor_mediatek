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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <cstring>

#include <dlfcn.h>

extern "C" {
#include "atci_cct_cmd.h"
#include "atcid_util.h"
#include "at_tok.h"
}

#define LOG_TAG "ATCI_CCT_CMD"
#include <cutils/log.h>

#define MY_LOGI(fmt, arg...)    ALOGI("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define MY_LOGD(fmt, arg...)    ALOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)

//#include "meta_ccap_para.h"

#if defined(__LP64__)
#define CCAP_LIB_PATH "/system/vendor/lib64/libccap.so"
#else
#define CCAP_LIB_PATH "/system/vendor/lib/libccap.so"
#endif

typedef enum
{
    ACDK_CCT_IO_ADB = 0,
    ACDK_CCT_IO_ATCI_NEXT,
    ACDK_CCT_IO_ATCI_END
} ACDK_CCT_IO_TYPE_ENUM;
static int FT_CCT_OP_END = -1;
static int sizeof_FT_CCT_REQ = -1;
static int sizeof_FT_CCT_CNF = -1;

//4 types of operation codes: ACTION / TEST(=?) / READ(?) / SET(=)

static int Req_Cnt = -1;    // unit = 4 bytes
static int Cnf_Cnt = -1;    // unit = 4 bytes
static void *CCT_Req = NULL;
static void *CCT_Cnf = NULL;
static bool isInit = false;

// CCT HIDL
#include <vendor/mediatek/hardware/camera/ccap/1.0/ICCAPControl.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

using vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_memory;
using ::android::sp;
//

bool allocate_hidl_mem(int size, hidl_memory& hidlHeap, native_handle_t** hidlHandle)
{
    bool allocSuccess = false;

    // allocate memory
    sp<IAllocator> ashmemAllocator = IAllocator::tryGetService("ashmem");
    if( ashmemAllocator == nullptr)
    {
        MY_LOGE("hidl: get ashmem service failed!");
        return false;
    }

    ashmemAllocator->allocate(size, [&](bool success, const android::hardware::hidl_memory& mem)
    {
        if (success)
        {
            /* ok */
            allocSuccess = true;
            *hidlHandle = native_handle_clone(mem.handle());
            hidlHeap = hidl_memory("ashmem", *hidlHandle, size);
            MY_LOGD("hidl: mem allocate ok");
        }
        else
        {
            /* error */
            MY_LOGE("hidl: mem allocate failed!");
        }
    // now you can use the hidl_memory object 'mem' or pass it around
    });

    return allocSuccess;
}

extern "C"
bool ccap_init()
{
    MY_LOGE("+");

    if(isInit)
    {
        return true;
    }

    sp<ICCAPControl> ccapCtrl = ICCAPControl::tryGetService("internal/0");
    if(ccapCtrl == nullptr)
    {
        MY_LOGE("hidl: get ccapCtrl service failed!");
        return false;
    }

    hidl_memory hidlHeap;
    native_handle_t* hidlHandle;

    if(!allocate_hidl_mem(1024, hidlHeap, &hidlHandle))
    {
        MY_LOGE("hidl: mem allocate failed!");
        return false;
    }

    // init const & memory
    if(ccapCtrl->intf_ccap_const(hidlHeap) != 0)
    {
        MY_LOGE("hidl: func_ccap_const failed!");
    }

    sp<IMemory> memory = mapMemory(hidlHeap);
    int32_t* data = static_cast<int32_t*>(static_cast<void*>(memory->getPointer()));

    sizeof_FT_CCT_REQ = data[0];
    sizeof_FT_CCT_CNF = data[1];
    FT_CCT_OP_END = data[2];

    MY_LOGD("reqSize(%d), cnfSize(%d), opEnd(%d)", sizeof_FT_CCT_REQ, sizeof_FT_CCT_CNF, FT_CCT_OP_END);

    if( FT_CCT_OP_END == -1 || sizeof_FT_CCT_REQ == -1 || sizeof_FT_CCT_CNF == -1)
    {
        MY_LOGE("invalid const value!");
        return false;
    }

    if(CCT_Req != NULL)
    {
        free( CCT_Req );
    }
    if(CCT_Cnf != NULL)
    {
        free( CCT_Cnf );
    }
    CCT_Req = malloc(sizeof_FT_CCT_REQ);
    CCT_Cnf = malloc(sizeof_FT_CCT_CNF);

    if (hidlHandle) {
        native_handle_close(hidlHandle);
        native_handle_delete(hidlHandle);
    }

    isInit = true;

    MY_LOGE("-");

    return true;
}

int cct_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
        MY_LOGD("cct cmd handler handles cmdline:%s", cmdline);
        int  CCT_Operation = -1;

        sp<ICCAPControl> ccapCtrl = ICCAPControl::tryGetService("internal/0");
        if(ccapCtrl == nullptr)
        {
            MY_LOGE("hidl: get ccapCtrl service failed!");
            return false;
        }

        switch(at_op){
        case AT_ACTION_OP:
        case AT_TEST_OP:
        case AT_READ_OP:
            sprintf(response,"CCT_at_op(%d)\r\n", at_op);
            break;
        case AT_SET_OP:

            if (at_tok_nextint(&cmdline, &CCT_Operation) == -1) // retrieve operation from input string
            {
                MY_LOGE("Grab output size failed");
                sprintf(response,"AT_SET_OP, CCT_op(failed) Grab output size failed\r\n");
                goto EXIT;
            }
            /*
            if( CCT_Operation < 0 || CCT_Operation >= (int)FT_CCT_OP_END )
            {
                MY_LOGE("[cct_cmd_handler] op=%d is out of range", CCT_Operation);
                sprintf(response,"\r\nOperation is out of range\r\n");
                goto EXIT;
            }
            */
            MY_LOGD("op=%d", CCT_Operation);

            if( ccap_init() == false )
            {
                sprintf(response,"ccap_init failed\r\n");
                goto EXIT;
            }

            if( ccapCtrl->intf_ccap_init() != 0 )
            {
                MY_LOGE("hidl: func_ccap_init failed!");
                sprintf(response,"CCT_op(failed) META_CCAP_init failed\r\n");
                goto EXIT;
            }

            if(CCT_Operation == ACDK_CCT_IO_ADB)
            {
                MY_LOGD("Send operation to META_CCAP_ADB_OP");
                if(ccapCtrl->intf_ccap_adb_op() != 0)
                {
                    MY_LOGE("hidl: func_ccap_adb_op failed!");
                }
                sprintf(response,"CCT_op(%d)\r\n", CCT_Operation);
            }
            else if(CCT_Operation == ACDK_CCT_IO_ATCI_NEXT)
            {
                MY_LOGD("Receive data to buffer");
                int Value = 0;
                if(Req_Cnt == -1)
                {
                    Req_Cnt = 0;
                    memset(CCT_Req, 0, sizeof_FT_CCT_REQ);
                    memset(CCT_Cnf, 0, sizeof_FT_CCT_CNF);
                }

                if(Cnf_Cnt == -1)
                {
                    if(at_tok_nextint(&cmdline, &Value) == -1)
                    {
                        MY_LOGE("Grab output size failed");
                        sprintf(response,"CCT_op(failed) Grab output size failed\r\n");
                        goto EXIT;
                    }

                    Cnf_Cnt = Value;
                    MY_LOGD("Grab output size(%d)", Cnf_Cnt);
                }

                for(int i=0; i<20; i++)
                {
                    if(at_tok_nextint(&cmdline, &Value) == -1)
                    {
                        MY_LOGD("Copy data finish, wait for next");
                        break;
                    }

                    int *Dest = ((int *)CCT_Req) + Req_Cnt;

                    MY_LOGD("Copy data(%d), offset(%d)", Value, Req_Cnt);
                    memcpy(Dest, &Value, 4);
                    Req_Cnt++;
                }

                sprintf(response,"CCT_op(%d)\r\n", CCT_Operation);
            }
            else if(CCT_Operation == ACDK_CCT_IO_ATCI_END)
            {
                MY_LOGD("Send operation to META_CCAP_ATCI_OP");
                hidl_memory reqHidlHeap;
                hidl_memory cnfHidlHeap;
                native_handle_t* reqHidlHandle;
                native_handle_t* cnfHidlHandle;

                if(!allocate_hidl_mem(sizeof_FT_CCT_REQ, reqHidlHeap, &reqHidlHandle))
                {
                    MY_LOGE("hidl: mem allocate failed!");
                    return false;
                }

                if(!allocate_hidl_mem(sizeof_FT_CCT_CNF, cnfHidlHeap, &cnfHidlHandle))
                {
                    MY_LOGE("hidl: mem allocate failed!");
                    return false;
                }

                sp<IMemory> memoryReq = mapMemory(reqHidlHeap);
                sp<IMemory> memoryCnf = mapMemory(cnfHidlHeap);
                void* pReg = static_cast<void*>(static_cast<void*>(memoryReq->getPointer()));
                void* pCnf = static_cast<void*>(static_cast<void*>(memoryCnf->getPointer()));

                memoryReq->update();
                memoryCnf->update();
                memcpy(pReg, CCT_Req, sizeof_FT_CCT_REQ);
                memcpy(pCnf, CCT_Cnf, sizeof_FT_CCT_CNF);
                memoryReq->commit();
                memoryCnf->commit();

                ccapCtrl->intf_ccap_atci_op(reqHidlHeap, cnfHidlHeap, -1);

                int *Source = ((int *)pCnf);

                // header + op + status
                sprintf(response, "CCT_op(%d),%d,%d,%d", CCT_Operation, Source[0], Source[1], Source[2]);
                Source += 3;
                for(int i=0; i<Cnf_Cnt; i++)
                {
                    char var[20] = { 0 };
                    sprintf(var, ",%d", Source[i]);
                    strncat(response, var, 20);
                }

                Req_Cnt = -1;
                Cnf_Cnt = -1;

                if (reqHidlHandle) {
                    native_handle_close(reqHidlHandle);
                    native_handle_delete(reqHidlHandle);
                }

                if (cnfHidlHandle) {
                    native_handle_close(cnfHidlHandle);
                    native_handle_delete(cnfHidlHandle);
                }
                MY_LOGE("-");
            }
            else
            {
                sprintf(response,"CCT_op(failed) CCT_Operation(%d) is not supported\r\n", CCT_Operation);
            }
            break;
        default:
            sprintf(response,"CCT_op(failed) at_op(%d) is not supported\r\n", at_op);
            break;
        }

EXIT:
        MY_LOGD("response=%s", response);
        //if(CcapHwhndl){
        //    dlclose(CcapHwhndl);
        //}
        return 0;
}
