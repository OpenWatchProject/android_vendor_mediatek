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
#ifndef __I_LSC_BUF_H__
#define __I_LSC_BUF_H__

#include <aaa_types.h>
#include <mtkcam/aaa/ILscTable.h>

typedef struct
{
    int    IMG_WD;
    int    IMG_HT;
    int    TSFS_W_HNUM;
    int    TSFS_W_VNUM;
    int    TSFS_W_HORG;
    int    TSFS_W_VORG;
    int    TSFS_W_HPIT;
    int    TSFS_W_VPIT;
    int    TSFS_W_HSIZE;
    int    TSFS_W_VSIZE;
    int    TSFS_PIXEL_CNT0;
    int    TSFS_PIXEL_CNT1;
    int    TSFS_PIXEL_CNT2;
} TSFS_INFO_STRUCT;

namespace NSIspTuning
{
class ILscBuf
{
public:
    struct Config
    {
        MINT32 i4ImgWd;
        MINT32 i4ImgHt;
        MINT32 i4BlkX;
        MINT32 i4BlkY;
        MINT32 i4BlkW;
        MINT32 i4BlkH;
        MINT32 i4BlkLastW;
        MINT32 i4BlkLastH;
    };

    typedef enum
    {
        E_LSC_IMEM = 0,
        E_LSC_IMGBUF = 1
    } E_BUF_MODE_T;
                            ILscBuf(MUINT32 sensorDev, MUINT32 u4Id, MBOOL usecache, const char* strName, E_BUF_MODE_T eBufMode = E_LSC_IMEM);
    virtual                 ~ILscBuf();

    virtual MUINT32*        editTable();
    virtual ILscBuf::Config getConfig() const;
    virtual MINT32          getFD() const;
    virtual MUINT32         getPhyAddr() const;
    virtual MUINT32         getSize() const;
    virtual MUINT32         getRatio() const;
    virtual const char*     getName() const;
    virtual MBOOL           setConfig(ILscBuf::Config rCfg);
#if (CAM3_3A_IP_BASE)
    virtual MBOOL           setConfig(ILscTable::Config rCfg);
#endif
    virtual const MUINT32*  getTable() const;
    virtual MBOOL           setTable(const void* data, MUINT32 u4Size);
    virtual MBOOL           setRatio(MUINT32 u4Ratio);
    virtual MBOOL           validate();
    virtual MBOOL           showInfo() const;
    virtual MBOOL           dump(const char* filename) const;
    virtual void*           getBuf();

                            class LscBufImp;

protected:
    LscBufImp*              m_pImp;
};
};
#endif //__LSC_UTIL_H__
