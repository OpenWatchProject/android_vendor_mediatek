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
#define LOG_TAG "lsc_buf"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include <LscUtil.h>
#include <lsc/ILscBuf.h>
#include <imem_3a.h>
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

using namespace android;
using namespace NSCam;
using namespace NSIspTuning;

/*******************************************************************************
 * ILscBuf::LscBufImp
 *******************************************************************************/
class ILscBuf::LscBufImp
{
public:
                            LscBufImp(MUINT32 sensorDev, MUINT32 u4Id, const char* strName);
    virtual                 ~LscBufImp();

    virtual ILscBuf::Config getConfig() const {return m_rConfig;}
    virtual MINT32          getFD() const = 0;
    virtual MUINT32         getPhyAddr() const = 0;
    virtual MBOOL           setConfig(ILscBuf::Config rCfg) = 0;
    virtual const MUINT32*  getTable() const = 0;
    virtual MUINT32         getSize() const = 0;
    virtual MUINT32         getRatio() const {return m_u4Rto;}
    virtual const char*     getName() const {return m_strName.c_str();}
    virtual MBOOL           setTable(const void* data, MUINT32 u4Size) = 0;
    virtual MBOOL           setRatio(MUINT32 u4Ratio) { m_u4Rto = u4Ratio; return MTRUE;}
    virtual MUINT32*        editTable() = 0;
    virtual MBOOL           validate() = 0;
    virtual MBOOL           showInfo() const = 0;
    virtual MBOOL           dump(const char* filename) const;
    virtual void*           getBuf() = 0;

protected:
    MUINT32                 m_u4SensorDev;
    MUINT32                 m_u4Id;
    MUINT32                 m_u4Rto;
    ILscBuf::Config         m_rConfig;
    std::string             m_strName;
};

ILscBuf::LscBufImp::
LscBufImp(MUINT32 sensorDev, MUINT32 u4Id, const char* strName)
    : m_u4SensorDev(sensorDev)
    , m_u4Id(u4Id)
    , m_u4Rto(32)
    , m_strName(strName)
{
}

ILscBuf::LscBufImp::
~LscBufImp()
{
}

MBOOL
ILscBuf::LscBufImp::
dump(const char* filename) const
{
    char strFilename[512];
    FILE *fhwtbl,*fsdblk;

    const MUINT32* pData = getTable();
    showInfo();

    if (pData == NULL)
    {
        LSC_ERR("NULL buffer");
        return MFALSE;
    }

    LSC_LOG_BEGIN();

    sprintf(strFilename, "%s.sdblk", filename);
    fsdblk = fopen(strFilename, "w");
    if ( fsdblk == NULL )
    {
        LSC_ERR("Can't open: %s", strFilename);
        return MFALSE;
    }

    sprintf(strFilename, "%s.hwtbl", filename);
    fhwtbl = fopen(strFilename, "w");
    if ( fhwtbl == NULL )
    {
        LSC_ERR("Can't open: %s", strFilename);
        fclose(fsdblk);
        return MFALSE;
    }

    fprintf(fsdblk," %8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
            0 /*LscConfig.ctl1.bits.SDBLK_XOFST*/,
            0 /*LscConfig.ctl1.bits.SDBLK_YOFST*/,
            m_rConfig.i4BlkW /*LscConfig.ctl2.bits.LSC_SDBLK_WIDTH*/,
            m_rConfig.i4BlkH /*LscConfig.ctl3.bits.LSC_SDBLK_HEIGHT*/,
            m_rConfig.i4BlkX /*LscConfig.ctl2.bits.LSC_SDBLK_XNUM*/,
            m_rConfig.i4BlkY /*LscConfig.ctl3.bits.LSC_SDBLK_YNUM*/,
            m_rConfig.i4BlkLastW /*LscConfig.lblock.bits.LSC_SDBLK_lWIDTH*/,
            m_rConfig.i4BlkLastH /*LscConfig.lblock.bits.LSC_SDBLK_lHEIGHT*/);

    MINT32 x_num = m_rConfig.i4BlkX + 1;
    MINT32 y_num = m_rConfig.i4BlkY + 1;

    MINT32 numCoef = x_num * y_num * 4 * 6;
    MINT32 i;

    for (i = numCoef-1; i >= 0; i-=6)
    {
        MUINT32 coef1, coef2;
        MUINT32 val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        val = *pData++;
        coef2 = (val& 0xFFFF0000) >> 16;
        coef1 = (val& 0x0000FFFF);
        fprintf(fsdblk, " %8d %8d", coef1, coef2);
        fprintf(fhwtbl,"0x%08x, ", val);

        fprintf(fhwtbl,"\n");
        fprintf(fsdblk,"\n");
    }

    fclose(fhwtbl);
    fclose(fsdblk);

    LSC_LOG_END();

    return MTRUE;
}

