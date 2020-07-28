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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _CCU_BUFFER_H_
#define _CCU_BUFFER_H_

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include "ccu_udrv.h"
#include "ccuif_compat.h"
#include "ccu_cmd_param_info.h"
#include "iccu_mgr.h"
#include "ccu_platform_def.h"

namespace NSCcuIf {

enum CCU_BUFFER_TYPE
{
    CCU_BUFTYPE_NONE = 0,
    CCU_BUFTYPE_MIN,
    //buffer type for DRAM buffer where CCU binary to load into
    CCU_BUFTYPE_DDRBIN = CCU_BUFTYPE_MIN,
    //buffer type for CCU IPC command I/O buffer
    //CCU_BUFTYPE_CTRL,
    //buffer type for buffers that CPU will reference, but managed by CCU
    CCU_BUFTYPE_CPUREF,
    CCU_BUFTYPE_MAX,
};

#define CCU_BUFFER_RING_CAPACITY 2
struct CcuBuffer
{
    //fields assigned by caller of CcuBuffer::init()
    const char *name;
    unsigned int size;
    enum CCU_BUFFER_TYPE bufType;

    //fields decided in CcuBuffer::init() by CcuMemMgr::allocateCcuBuffer(...)
    uint32_t ringCnt;
    char *va[CCU_BUFFER_RING_CAPACITY];
    unsigned int mva[CCU_BUFFER_RING_CAPACITY];
    bool cached;

    //runtime use variable
    uint32_t ringIdx;

    void reset();
    bool init(const char *name, int size, enum CCU_BUFFER_TYPE bufType,  uint32_t ringCnt=1);
    bool ring();
    char * getVa();
    unsigned int getMva();
};

struct CcuBufferList
{
public:
    bool isValid;
    struct CcuBuffer mailboxInBuf;
    struct CcuBuffer mailboxOutBuf;
    struct CcuBuffer CtrlMsgBufs[CCU_MSG_CNT_MAX];

    CcuBufferList();
    virtual ~CcuBufferList() {}
    virtual bool init();

protected:
    enum ccu_feature_type m_featureType;
    bool _initControlBuffers();
};

struct CcuAfBufferList : CcuBufferList
{
public:
    struct CcuBuffer AF_Buf[Half_AFO_BUFFER_COUNT];
    struct CcuBuffer AF_Hwinit_Output[CCU_AF_INSTANCE_CAPACITY];

    bool init();

private:
    bool _initAFBuffers();
};

//TODO: Imp this func.
struct CcuLtmBufferList : CcuBufferList
{
public:
    struct CcuBuffer ltm_info_exif[CCU_LTM_DBG_RING_SIZE];
    struct CcuBuffer ltm_info_isp[CCU_LTM_DBG_RING_SIZE];
    struct CcuBuffer hlr_info_exif[CCU_LTM_DBG_RING_SIZE];
    struct CcuBuffer hlr_info_isp[CCU_LTM_DBG_RING_SIZE];
    struct CcuBuffer ltmso_ring_buf[CCU_LTMSO_RING_SIZE];
    // struct CcuBuffer ltm_nvram;
    // struct CcuBuffer hlr_nvram;
    bool init();

private:
    bool _initLtmBuffers();
};

struct CcuAeBufferList : CcuBufferList
{
public:
    struct CcuBuffer hdrDataBuffer[CCU_HDR_DATA_BUF_CNT];
    struct CcuBuffer actoutAeAlgoData[AE_ACTOUT_RING_SIZE];
    struct CcuBuffer actoutAeOutput[AE_ACTOUT_RING_SIZE];
    struct CcuBuffer actoutAeStat[AE_ACTOUT_RING_SIZE];

    bool init();

private:
    bool _initAeBuffers();
};

struct Ccu3ASyncBufferList : CcuBufferList
{
public:
    bool init();
};

struct CcuSystemBufferList : CcuBufferList
{
public:
    bool init();
};

};  //namespace NSCcuIf

#endif