#define DEFAULT_TABLE_SIZE (16*16*4*4*6)
#define TSF_IN_SIZE (19200)
#define TSF_OUT_SIZE (2400)

#define GET_PROP(prop, init, val)\
{\
    val = property_get_int32(prop, (init));\
}


/*******************************************************************************
 * LscImemBuf
 *******************************************************************************/
class LscImemBuf : public ILscBuf::LscBufImp
{
public:
                            LscImemBuf(MUINT32 sensorDev, MUINT32 u4Id, MBOOL usecache, const char* strName);
    virtual                 ~LscImemBuf();

    //virtual ILscBuf::Config getConfig() const;
    virtual MINT32          getFD() const;
    virtual MUINT32         getPhyAddr() const;
    virtual MBOOL           setConfig(ILscBuf::Config rCfg);
    virtual const MUINT32*  getTable() const;
    virtual MUINT32         getSize() const;
    virtual MBOOL           setTable(const void* data, MUINT32 u4Size);
    virtual MBOOL           validate();
    virtual MUINT32*        editTable();
    virtual MBOOL           showInfo() const;
    virtual void*           getBuf(){return &m_rRawLscInfo;}

protected:
    virtual MBOOL           initMemBuf(MINT32 usecache);
    virtual MBOOL           uninitMemBuf();
    virtual MBOOL           allocMemBuf(IMEM_BUF_INFO& rBufInfo, MUINT32 const u4Size);
    virtual MBOOL           freeMemBuf(IMEM_BUF_INFO& rBufInfo);

    IMem3A*                m_pIMemDrv;
    IMEM_BUF_INFO           m_rRawLscInfo;

private:
    MINT32                  m_LogEnable;
};

LscImemBuf::
LscImemBuf(MUINT32 sensorDev, MUINT32 u4Id, MBOOL usecache, const char* strName)
    : ILscBuf::LscBufImp(sensorDev, u4Id, strName)
    , m_pIMemDrv(NULL)
    , m_LogEnable(0)
{
    GET_PROP("debug.lsc_mgr.log", 0, m_LogEnable);
    initMemBuf(usecache);
}

LscImemBuf::
~LscImemBuf()
{
    uninitMemBuf();
}

MBOOL
LscImemBuf::
initMemBuf(MBOOL usecache)
{
    LSC_LOG_IF(m_LogEnable, "m_u4SensorDev(0x%02x), id(%d), name(%s)", m_u4SensorDev, m_u4Id, m_strName.c_str());

    MBOOL ret = MFALSE;

    MUINT32 i = 0;

    MUINT32 iMemSize = DEFAULT_TABLE_SIZE;

    if (m_strName.compare("TSF_in") == 0 )
    {
        iMemSize = TSF_IN_SIZE;
    }
    else if (m_strName.compare("TSF_out") == 0 )
    {
        iMemSize = TSF_OUT_SIZE;
    }

    ret = MTRUE;
    if (!m_pIMemDrv)
    {
        m_pIMemDrv = IMem3A::createInstance();

        if (!m_pIMemDrv)
        {
            LSC_LOG("m_pIMemDrv create Fail.");
            ret = MFALSE;
        }
        else
        {
            ret = m_pIMemDrv->init();
            if (ret == MTRUE)
            {
                m_rRawLscInfo.useNoncache=!usecache;
                if (!allocMemBuf(m_rRawLscInfo, iMemSize))
                {
                    LSC_ERR("allocMemBuf sensor(0x%02x), id(%d) FAILED", m_u4SensorDev, m_u4Id);
                }
                else
                {
                    LSC_LOG_IF(m_LogEnable, "sensor(0x%02x), id(%d), memID(%d), virtAddr(%d), phyAddr(%d), size(%d)",
                        m_u4SensorDev, m_u4Id, m_rRawLscInfo.memID, (MINT32)m_rRawLscInfo.virtAddr, (MINT32)m_rRawLscInfo.phyAddr, m_rRawLscInfo.size);
                }
            }
            else
            {
                LSC_ERR("m_pIMemDrv init Fail!");
            }
        }
    }
    else
    {
        LSC_LOG("m_pIMemDrv(%p) already exists.", m_pIMemDrv);
    }

    LSC_LOG_IF(m_LogEnable, "[%s -]", __FUNCTION__);
    return ret;
}

MBOOL
LscImemBuf::
uninitMemBuf()
{
    MUINT32 ret = 0;
    MUINT32 i = 0;

    LSC_LOG_IF(m_LogEnable, "m_u4SensorDev(0x%02x), id(%d), name(%s)", m_u4SensorDev, m_u4Id, m_strName.c_str());

    freeMemBuf(m_rRawLscInfo);

    if (m_pIMemDrv)
    {
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    LSC_LOG_IF(m_LogEnable, "[%s -]", __FUNCTION__);
    return MTRUE;
}

MBOOL
LscImemBuf::
allocMemBuf(IMEM_BUF_INFO& rBufInfo, MUINT32 const u4Size)
{
    MBOOL ret = MFALSE;

    if (!rBufInfo.virtAddr)
    {
        rBufInfo.size = u4Size;
        if (0 == m_pIMemDrv->allocVirtBuf(&rBufInfo))
        {
            if (0 != m_pIMemDrv->mapPhyAddr(&rBufInfo))
            {
                LSC_ERR("mapPhyAddr error, virtAddr(%d), size(%d)\n", (MINT32)rBufInfo.virtAddr, rBufInfo.size);
                ret = MFALSE;
            }
            else
            {
                ret = MTRUE;
            }
        }
        else
        {
            LSC_ERR("allocVirtBuf error, size(%d)\n", rBufInfo.size);
            ret = MFALSE;
        }
    }
    else
    {
        ret = MTRUE;
        LSC_LOG("Already Exist! virtAddr(%d), size(%d)\n", (MINT32)rBufInfo.virtAddr, u4Size);
    }
    return ret;
}

MBOOL
LscImemBuf::
freeMemBuf(IMEM_BUF_INFO& rBufInfo)
{
    MBOOL ret = MTRUE;

    if (!m_pIMemDrv || rBufInfo.virtAddr == 0)
    {
        LSC_ERR("Null m_pIMemDrv driver \n");
        return MFALSE;
    }

    if (0 == m_pIMemDrv->unmapPhyAddr(&rBufInfo))
    {
        if (0 == m_pIMemDrv->freeVirtBuf(&rBufInfo))
        {
            LSC_LOG("freeVirtBuf OK, memID(%d), virtAddr(%d), phyAddr(%d)", rBufInfo.memID, (MINT32)rBufInfo.virtAddr, (MINT32)rBufInfo.phyAddr);
            rBufInfo.virtAddr = 0;
            ret = MTRUE;
        }
        else
        {
            LSC_LOG("freeVirtBuf Fail, memID(%d), virtAddr(%d), phyAddr(%d)", rBufInfo.memID, (MINT32)rBufInfo.virtAddr, (MINT32)rBufInfo.phyAddr);
            ret = MFALSE;
        }
    }
    else
    {
        LSC_ERR("memID(%d) unmapPhyAddr error", rBufInfo.memID);
        ret = MFALSE;
    }

    return ret;
}

MBOOL
LscImemBuf::
showInfo() const
{
    LSC_LOG("sensor(0x%02x), name(%s), id(%d), memID(%d), virtAddr(%d), phyAddr(%d), size(%d), Img(%dx%d), Blk(%d,%d,%d,%d,%d,%d), Rto(%d)",
        m_u4SensorDev, m_strName.c_str(), m_u4Id, m_rRawLscInfo.memID, (MINT32)m_rRawLscInfo.virtAddr, (MINT32)m_rRawLscInfo.phyAddr, m_rRawLscInfo.size,
        m_rConfig.i4ImgWd, m_rConfig.i4ImgHt, m_rConfig.i4BlkX, m_rConfig.i4BlkY, m_rConfig.i4BlkW, m_rConfig.i4BlkH, m_rConfig.i4BlkLastW, m_rConfig.i4BlkLastH, m_u4Rto);
    return MTRUE;
}
MINT32
LscImemBuf::
getFD() const
{
    return m_rRawLscInfo.memID;
}
MUINT32
LscImemBuf::
getPhyAddr() const
{
    return m_rRawLscInfo.phyAddr;
}

const MUINT32*
LscImemBuf::
getTable() const
{
    return (MUINT32*)m_rRawLscInfo.virtAddr;
}

MUINT32*
LscImemBuf::
editTable()
{
    return (MUINT32*)m_rRawLscInfo.virtAddr;
}

MUINT32
LscImemBuf::
getSize() const
{
    return m_rRawLscInfo.size;
}

MBOOL
LscImemBuf::
setConfig(ILscBuf::Config rConfig)
{
    if (rConfig.i4BlkX >= 32 || rConfig.i4BlkX == 0 || rConfig.i4BlkY >= 32 || rConfig.i4BlkY == 0)
    {
        // assert
        LSC_ERR("XNum(%d), YNum(%d)", rConfig.i4BlkX, rConfig.i4BlkY);
        return MFALSE;
    }

    m_rConfig.i4ImgWd = rConfig.i4ImgWd;
    m_rConfig.i4ImgHt = rConfig.i4ImgHt;
    m_rConfig.i4BlkX  = rConfig.i4BlkX;
    m_rConfig.i4BlkY  = rConfig.i4BlkY;
    m_rConfig.i4BlkW  = (rConfig.i4ImgWd)/(2*(rConfig.i4BlkX+1));
    m_rConfig.i4BlkH  = (rConfig.i4ImgHt)/(2*(rConfig.i4BlkY+1));
    m_rConfig.i4BlkLastW = rConfig.i4ImgWd/2 - (rConfig.i4BlkX*m_rConfig.i4BlkW);
    m_rConfig.i4BlkLastH = rConfig.i4ImgHt/2 - (rConfig.i4BlkY*m_rConfig.i4BlkH);

    MUINT32 u4Size = (rConfig.i4BlkX+1) * (rConfig.i4BlkY+1) * 4 * 4 * 6;
    if (u4Size != m_rRawLscInfo.size)
    {
        // need to reallocate memory
        if (!freeMemBuf(m_rRawLscInfo))
        {
            LSC_ERR("Fail to free buf (%d, %d)", m_u4SensorDev, m_u4Id);
        }
        else
        {
            if (!allocMemBuf(m_rRawLscInfo, u4Size))
            {
                LSC_LOG("allocMemBuf sensor(0x%02x), id(%d) FAILED", m_u4SensorDev, m_u4Id);
            }
            else
            {
                LSC_LOG("sensor(0x%02x), id(%d), memID(%d), virtAddr(%d), phyAddr(%d), size(%d)",
                    m_u4SensorDev, m_u4Id, m_rRawLscInfo.memID, (MINT32)m_rRawLscInfo.virtAddr, (MINT32)m_rRawLscInfo.phyAddr, m_rRawLscInfo.size);
            }
        }
    }

    return MTRUE;
}

MBOOL
LscImemBuf::
setTable(const void* data, MUINT32 u4Size)
{
    if (data == NULL || u4Size > m_rRawLscInfo.size)
    {
        LSC_ERR("src(%p), dest(%d), size(%d)", data, (MINT32)m_rRawLscInfo.virtAddr, u4Size);
        return MFALSE;
    }
    ::memcpy((MUINT32*)m_rRawLscInfo.virtAddr, data, u4Size);
    return MTRUE;
}

MBOOL
LscImemBuf::
validate()
{
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &m_rRawLscInfo);
    return MTRUE;
}

/*******************************************************************************
 * LscImgBuf
 *******************************************************************************/
class LscImgBuf : public ILscBuf::LscBufImp
{
public:
                            LscImgBuf(MUINT32 sensorDev, MUINT32 u4Id, const char* strName);
    virtual                 ~LscImgBuf();
    virtual MINT32          getFD() const;
    virtual MUINT32         getPhyAddr() const;
    virtual MBOOL           setConfig(ILscBuf::Config rCfg);
    virtual const MUINT32*  getTable() const;
    virtual MUINT32         getSize() const;
    virtual MBOOL           setTable(const void* data, MUINT32 u4Size);
    virtual MUINT32*        editTable();
    virtual MBOOL           validate();
    virtual MBOOL           showInfo() const;
    virtual void*           getBuf(){return (void*)m_pImgBuf.get();}

protected:
    virtual MBOOL           allocBuf(MUINT32 x_size, MUINT32 y_size);
    virtual MBOOL           freeBuf();

    sp<NSCam::IImageBuffer> m_pImgBuf;
};

LscImgBuf::
LscImgBuf(MUINT32 sensorDev, MUINT32 u4Id, const char* strName)
    : ILscBuf::LscBufImp(sensorDev, u4Id, strName)
    , m_pImgBuf(NULL)
{
    allocBuf(0x600, 0x10);
}

LscImgBuf::
~LscImgBuf()
{
    freeBuf();
}

MBOOL
LscImgBuf::
allocBuf(MUINT32 x_size, MUINT32 y_size)
{
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {x_size, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam((NSCam::EImageFormat)NSCam::eImgFmt_STA_BYTE,
            NSCam::MSize(x_size, y_size), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(m_strName.c_str(), imgParam);
    if (pHeap == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", m_strName.c_str());
        return MFALSE;
    }
    sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer();
    if (pImgBuf == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", m_strName.c_str());
        return MFALSE;
    }
    // lock buffer
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(pImgBuf->lockBuf(m_strName.c_str(), usage)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  m_strName.c_str());
        return MFALSE;
    }

    m_pImgBuf = pImgBuf;

    LSC_LOG("sensor(0x%02x), name(%s), id(%d), virtAddr(%p), phyAddr(%d), size(%d)",
        m_u4SensorDev, m_strName.c_str(), m_u4Id, getTable(), getPhyAddr(), getSize());

    return MTRUE;
}

MBOOL
LscImgBuf::
freeBuf()
{
    MUINT32 u4PhyAddr = getPhyAddr();
    const void* pVirAddr = getTable();
    if (m_pImgBuf == NULL) {
        CAM_LOGE("sensorDev(0x%02x), id(%d), name(%s): Stuff ImageBuffer not exist", m_u4SensorDev, m_u4Id, m_strName.c_str());
        return MFALSE;
    }

    m_pImgBuf->unlockBuf(m_strName.c_str());
    // destroy buffer
    m_pImgBuf = NULL;
    //
    LSC_LOG("sensor(0x%02x), name(%s), id(%d), virtAddr(%p), phyAddr(%d)", m_u4SensorDev, m_strName.c_str(), m_u4Id, pVirAddr, u4PhyAddr);
    return MTRUE;
}

MBOOL
LscImgBuf::
showInfo() const
{
    LSC_LOG("sensor(0x%02x), name(%s), id(%d), virtAddr(%p), phyAddr(0x%08x), size(%d), Img(%dx%d), Blk(%d,%d,%d,%d,%d,%d), Rto(%d)",
        m_u4SensorDev, m_strName.c_str(), m_u4Id, getTable(), getPhyAddr(), getSize(),
        m_rConfig.i4ImgWd, m_rConfig.i4ImgHt, m_rConfig.i4BlkX, m_rConfig.i4BlkY, m_rConfig.i4BlkW, m_rConfig.i4BlkH, m_rConfig.i4BlkLastW, m_rConfig.i4BlkLastH, m_u4Rto);

    return MTRUE;
}
MINT32
LscImgBuf::
getFD() const
{
    return m_pImgBuf->getFD(0);
}
MUINT32
LscImgBuf::
getPhyAddr() const
{
    return m_pImgBuf->getBufPA(0);
}

const MUINT32*
LscImgBuf::
getTable() const
{
    return (MUINT32*)m_pImgBuf->getBufVA(0);
}

MUINT32*
LscImgBuf::
editTable()
{
    return (MUINT32*)m_pImgBuf->getBufVA(0);
}

MUINT32
LscImgBuf::
getSize() const
{
    return m_pImgBuf->getBufSizeInBytes(0);
}

MBOOL
LscImgBuf::
setConfig(ILscBuf::Config rConfig)
{
    if (rConfig.i4BlkX >= 32 || rConfig.i4BlkX == 0 || rConfig.i4BlkY >= 32 || rConfig.i4BlkY == 0)
    {
        // assert
        LSC_ERR("XNum(%d), YNum(%d)", rConfig.i4BlkX, rConfig.i4BlkY);
        return MFALSE;
    }

    m_rConfig.i4ImgWd = rConfig.i4ImgWd;
    m_rConfig.i4ImgHt = rConfig.i4ImgHt;
    m_rConfig.i4BlkX  = rConfig.i4BlkX;
    m_rConfig.i4BlkY  = rConfig.i4BlkY;
    m_rConfig.i4BlkW  = (rConfig.i4ImgWd)/(2*(rConfig.i4BlkX+1));
    m_rConfig.i4BlkH  = (rConfig.i4ImgHt)/(2*(rConfig.i4BlkY+1));
    m_rConfig.i4BlkLastW = rConfig.i4ImgWd/2 - (rConfig.i4BlkX*m_rConfig.i4BlkW);
    m_rConfig.i4BlkLastH = rConfig.i4ImgHt/2 - (rConfig.i4BlkY*m_rConfig.i4BlkH);

    MUINT32 u4Size = (rConfig.i4BlkX+1) * (rConfig.i4BlkY+1) * 4 * 4 * 6;
    if (u4Size != getSize())
    {
        // need to reallocate memory
        if (!freeBuf())
        {
            LSC_ERR("Fail to free buf (%d, %d)", m_u4SensorDev, m_u4Id);
        }
        else
        {
            MUINT32 LSCI_XSIZE = (m_rConfig.i4BlkX+1)*4*4*6;
            MUINT32 LSCI_YSIZE = m_rConfig.i4BlkY+1;
            if (!allocBuf(LSCI_XSIZE, LSCI_YSIZE))
            {
                LSC_ERR("allocBuf sensor(0x%02x), id(%d) FAILED", m_u4SensorDev, m_u4Id);
            }
            else
            {
                LSC_LOG("sensor(0x%02x), id(%d), virtAddr(%p), phyAddr(%d), size(%d)",
                    m_u4SensorDev, m_u4Id, getTable(), getPhyAddr(), getSize());
            }
        }
    }

    return MTRUE;
}

MBOOL
LscImgBuf::
setTable(const void* data, MUINT32 u4Size)
{
    MUINT32* pDest = (MUINT32*) m_pImgBuf->getBufVA(0);
    if (data == NULL || u4Size > getSize())
    {
        LSC_ERR("src(%p), dest(%p), size(%d)", data, pDest, u4Size);
        return MFALSE;
    }
    ::memcpy(pDest, data, u4Size);
    return MTRUE;
}

MBOOL
LscImgBuf::
validate()
{
    m_pImgBuf->syncCache();
    return MTRUE;
}

/*******************************************************************************
 * ILscBuf
 *******************************************************************************/
ILscBuf::
ILscBuf(MUINT32 sensorDev, MUINT32 u4Id, MBOOL usecache, const char* strName, E_BUF_MODE_T eBufMode)
{
    if (eBufMode == E_LSC_IMEM)
    {
        m_pImp = new LscImemBuf(sensorDev, u4Id, usecache, strName);
    }
    else
    {
        m_pImp = new LscImgBuf(sensorDev, u4Id, strName);
    }
}

ILscBuf::
~ILscBuf()
{
    if (m_pImp) delete m_pImp;
}

ILscBuf::Config
ILscBuf::
getConfig() const
{
    return m_pImp->getConfig();
}
MINT32
ILscBuf::
getFD() const
{
    return m_pImp->getFD();
}
MUINT32
ILscBuf::
getPhyAddr() const
{
    return m_pImp->getPhyAddr();
}

const MUINT32*
ILscBuf::
getTable() const
{
    return m_pImp->getTable();
}

MUINT32*
ILscBuf::
editTable()
{
    return m_pImp->editTable();
}

MUINT32
ILscBuf::
getSize() const
{
    return m_pImp->getSize();
}

MUINT32
ILscBuf::
getRatio() const
{
    return m_pImp->getRatio();
}

const char*
ILscBuf::
getName() const
{
    return m_pImp->getName();
}

#if (CAM3_3A_IP_BASE)
MBOOL
ILscBuf::
setConfig(ILscTable::Config rCfg)
{
    return MTRUE;
}
#endif

MBOOL
ILscBuf::
setConfig(ILscBuf::Config rCfg)
{
    return m_pImp->setConfig(rCfg);
}

MBOOL
ILscBuf::
setTable(const void* data, MUINT32 u4Size)
{
    return m_pImp->setTable(data, u4Size);
}

MBOOL
ILscBuf::
setRatio(MUINT32 u4Ratio)
{
    return m_pImp->setRatio(u4Ratio);
}

MBOOL
ILscBuf::
validate()
{
    return m_pImp->validate();
}

MBOOL
ILscBuf::
showInfo() const
{
    return m_pImp->showInfo();
}

MBOOL
ILscBuf::
dump(const char* strFile) const
{
    return m_pImp->dump(strFile);
}

void*
ILscBuf::
getBuf()
{
    return m_pImp->getBuf();
}
