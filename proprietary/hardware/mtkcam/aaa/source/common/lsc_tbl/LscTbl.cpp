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
#define LOG_TAG "lsc_tbl"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include <lsc/ILscTbl.h>
#include <LscUtil.h>

#include <ShadingTblTransform.h>
#include <mutex>

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_LSCTBL(String) \
          do { \
              aee_system_exception( \
                  LOG_TAG, \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_LSCTBL(String)
#endif

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

using namespace NSIspTuning;
using namespace std;

typedef struct SWAP_PAIR
{
    MINT32 a0;
    MINT32 b0;
    MINT32 a1;
    MINT32 b1;
} SWAP_PAIR_T;

static std::mutex g_algo_mutex;

static const SWAP_PAIR_T _swap[4][4] =
{
    {   // from B
        {0, 0, 0, 0},   // to B
        {ILscTable::BAYER_B, ILscTable::BAYER_GB, ILscTable::BAYER_GR, ILscTable::BAYER_R}, // to GB
        {ILscTable::BAYER_B, ILscTable::BAYER_GR, ILscTable::BAYER_GB, ILscTable::BAYER_R}, // to GR
        {ILscTable::BAYER_B, ILscTable::BAYER_R, ILscTable::BAYER_GB, ILscTable::BAYER_GR}  // to R
    },
    {   // from GB
        {ILscTable::BAYER_B, ILscTable::BAYER_GB, ILscTable::BAYER_GR, ILscTable::BAYER_R}, // to B
        {0, 0, 0, 0}, // to GB
        {ILscTable::BAYER_GB, ILscTable::BAYER_GR, ILscTable::BAYER_B, ILscTable::BAYER_R}, // to GR
        {ILscTable::BAYER_GB, ILscTable::BAYER_R, ILscTable::BAYER_B, ILscTable::BAYER_GR}  // to R
    },
    {   // from GR
        {ILscTable::BAYER_B, ILscTable::BAYER_GR, ILscTable::BAYER_GB, ILscTable::BAYER_R}, // to B
        {ILscTable::BAYER_GR, ILscTable::BAYER_GB, ILscTable::BAYER_R, ILscTable::BAYER_B}, // to GB
        {0, 0, 0, 0}, // to GR
        {ILscTable::BAYER_GR, ILscTable::BAYER_R, ILscTable::BAYER_B, ILscTable::BAYER_GB}  // to R
    },
    {   // from R
        {ILscTable::BAYER_B, ILscTable::BAYER_R, ILscTable::BAYER_GB, ILscTable::BAYER_GR}, // to B
        {ILscTable::BAYER_R, ILscTable::BAYER_GB, ILscTable::BAYER_GR, ILscTable::BAYER_B}, // to GB
        {ILscTable::BAYER_R, ILscTable::BAYER_GR, ILscTable::BAYER_B, ILscTable::BAYER_GB}, // to GR
        {0, 0, 0, 0}  // to R
    },
};

/******************************************************************************
 *
 ******************************************************************************/
ILscTable*
ILscTable::
createInstance(TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
{
    if  ( -1 == i4W && -1 == i4H && -1 == i4GridX && -1 == i4GridY ) {
        return new (std::nothrow) ILscTbl(eType);
    }
    //
    return new (std::nothrow) ILscTbl(eType, i4W, i4H, i4GridX, i4GridY);
}


/*******************************************************************************
 * ILscTable::ConfigBlk
 * ILscTable::Config
 *******************************************************************************/
ILscTable::Config
ILscTbl::
makeConfig(MINT32 _i4ImgWd, MINT32 _i4ImgHt, MINT32 _i4GridX, MINT32 _i4GridY)
{
    ILscTable::Config config;
    config.i4ImgWd = _i4ImgWd;
    config.i4ImgHt = _i4ImgHt;
    config.i4GridX = _i4GridX;
    config.i4GridY = _i4GridY;

    if (config.i4GridX > 200 || config.i4GridX <= 2 || config.i4GridY > 200 || config.i4GridY <= 2)
    {
        // assert
        char strErr[512];
        sprintf(strErr, "Line: %d, Abnormal grid X(%d) Y(%d)", __LINE__, config.i4GridX, config.i4GridY);
        LSC_ERR("%s", strErr);
        AEE_ASSERT_LSCTBL(strErr);
        config.i4GridX = 17;
        config.i4GridY = 17;
    }
    config.rCfgBlk = ILscTable::ConfigBlk(config.i4ImgWd, config.i4ImgHt, config.i4GridX, config.i4GridY);
    return config;
}

static inline
ILscTable::Config
makeConfig(const ILscTable::ConfigBlk& _rCfgBlk)
{
    ILscTable::Config config;
    config.i4ImgWd = (_rCfgBlk.i4BlkW * _rCfgBlk.i4BlkX + _rCfgBlk.i4BlkLastW) * 2;
    config.i4ImgHt = (_rCfgBlk.i4BlkH * _rCfgBlk.i4BlkY + _rCfgBlk.i4BlkLastH) * 2;
    config.i4GridX = _rCfgBlk.i4BlkX + 2;
    config.i4GridY = _rCfgBlk.i4BlkY + 2;

    if (config.i4GridX > 200 || config.i4GridX <= 2 || config.i4GridY > 200 || config.i4GridY <= 2)
    {
        // assert
        char strErr[512];
        sprintf(strErr, "Line: %d, Abnormal grid X(%d) Y(%d)", __LINE__, config.i4GridX, config.i4GridY);
        LSC_ERR("%s", strErr);
        AEE_ASSERT_LSCTBL(strErr);
        config.i4GridX = 17;
        config.i4GridY = 17;
    }
    config.rCfgBlk = ILscTable::ConfigBlk(config.i4ImgWd, config.i4ImgHt, config.i4GridX, config.i4GridY);
    return config;
}

ILscTable::RsvdData::
RsvdData()
    : u4HwRto(32)
{}

ILscTable::RsvdData::
RsvdData(MUINT32 u4Rto)
    : u4HwRto(u4Rto)
{}

static MINT32
ConvertToHwTbl(float *p_pgn_float, unsigned int *p_lsc_tbl, int grid_x, int grid_y, int RawImgW, int RawImgH)
{
    float* afWorkingBuf = new (std::nothrow) float[BUFFERSIZE];
    if (!afWorkingBuf)
    {
        LSC_ERR("Allocate afWorkingBuf Fail");
        return -1;
    }

    MUINT32 u4RetLSCHwTbl =
        LscGaintoHWTbl(p_pgn_float,
                       p_lsc_tbl,
                       grid_x,
                       grid_y,
                       RawImgW,
                       RawImgH,
                       (void*)afWorkingBuf,
                       BUFFERSIZE);

    delete [] afWorkingBuf;

    return u4RetLSCHwTbl;
}

/*******************************************************************************
 * LscRatio
 *******************************************************************************/
class LscRatio
{
public:
    LscRatio();
    virtual ~LscRatio();

    virtual MBOOL updateCfg(const ILscTable::Config& rCfg);
    virtual MBOOL genHwTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest);
    virtual MBOOL genGainTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest);

private:
    // ra
    MTKLscUtil*                     m_pLscUtilInterface;
    LSC_PARAM_T                     m_rRaLscConfig;
    LSC_RA_STRUCT                   m_rRaConfig;
};

LscRatio::
LscRatio()
    : m_pLscUtilInterface(NULL)
{
    m_pLscUtilInterface         = MTKLscUtil::createInstance();

    m_rRaConfig.working_buf     = (int*)new (std::nothrow) MUINT8[LSC_RA_BUFFER_SIZE];
    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_COEFF; // TSF 4x frame, use coef table
    m_rRaConfig.pix_id          = (int)BAYER_B;
    m_rRaConfig.ra              = 32;
    //m_rRaConfig.lsc_config      = m_rRaLscConfig;

    if (m_pLscUtilInterface)
    {
        ::memset((MUINT8*)m_rRaConfig.working_buf, 0, LSC_RA_BUFFER_SIZE);
        m_pLscUtilInterface->LscRaSwInit((void*)m_rRaConfig.working_buf);
    }
    else
    {
        LSC_ERR("Fail to create MTKLscUtil!");
    }
}

LscRatio::
~LscRatio()
{
    if (m_rRaConfig.working_buf)
    {
        delete [] (MUINT8*) m_rRaConfig.working_buf;
        m_rRaConfig.working_buf = NULL;
    }

    if (m_pLscUtilInterface)
    {
        m_pLscUtilInterface->destroyInstance(m_pLscUtilInterface);
        m_pLscUtilInterface = NULL;
    }
}

MBOOL
LscRatio::
updateCfg(const ILscTable::Config& rCfg)
{
    m_rRaLscConfig.raw_wd        = rCfg.i4ImgWd;
    m_rRaLscConfig.raw_ht        = rCfg.i4ImgHt;
    m_rRaLscConfig.crop_ini_x    = 0;
    m_rRaLscConfig.crop_ini_y    = 0;
    m_rRaLscConfig.block_wd      = rCfg.rCfgBlk.i4BlkW;
    m_rRaLscConfig.block_ht      = rCfg.rCfgBlk.i4BlkH;
    m_rRaLscConfig.x_grid_num    = rCfg.i4GridX;
    m_rRaLscConfig.y_grid_num    = rCfg.i4GridY;
    m_rRaLscConfig.block_wd_last = rCfg.rCfgBlk.i4BlkLastW;
    m_rRaLscConfig.block_ht_last = rCfg.rCfgBlk.i4BlkLastH;

    m_rRaConfig.lsc_config       = m_rRaLscConfig;
    return MTRUE;
}

MBOOL
LscRatio::
genHwTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest)
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT ra_ret;

    if (m_pLscUtilInterface == NULL || m_rRaConfig.working_buf == NULL)
    {
        LSC_ERR("Null Util(%p, %p)", m_pLscUtilInterface, m_rRaConfig.working_buf);
        goto lbExit;
    }

    if (pSrc == NULL || pDest == NULL)
    {
        LSC_ERR("Null pointer(%p, %p)", pSrc, pDest);
        goto lbExit;
    }

    if (ratio > 32)
    {
        LSC_ERR("ratio(%d)", ratio);
        goto lbExit;
    }

    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_COEFF;
    m_rRaConfig.in_tbl          = (int*)pSrc;
    m_rRaConfig.out_tbl         = (int*)pDest;
    m_rRaConfig.ra              = ratio;

    if (ratio == 32)
    {
        MUINT32 u4Size = (m_rRaLscConfig.x_grid_num - 1) * (m_rRaLscConfig.y_grid_num - 1) * 4 * 4 * 6;
        u4Size = MIN(u4Size, 98304);
        ::memcpy(pDest, pSrc, u4Size);
    }
    else
    {
        // do ratio
        ra_ret = m_pLscUtilInterface->LscRaSwMain((void*)&m_rRaConfig);
    }
lbExit:
    return fgRet;
}

MBOOL
LscRatio::
genGainTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest)
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT ra_ret;

    if (m_pLscUtilInterface == NULL || m_rRaConfig.working_buf == NULL)
    {
        LSC_ERR("Null Util(%p, %p)", m_pLscUtilInterface, m_rRaConfig.working_buf);
        goto lbExit;
    }

    if (pSrc == NULL || pDest == NULL)
    {
        LSC_ERR("Null pointer(%p, %p)", pSrc, pDest);
        goto lbExit;
    }

    if (ratio > 32)
    {
        LSC_ERR("ratio(%d)", ratio);
        goto lbExit;
    }

    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_GAIN;
    m_rRaConfig.in_tbl          = (int*)pSrc;
    m_rRaConfig.out_tbl         = (int*)pDest;
    m_rRaConfig.ra              = ratio;
    // do ratio
    ra_ret = m_pLscUtilInterface->LscRaSwMain((void*)&m_rRaConfig);

lbExit:
    return fgRet;
}

/*******************************************************************************
 * ILscTbl::LscTblImp
 *******************************************************************************/
class ILscTbl::LscTblImp
{
public:
                                    LscTblImp(ILscTable::TBL_TYPE_T eType);
                                    LscTblImp(LscTblImp const& other);
    LscTblImp&                      operator=(LscTblImp const& other);

                                    LscTblImp(ILscTable::TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
    virtual                         ~LscTblImp();
    virtual ILscTable::TBL_TYPE_T   getType() const {return m_eType;}
    virtual ILscTable::TBL_BAYER_T  getBayer() const {return m_eBayer;}
    virtual const MVOID*            getData() const;
    virtual MUINT32                 getSize() const;
    virtual const ILscTable::Config&  getConfig() const {return m_rCfg;}
    virtual const ILscTable::RsvdData&    getRsvdData() const {return m_rRsvdData;}
    virtual MVOID*                  editData();
    virtual MVOID*                  editData(int channel);
    virtual ILscTable::RsvdData&      editRsvdData() {return m_rRsvdData;}
    virtual MBOOL                   setData(const MVOID* src, MUINT32 size);
    virtual MBOOL                   setConfig(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
    virtual MBOOL                   setBayer(ILscTable::TBL_BAYER_T bayer);
    virtual MBOOL                   setRsvdData(const ILscTable::RsvdData & rData) {m_rRsvdData = rData; return MTRUE;}
    virtual MBOOL                   reorder(ILscTable::TBL_BAYER_T bayer);
    virtual MBOOL                   dump(const char* filename) const;

    virtual MBOOL                   cropOut(const TransformCfg_T& trfm, ILscTable& output) const;
    virtual MBOOL                   cropToGain(const TransformCfg_T& trfm, ILscTable& output, vector<MUINT16*> &vecGain, MUINT32 mode) const;
    virtual MBOOL                   mergeAndCrop(ILscTable& baseTbl, ILscTable& output);
    virtual MBOOL                   convert(ILscTable& output) const;
    virtual MBOOL                   getRaTbl(MUINT32 u4Ratio, ILscTable& output) const;
    virtual MBOOL                   toBuf(ILscBuf& buf) const;

protected:
    ILscTable::TBL_TYPE_T           m_eType;
    ILscTable::TBL_BAYER_T          m_eBayer;
    ILscTable::Config               m_rCfg;
    ILscTable::RsvdData             m_rRsvdData;
    MUINT32                         m_u4Size;
    vector<MUINT8>                  m_vecData;
    vector<MUINT8>                  m_vecDataR;
    vector<MUINT8>                  m_vecDataG1;
    vector<MUINT8>                  m_vecDataG2;
    vector<MUINT8>                  m_vecDataB;
};

ILscTbl::LscTblImp::
LscTblImp(ILscTable::TBL_TYPE_T eType)
    : m_eType(eType)
    , m_eBayer(ILscTable::BAYER_B)
    , m_rCfg()
    , m_rRsvdData()
    , m_u4Size(0)
{
}

ILscTbl::LscTblImp::
LscTblImp(ILscTable::TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
    : m_eType(eType)
    , m_eBayer(ILscTable::BAYER_B)
    , m_rCfg()
    , m_rRsvdData()
    , m_u4Size(0)
{
    m_vecData.clear();
    m_vecDataR.clear();
    m_vecDataG1.clear();
    m_vecDataG2.clear();
    m_vecDataB.clear();
    if (!setConfig(i4W, i4H, i4GridX, i4GridY))
    {
        LSC_ERR("Force grid as 17x17 due error config");
    }
}

ILscTbl::LscTblImp::
LscTblImp(ILscTbl::LscTblImp const& other)
    : m_eType(other.m_eType)
    , m_eBayer(other.m_eBayer)
    , m_rCfg(other.m_rCfg)
    , m_rRsvdData(other.m_rRsvdData)
    , m_u4Size(other.m_u4Size)
    , m_vecData(other.m_vecData)
    , m_vecDataR(other.m_vecDataR)
    , m_vecDataG1(other.m_vecDataG1)
    , m_vecDataG2(other.m_vecDataG2)
    , m_vecDataB(other.m_vecDataB)
{
}

ILscTbl::LscTblImp&
ILscTbl::LscTblImp::
operator=(ILscTbl::LscTblImp const& other)
{
    if (this != &other)
    {
        m_eType = other.m_eType;
        m_eBayer = other.m_eBayer;
        m_rCfg = other.m_rCfg;
        m_rRsvdData = other.m_rRsvdData;
        m_u4Size = other.m_u4Size;
        m_vecData = other.m_vecData;
        m_vecDataR = other.m_vecDataR;
        m_vecDataG1= other.m_vecDataG1;
        m_vecDataG2= other.m_vecDataG2;
        m_vecDataB = other.m_vecDataB;
    }
    else
    {
        LSC_LOG("this(%p) == other(%p)", this, &other);
    }

    return *this;
}

ILscTbl::LscTblImp::
~LscTblImp()
{
}

MBOOL
ILscTbl::LscTblImp::
setConfig(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
{
    MBOOL fgRet = MTRUE;

    if (i4GridX > 200 || i4GridX <= 2 || i4GridY > 200 || i4GridY <= 2)
    {
        // assert
        char strErr[512];
        sprintf(strErr, "Line: %d, Abnormal grid X(%d) Y(%d)", __LINE__, i4GridX, i4GridY);
        LSC_ERR("%s", strErr);
        AEE_ASSERT_LSCTBL(strErr);
        i4GridX = 17;
        i4GridY = 17;
        fgRet = MFALSE;
    }

    // set config
    m_rCfg = makeConfig(i4W, i4H, i4GridX, i4GridY);

    MUINT32 u4Size = 0;
    switch (m_eType)
    {
    default:
    case ILscTable::HWTBL:
        u4Size = (m_rCfg.rCfgBlk.i4BlkX+1) * (m_rCfg.rCfgBlk.i4BlkY+1) * 4 * 4 * 6;
        break;
    case ILscTable::GAIN_FIXED:
        u4Size = i4GridX * i4GridY * 4 * 2;
        m_vecDataR.resize(u4Size);
        m_vecDataG1.resize(u4Size);
        m_vecDataG2.resize(u4Size);
        m_vecDataB.resize(u4Size);
        break;
    case ILscTable::GAIN_FLOAT:
        u4Size = i4GridX * i4GridY * 4 * sizeof(MFLOAT);
        break;
    }

    if (m_u4Size < u4Size)
    {
        m_u4Size = u4Size;
        m_vecData.resize(u4Size);
    }

    return fgRet;
}

MBOOL
ILscTbl::LscTblImp::
setBayer(ILscTable::TBL_BAYER_T bayer)
{
    m_eBayer = bayer;
    return MTRUE;
}

MBOOL
ILscTbl::LscTblImp::
reorder(ILscTable::TBL_BAYER_T bayer)
{
    if (m_eType == ILscTable::GAIN_FIXED)
    {
    #ifdef _SWAP_
    #undef _SWAP_
    #endif
    #define _SWAP_(a, b) (a) ^= (b); (b) ^= (a); (a) ^= (b);
        if (m_eBayer != bayer)
        {
            SWAP_PAIR_T swp = _swap[m_eBayer][bayer];
            MINT32 i4Count = m_u4Size / sizeof(MUINT16) / 4;
            MUINT16* pDest = static_cast<MUINT16*>(editData());

            while (i4Count--)
            {
                _SWAP_(pDest[swp.a0], pDest[swp.b0]);
                _SWAP_(pDest[swp.a1], pDest[swp.b1]);
                pDest += 4;
            }
        }
        return MTRUE;
    }
    else
    {
        // HWTBL
        return MFALSE;
    }
}

MBOOL
ILscTbl::LscTblImp::
setData(const MVOID* src, MUINT32 size)
{
    if (m_u4Size < size)
    {
        LSC_ERR("input data size(%d) is larger than this size(%d), please call setConfig to resize", size, m_u4Size);
        return MFALSE;
    }

    MBOOL fgRet = MTRUE;
    MUINT8* dest = m_vecData.data();
    if (dest && src)
    {
        ::memcpy(dest, src, size);
    }
    else
    {
        LSC_ERR("NULL pointer dest(%p), src(%p)", dest, src);
        fgRet = MFALSE;
    }

    return fgRet;
}

const MVOID*
ILscTbl::LscTblImp::
getData() const
{
    return m_vecData.data();
}

MVOID*
ILscTbl::LscTblImp::
editData()
{
    return m_vecData.data();
}

MVOID*
ILscTbl::LscTblImp::
editData(int channel)
{
    switch(channel)
    {
        case 0:
            return m_vecDataR.data();
        case 1:
            return m_vecDataG1.data();
        case 2:
            return m_vecDataG2.data();
        case 3:
            return m_vecDataB.data();
        default:

            return NULL;
    }

    return NULL;
}

MUINT32
ILscTbl::LscTblImp::
getSize() const
{
    return m_u4Size;
}

MBOOL
ILscTbl::LscTblImp::
dump(const char* filename) const
{
    MINT32 i = m_u4Size / 4;
    FILE* fpdebug;
    fpdebug = fopen(filename, "w");
    if ( fpdebug == NULL )
    {
        LSC_LOG("Can't open: %s", filename);
        return MFALSE;
    }

    const MUINT32* pData = static_cast<const MUINT32*>(getData());
    while (i >= 6)
    {
        MUINT32 a = *pData++;
        MUINT32 b = *pData++;
        MUINT32 c = *pData++;
        MUINT32 d = *pData++;
        MUINT32 e = *pData++;
        MUINT32 f = *pData++;
        fprintf(fpdebug, "0x%08x    0x%08x    0x%08x    0x%08x    0x%08x    0x%08x\n", a, b, c, d, e, f);
        i -= 6;
    }

    while (i)
    {
        fprintf(fpdebug, "0x%08x    ", *pData++);
        i --;
    }

    fclose(fpdebug);
    return MTRUE;
}

MBOOL
ILscTbl::LscTblImp::
cropOut(const TransformCfg_T& trfm, ILscTable& output) const
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT result = S_LSC_CONVERT_OK;
    SHADIND_TRFM_CONF rTrfm;
    MUINT8* gWorkinBuffer = new (std::nothrow) MUINT8[SHADIND_FUNC_WORKING_BUFFER_SIZE];
    int k=0;
    if (!gWorkinBuffer)
    {
        LSC_ERR("Fail to allocate gWorkinBuffer");
        fgRet = MFALSE;
    }
    else
    {
        ::memset(gWorkinBuffer, 0, SHADIND_FUNC_WORKING_BUFFER_SIZE*sizeof(MUINT8));
        //LSC_LOG("gWorkinBuffer(%p)", gWorkinBuffer);
    }

    vector<MUINT8> tmpInput = m_vecData;
    if (!output.setConfig(trfm.u4W, trfm.u4H, trfm.u4GridX, trfm.u4GridY))
    {
        LSC_ERR("resize(%d,%d), grid(%d,%d), crop(%d,%d,%d,%d)",
            trfm.u4ResizeW, trfm.u4ResizeH, trfm.u4GridX, trfm.u4GridY, trfm.u4X, trfm.u4Y, trfm.u4W, trfm.u4H);
        goto lbExit;
    }
    output.setBayer(m_eBayer);

    rTrfm.working_buff_addr     = gWorkinBuffer;
    rTrfm.working_buff_size     = SHADIND_FUNC_WORKING_BUFFER_SIZE;
    rTrfm.afn                   = SHADING_AFN_R0D;

    rTrfm.input.img_width       = m_rCfg.i4ImgWd;
    rTrfm.input.img_height      = m_rCfg.i4ImgHt;
    rTrfm.input.offset_x        = 0;
    rTrfm.input.offset_y        = 0;
    rTrfm.input.crop_width      = m_rCfg.i4ImgWd;
    rTrfm.input.crop_height     = m_rCfg.i4ImgHt;
    rTrfm.input.bayer           = (BAYER_ID_T)m_eBayer;
    rTrfm.input.grid_x          = m_rCfg.i4GridX;           // Input gain
    rTrfm.input.grid_y          = m_rCfg.i4GridY;           // Input gain
    rTrfm.input.lwidth          = 0;
    rTrfm.input.lheight         = 0;
    rTrfm.input.ratio_idx       = 0;
    rTrfm.input.grgb_same       = SHADING_GRGB_SAME_NO;
    rTrfm.input.table           = (MUINT32*)tmpInput.data(); // Input gain

    switch (m_eType)
    {
    case ILscTable::HWTBL:
        rTrfm.input.data_type  = SHADING_TYPE_COEFF;       // coef
        break;
    default:
    case ILscTable::GAIN_FIXED:
        rTrfm.input.data_type  = SHADING_TYPE_GAIN;        // gain
        break;
    }

    rTrfm.output.img_width      = trfm.u4ResizeW;           // output width, resize from input width
    rTrfm.output.img_height     = trfm.u4ResizeH;           // output height, resize from input height
    rTrfm.output.offset_x       = trfm.u4X;                 // crop
    rTrfm.output.offset_y       = trfm.u4Y;                 // crop
    rTrfm.output.crop_width     = trfm.u4W;                 // crop
    rTrfm.output.crop_height    = trfm.u4H;                 // crop
    rTrfm.output.bayer          = (BAYER_ID_T)output.getBayer();
    rTrfm.output.grid_x         = trfm.u4GridX;             // output gain (alwasy 16x16)
    rTrfm.output.grid_y         = trfm.u4GridY;             // output gain
    rTrfm.output.lwidth         = 0;
    rTrfm.output.lheight        = 0;
    rTrfm.output.ratio_idx      = 0;
    rTrfm.output.grgb_same      = SHADING_GRGB_SAME_YES;
    rTrfm.output.table          = (MUINT32*)output.editData(); // output gain

    switch (output.getType())
    {
    case ILscTable::HWTBL:
        rTrfm.output.data_type  = SHADING_TYPE_COEFF;       // coef
        break;
    default:
    case ILscTable::GAIN_FIXED:
        rTrfm.output.data_type  = SHADING_TYPE_GAIN;        // gain
        break;
    }
    //_LogShadingSpec("input",  rTrfm.input);
    //_LogShadingSpec("output", rTrfm.output);

    if (ILscTbl::HWTBL == m_eType &&
        ILscTbl::GAIN_FIXED == output.getType() &&
        (rTrfm.output.grid_x > 33 || rTrfm.output.grid_y > 33 ))
    {
        // only for hwtbl to gain and support up to 200x200 gain table
        g_algo_mutex.lock();
        result = shading_transform_new(rTrfm);
        g_algo_mutex.unlock();
        LSC_ERR(" end of shading_transform_new");
    }
    else
    {
        g_algo_mutex.lock();
        result = shading_transform(rTrfm);
        g_algo_mutex.unlock();
/*
        if(ILscTbl::GAIN_FIXED == output.getType())
        {
            const MUINT16* rLSCGain = static_cast<const MUINT16*>(output.editData());
            for(k=0;k<10;k++) // Gain table defined in lsc_mgr always starts with B pixel.
            {
                LSC_ERR("tbl1 k= %d, (%d, %d, %d)", k, rLSCGain[4*k+3], rLSCGain[4*k+2], rLSCGain[4*k]);
            }
        }
*/
    }

    if (S_LSC_CONVERT_OK != result)
    {
        LSC_ERR("Transform Error(%d)", result);
        fgRet = MFALSE;
    }
    /*else
    {
        LSC_LOG("Transform done.");
    }*/

lbExit:
    delete [] gWorkinBuffer;
    return fgRet;
}

MBOOL
ILscTbl::LscTblImp::
cropToGain(const TransformCfg_T& trfm, ILscTable& output, vector<MUINT16*> &vecGain, MUINT32 mode) const
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT result = S_LSC_CONVERT_OK;
    SHADIND_TRFM_CONF_PSO rTrfm;
    MUINT8* gWorkinBuffer = new (std::nothrow) MUINT8[SHADIND_FUNC_WORKING_BUFFER_SIZE];

    int k=0;

    if (!gWorkinBuffer)
    {
        LSC_ERR("Fail to allocate gWorkinBuffer");
        fgRet = MFALSE;
        goto lbExit;
    }
    else
    {
        ::memset(gWorkinBuffer, 0, SHADIND_FUNC_WORKING_BUFFER_SIZE*sizeof(MUINT8));
        LSC_LOG("gWorkinBuffer(%p)", gWorkinBuffer);
    }

    if (!output.setConfig(trfm.u4W, trfm.u4H, trfm.u4GridX, trfm.u4GridY))
    {
        LSC_ERR("resize(%d,%d), grid(%d,%d), crop(%d,%d,%d,%d)",
            trfm.u4ResizeW, trfm.u4ResizeH, trfm.u4GridX, trfm.u4GridY, trfm.u4X, trfm.u4Y, trfm.u4W, trfm.u4H);
        goto lbExit;
    }

    if(m_vecData.size()==0)
        goto lbExit;

    output.setBayer(m_eBayer);

    rTrfm.working_buff_addr     = gWorkinBuffer;
    rTrfm.working_buff_size     = SHADIND_FUNC_WORKING_BUFFER_SIZE;
    rTrfm.afn                   = SHADING_AFN_R0D;

    rTrfm.input.img_width       = m_rCfg.i4ImgWd;
    rTrfm.input.img_height      = m_rCfg.i4ImgHt;
    rTrfm.input.offset_x        = 0;
    rTrfm.input.offset_y        = 0;
    rTrfm.input.crop_width      = m_rCfg.i4ImgWd;
    rTrfm.input.crop_height     = m_rCfg.i4ImgHt;
    rTrfm.input.bayer           = (BAYER_ID_T)m_eBayer;
    rTrfm.input.grid_x          = m_rCfg.i4GridX;           // Input gain
    rTrfm.input.grid_y          = m_rCfg.i4GridY;           // Input gain
    rTrfm.input.lwidth          = 0;
    rTrfm.input.lheight         = 0;
    rTrfm.input.ratio_idx       = 0;
    rTrfm.input.grgb_same       = SHADING_GRGB_SAME_NO;
    rTrfm.input.table           = (MUINT32*)m_vecData.data(); // Input gain

    switch (m_eType)
    {
    case ILscTable::HWTBL:
        rTrfm.input.data_type  = SHADING_TYPE_COEFF;       // coef
        break;
    default:
    case ILscTable::GAIN_FIXED:
        rTrfm.input.data_type  = SHADING_TYPE_GAIN;        // gain
        break;
    }

    rTrfm.output.img_width      = trfm.u4ResizeW;           // output width, resize from input width
    rTrfm.output.img_height     = trfm.u4ResizeH;           // output height, resize from input height
    rTrfm.output.offset_x       = trfm.u4X;                 // crop
    rTrfm.output.offset_y       = trfm.u4Y;                 // crop
    rTrfm.output.crop_width     = trfm.u4W;                 // crop
    rTrfm.output.crop_height    = trfm.u4H;                 // crop
    rTrfm.output.bayer          = (BAYER_ID_T)output.getBayer();
    rTrfm.output.grid_x         = trfm.u4GridX;             // output gain (alwasy 16x16)
    rTrfm.output.grid_y         = trfm.u4GridY;             // output gain
    rTrfm.output.lwidth         = 0;
    rTrfm.output.lheight        = 0;
    rTrfm.output.ratio_idx      = 0;
    rTrfm.output.grgb_same      = SHADING_GRGB_SAME_YES;

    rTrfm.output.tableR         = (MUINT16*)vecGain.at(0);
    rTrfm.output.tableGr        = (vecGain.size()==4)?(MUINT16*)vecGain.at(3):(MUINT16*)output.editData();
    rTrfm.output.tableGb        = (MUINT16*)vecGain.at(1);
    rTrfm.output.tableB         = (MUINT16*)vecGain.at(2);

    switch (output.getType())
    {
    case ILscTable::HWTBL:
        rTrfm.output.data_type  = SHADING_TYPE_COEFF;       // coef
        break;
    default:
    case ILscTable::GAIN_FIXED:
        rTrfm.output.data_type  = SHADING_TYPE_GAIN;        // gain
        break;
    }
    //_LogShadingSpec("input",  rTrfm.input);
    //_LogShadingSpec("output", rTrfm.output);

    g_algo_mutex.lock();

    if(mode==0)
        result = shading_transform_new(rTrfm);
    else
        result = shading_transform_new(rTrfm, SHADING_GAIN_TYPE_MODE1);

    g_algo_mutex.unlock();
/*
    if(ILscTbl::GAIN_FIXED == output.getType())
    {
        const MUINT16* rLSCGainR  = static_cast<const MUINT16*>(output.editData(0));
        const MUINT16* rLSCGainGr = static_cast<const MUINT16*>(output.editData(1));
        const MUINT16* rLSCGainB  = static_cast<const MUINT16*>(output.editData(3));
        for(k=0;k<10;k++) // Gain table defined in lsc_mgr always starts with B pixel.
        {
            LSC_ERR("tbl2 k= %d, (%d, %d, %d)", k, vecGain.at(0)[k], vecGain.at(1)[k], vecGain.at(2)[k]);
        }
    }
*/
    if (S_LSC_CONVERT_OK != result)
    {
        LSC_ERR("Transform Error(%d)", result);
        fgRet = MFALSE;
    }
    else
    {
        LSC_LOG("Transform done.");
    }

lbExit:
    if(gWorkinBuffer!=NULL)
        delete [] gWorkinBuffer;
    return fgRet;
}

MBOOL
ILscTbl::LscTblImp::
mergeAndCrop(ILscTable& baseTbl, ILscTable& output)
{
    LSC_PARAM_T LscConfig = {};
    LSC_RESULT ret = S_LSC_CONVERT_OK;
    MBOOL fgRet = MTRUE;

    int *hwtbl_use_G = (MINT32*)baseTbl.editData();
    int *hwtbl_use_RGBG = (MINT32*)m_vecData.data();

    // working buffer for RA function
    int *working_buf_transform = (int*)malloc(SHADIND_FUNC_WORKING_BUFFER_SIZE);
    int *working_buf_merge = (int*)malloc(33* 33 * 12 * 4 * 3 * sizeof(int));

    if (!output.setConfig(baseTbl.getConfig()))
    {
        LSC_ERR("(i4ImgWd, i4ImgHt)=(%d,%d)",
            baseTbl.getConfig().i4ImgWd, baseTbl.getConfig().i4ImgHt,
            baseTbl.getConfig().i4GridX, baseTbl.getConfig().i4GridY);
        fgRet = MFALSE;
        goto lbExit;
    }

    if(m_vecData.size()==0 || working_buf_transform==NULL || working_buf_merge==NULL)
    {
        LSC_ERR("m_vecData.size %d, working_buf_transform(%p), working_buf_merge(%p)",
            m_vecData.size(), working_buf_transform, working_buf_merge);
        fgRet = MFALSE;
        goto lbExit;
    }

    output.setBayer(m_eBayer);

    LscConfig.raw_wd = baseTbl.getConfig().i4ImgWd;
    LscConfig.raw_ht = baseTbl.getConfig().i4ImgHt;
    LscConfig.block_wd = baseTbl.getConfig().rCfgBlk.i4BlkW;    // half size
    LscConfig.block_ht = baseTbl.getConfig().rCfgBlk.i4BlkH;
    LscConfig.x_grid_num = baseTbl.getConfig().i4GridX;
    LscConfig.y_grid_num = baseTbl.getConfig().i4GridY;
    LscConfig.block_wd_last = baseTbl.getConfig().rCfgBlk.i4BlkLastW;
    LscConfig.block_ht_last = baseTbl.getConfig().rCfgBlk.i4BlkLastH;
    LscConfig.crop_ini_x = 0;
    LscConfig.crop_ini_y = 0;
    LscConfig.crop_end_x = baseTbl.getConfig().i4ImgWd;
    LscConfig.crop_end_y = baseTbl.getConfig().i4ImgHt;

    LSC_LOG("lscConfig raw (%d, %d), block (%d, %d), block_last (%d, %d), grid_num = (%d, %d)",
            baseTbl.getConfig().i4ImgWd, baseTbl.getConfig().i4ImgHt,
            baseTbl.getConfig().rCfgBlk.i4BlkW, baseTbl.getConfig().rCfgBlk.i4BlkH,
            baseTbl.getConfig().rCfgBlk.i4BlkLastW, baseTbl.getConfig().rCfgBlk.i4BlkLastH,
            baseTbl.getConfig().i4GridX, baseTbl.getConfig().i4GridY);

    ret = shading_merge_tbl(working_buf_transform, working_buf_merge, hwtbl_use_RGBG, hwtbl_use_G, (MINT32*)output.editData(), LscConfig);

    LSC_LOG("coef table out %d, %d, output Config (%d, %d, %d, %d), bayer %d",
        ((MINT32*)output.editData())[0], ((MINT32*)output.editData())[1],
        output.getConfig().i4ImgWd, output.getConfig().i4ImgHt,
        output.getConfig().i4GridX, output.getConfig().i4GridY, m_eBayer);

lbExit:
    if(working_buf_transform!=NULL)
        free(working_buf_transform);

    if(working_buf_merge!=NULL)
        free(working_buf_merge);

    if (S_LSC_CONVERT_OK != ret)
    {
        LSC_ERR("Transform Error(%d)", ret);
        fgRet = MFALSE;
    }
    else
    {
        fgRet = MTRUE;
    }

    return fgRet;
}

MBOOL
ILscTbl::LscTblImp::
convert(ILscTable& output) const
{
    if (ILscTable::GAIN_FLOAT != m_eType && ILscTable::GAIN_FLOAT != output.getType())
    {
        return cropOut(TransformCfg_T(m_rCfg.i4ImgWd, m_rCfg.i4ImgHt, m_rCfg.i4GridX, m_rCfg.i4GridY, 0, 0, m_rCfg.i4ImgWd, m_rCfg.i4ImgHt), output);
    }
    else
    {
        if (ILscTable::GAIN_FLOAT == output.getType())
        {
            switch (m_eType)
            {
            case ILscTable::HWTBL:
                {
                    // HWTBL -> FLOAT
                    ILscTbl tmp(ILscTable::GAIN_FIXED);
                    if (cropOut(TransformCfg_T(m_rCfg.i4ImgWd, m_rCfg.i4ImgHt, m_rCfg.i4GridX, m_rCfg.i4GridY, 0, 0, m_rCfg.i4ImgWd, m_rCfg.i4ImgHt), tmp))
                    {
                        return tmp.convert(output);
                    }
                }
                return MFALSE;
            case ILscTable::GAIN_FIXED:
                {
                    // FIXED -> FLOAT
                    output.setConfig(m_rCfg);
                    MINT32 i4Count = m_u4Size / sizeof(MUINT16);
                    const MUINT16* pSrc = static_cast<const MUINT16*>(getData());
                    MFLOAT* pDest = static_cast<MFLOAT*>(output.editData());
                    while (i4Count--)
                    {
                        // ALPS03271644 : Fix CTS fail
                        // Ensure gain table values are always larger than or equal to 1
                        MFLOAT temp = (MFLOAT) *pSrc++ / 8192.0f;
                        *pDest++ = temp < 1.0 ? 1.0 : temp;
                    }
                }
                return MTRUE;
            default: // impossible case
            case ILscTable::GAIN_FLOAT:
                CAM_LOGE("impossible case");
                return MFALSE;
            }
        }
        else // (ILscTable::GAIN_FLOAT == m_eType)
        {
            CAM_LOGE("impossible case");
            return MFALSE;
        }
    }
}

MBOOL
ILscTbl::LscTblImp::
getRaTbl(MUINT32 u4Ratio, ILscTable& output) const
{
    MBOOL fgRet = MFALSE;

    if (u4Ratio > 32)
    {
        LSC_ERR("ratio(%d)", u4Ratio);
        return MFALSE;
    }

    switch (m_eType)
    {
    case ILscTable::HWTBL: // input type
        {
            TBL_TYPE_T eType = output.getType();
            LscRatio rLscRatio;
            rLscRatio.updateCfg(m_rCfg);
            if (ILscTable::GAIN_FLOAT != eType)
            {
                output.setBayer(m_eBayer);
                output.setConfig(m_rCfg);
                if (ILscTable::HWTBL == eType)
                {
                    fgRet = rLscRatio.genHwTbl(u4Ratio, static_cast<const MUINT32*>(getData()), static_cast<MUINT32*>(output.editData()));
                }
                else
                {
                    fgRet = rLscRatio.genGainTbl(u4Ratio, static_cast<const MUINT32*>(getData()), static_cast<MUINT32*>(output.editData()));
                }
            }
            else
            {
                // output = ILscTable::GAIN_FLOAT
                ILscTbl rTmp(ILscTable::GAIN_FIXED);
                rTmp.setBayer(m_eBayer);
                rTmp.setConfig(m_rCfg);
                fgRet = rLscRatio.genGainTbl(u4Ratio, static_cast<const MUINT32*>(getData()), static_cast<MUINT32*>(rTmp.editData()));
                if (fgRet)
                {
                    fgRet = rTmp.convert(output);
                }
                else
                {
                    LSC_ERR("Fail to generate gain table with ratio(%d)", u4Ratio);
                }
            }
        }
        break;
    default:
        LSC_ERR("Not support");
        break;
    }

    return fgRet;
}

MBOOL
ILscTbl::LscTblImp::
toBuf(ILscBuf& buf) const
{
    if (ILscTable::HWTBL == m_eType)
    {
        ILscBuf::Config rBufCfg;
        rBufCfg.i4ImgWd = m_rCfg.i4ImgWd;
        rBufCfg.i4ImgHt = m_rCfg.i4ImgHt;
        rBufCfg.i4BlkX  = m_rCfg.rCfgBlk.i4BlkX;
        rBufCfg.i4BlkY  = m_rCfg.rCfgBlk.i4BlkY;
        rBufCfg.i4BlkW  = m_rCfg.rCfgBlk.i4BlkW;
        rBufCfg.i4BlkH  = m_rCfg.rCfgBlk.i4BlkH;
        rBufCfg.i4BlkLastW = m_rCfg.rCfgBlk.i4BlkLastW;
        rBufCfg.i4BlkLastH = m_rCfg.rCfgBlk.i4BlkLastH;
        if (buf.setConfig(rBufCfg))
        {
            MBOOL fgResult = buf.setTable(getData(), getSize());
            if (!fgResult)
            {
                LSC_ERR("Fail to set table");
            }
            return fgResult;
        }
        else
        {
            LSC_ERR("Fail to set config");
            return MFALSE;
        }
    }
    else
    {
        ILscTbl tmp(ILscTable::HWTBL);
        if (convert(tmp))
        {
            return tmp.toBuf(buf);
        }
        else
        {
            LSC_ERR("Fail to convert");
            return MFALSE;
        }
    }
    return MFALSE;
}
/*******************************************************************************
 * LscTblFloatImp
 *******************************************************************************/
class LscTblFloatImp : public ILscTbl::LscTblImp
{
public:
                                    LscTblFloatImp();
                                    LscTblFloatImp(ILscTbl::LscTblImp const& other);
                                    LscTblFloatImp(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
    virtual MBOOL                   reorder(ILscTable::TBL_BAYER_T bayer);

    virtual MBOOL                   dump(const char* filename) const;

    virtual MBOOL                   cropOut(const ILscTable::TransformCfg_T& trfm, ILscTable& output) const;
    virtual MBOOL                   cropToGain(const ILscTable::TransformCfg_T& trfm, ILscTable& output, vector<MUINT16*> &vecGain, MUINT32 mode) const;
    virtual MBOOL                   convert(ILscTable& output) const;
};

LscTblFloatImp::
LscTblFloatImp()
    : ILscTbl::LscTblImp(ILscTable::GAIN_FLOAT)
{}

LscTblFloatImp::
LscTblFloatImp(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
    : ILscTbl::LscTblImp(ILscTable::GAIN_FLOAT, i4W, i4H, i4GridX, i4GridY)
{}

LscTblFloatImp::
LscTblFloatImp(ILscTbl::LscTblImp const& other)
    : ILscTbl::LscTblImp(other)
{}

MBOOL
LscTblFloatImp::
reorder(ILscTable::TBL_BAYER_T bayer)
{
#ifdef _SWAP_
#undef _SWAP_
#endif
#define _SWAP_(a, b) {MFLOAT c = (a); (a) = (b); (b) = (c);}

    if (m_eBayer != bayer)
    {
        SWAP_PAIR_T swp = _swap[m_eBayer][bayer];
        MINT32 i4Count = m_u4Size / sizeof(MFLOAT) / 4;
        MFLOAT* pDest = static_cast<MFLOAT*>(editData());

        while (i4Count--)
        {
            _SWAP_(pDest[swp.a0], pDest[swp.b0]);
            _SWAP_(pDest[swp.a1], pDest[swp.b1]);
            pDest += 4;
        }
    }
    return MTRUE;
}

MBOOL
LscTblFloatImp::
dump(const char* filename) const
{
    MINT32 i = m_u4Size / sizeof(MFLOAT);
    FILE* fpdebug;
    fpdebug = fopen(filename, "w");
    if ( fpdebug == NULL )
    {
        LSC_LOG("Can't open: %s", filename);
        return MFALSE;
    }

    const MFLOAT* pData = static_cast<const MFLOAT*>(getData());
    while (i >= 4)
    {
        MFLOAT a = *pData++;
        MFLOAT b = *pData++;
        MFLOAT c = *pData++;
        MFLOAT d = *pData++;
        fprintf(fpdebug, "%3.6f    %3.6f    %3.6f    %3.6f\n", a, b, c, d);
        i -= 4;
    }

    while (i)
    {
        fprintf(fpdebug, "%3.6f    ", *pData++);
        i --;
    }

    fclose(fpdebug);
    return MTRUE;
}

MBOOL
LscTblFloatImp::
convert(ILscTable& output) const
{
    output.setConfig(m_rCfg);
    switch (output.getType())
    {
    case ILscTable::HWTBL:
        {
            LSC_LOG("F2H");
            if (S_LSC_CONVERT_OK !=
                    ConvertToHwTbl(
                        (float*)m_vecData.data(),
                        (unsigned int*)output.editData(),
                        m_rCfg.i4GridX,
                        m_rCfg.i4GridY,
                        m_rCfg.i4ImgWd,
                        m_rCfg.i4ImgHt))
            {
                return MFALSE;
            }
            return MTRUE;
        }
        break;
    case ILscTable::GAIN_FIXED:
        {
            LSC_LOG("float to fixed");
            MINT32 i4Count = m_u4Size / sizeof(MFLOAT);
            const MFLOAT* pSrc = static_cast<const MFLOAT*>(getData());
            MUINT16* pDest = static_cast<MUINT16*>(output.editData());
            while (i4Count--)
            {
                MFLOAT fVal = (*pSrc++);
                *pDest++ = MIN(65535, (fVal * 8192.0f + 0.5f));
            }
        }
        return MTRUE;
    default:
    case ILscTable::GAIN_FLOAT:
        output.setData(getData(), m_u4Size);
        return MTRUE;
    }

    return MTRUE;
}

MBOOL
LscTblFloatImp::
cropOut(const ILscTable::TransformCfg_T& /*trfm*/, ILscTable& /*output*/) const
{
    return MFALSE;
}

MBOOL
LscTblFloatImp::
cropToGain(const ILscTable::TransformCfg_T& trfm, ILscTable& output, vector<MUINT16*> &vecGain, MUINT32 mode) const
{
    return MFALSE;
}

/*******************************************************************************
 * ILscTbl
 *******************************************************************************/
ILscTbl::
ILscTbl()
    : m_pImp(new (std::nothrow) ILscTbl::LscTblImp(ILscTable::HWTBL))
{
}

ILscTbl::
ILscTbl(ILscTable::TBL_TYPE_T eType)
{
    if (ILscTable::GAIN_FLOAT == eType)
    {
        m_pImp = new (std::nothrow) LscTblFloatImp();
    }
    else
    {
        m_pImp = new (std::nothrow) ILscTbl::LscTblImp(eType);
    }
}

ILscTbl::
ILscTbl(ILscTable::TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
{
    if (ILscTable::GAIN_FLOAT == eType)
    {
        m_pImp = new (std::nothrow) LscTblFloatImp(i4W, i4H, i4GridX, i4GridY);
    }
    else
    {
        m_pImp = new (std::nothrow) ILscTbl::LscTblImp(eType, i4W, i4H, i4GridX, i4GridY);
    }
}

ILscTbl::
ILscTbl(ILscTbl const& other)
{
    if (ILscTable::GAIN_FLOAT == other.getType())
    {
        m_pImp = new (std::nothrow) LscTblFloatImp(*(other.m_pImp));
    }
    else
    {
        m_pImp = new (std::nothrow) ILscTbl::LscTblImp(*(other.m_pImp));
    }
}

ILscTbl&
ILscTbl::
operator=(ILscTbl const& other)
{
    if (this != &other) {

        if(m_pImp)
            delete m_pImp;
        if (ILscTable::GAIN_FLOAT == other.getType())
        {
            m_pImp = new (std::nothrow) LscTblFloatImp(*(other.m_pImp));
        }
        else
        {
            m_pImp = new (std::nothrow) ILscTbl::LscTblImp(*(other.m_pImp));
        }
    }
    else {
        LSC_LOG("this(%p) == other(%p)", this, &other);
    }

    return *this;
}

ILscTbl::
~ILscTbl()
{
    if (m_pImp) delete m_pImp;
}

ILscTable::TBL_TYPE_T
ILscTbl::
getType() const
{
    return m_pImp->getType();
}

ILscTable::TBL_BAYER_T
ILscTbl::
getBayer() const
{
    return m_pImp->getBayer();
}

const MVOID*
ILscTbl::
getData() const
{
    return m_pImp->getData();
}

MUINT32
ILscTbl::
getSize() const
{
    return m_pImp->getSize();
}

const ILscTable::Config&
ILscTbl::
getConfig() const
{
    return m_pImp->getConfig();
}

const ILscTable::RsvdData&
ILscTbl::
getRsvdData() const
{
    return m_pImp->getRsvdData();
}

MVOID*
ILscTbl::
editData()
{
    return m_pImp->editData();
}

MVOID*
ILscTbl::
editData(int channel)
{
    return m_pImp->editData(channel);
}

ILscTable::RsvdData&
ILscTbl::
editRsvdData()
{
    return m_pImp->editRsvdData();
}

MBOOL
ILscTbl::
setData(const MVOID* src, MUINT32 size)
{
    return m_pImp->setData(src, size);
}

MBOOL
ILscTbl::
setConfig(const ILscTable::Config& rCfg)
{
    return m_pImp->setConfig(rCfg.i4ImgWd, rCfg.i4ImgHt, rCfg.i4GridX, rCfg.i4GridY);
}

MBOOL
ILscTbl::
setConfig(const ConfigBlk& rCfgBlk)
{
    const ILscTable::Config cfg = ::makeConfig(rCfgBlk);
    return m_pImp->setConfig(cfg.i4ImgWd, cfg.i4ImgHt, cfg.i4GridX, cfg.i4GridY);
}

MBOOL
ILscTbl::
setConfig(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)
{
    return m_pImp->setConfig(i4W, i4H, i4GridX, i4GridY);
}

MBOOL
ILscTbl::
setBayer(ILscTable::TBL_BAYER_T bayer)
{
    return m_pImp->setBayer(bayer);
}

MBOOL
ILscTbl::
reorder(ILscTable::TBL_BAYER_T bayer)
{
    return m_pImp->reorder(bayer);
}

MBOOL
ILscTbl::
setRsvdData(const ILscTable::RsvdData& rData)
{
    return m_pImp->setRsvdData(rData);
}

MBOOL
ILscTbl::
dump(const char* filename) const
{
    return m_pImp->dump(filename);
}

MBOOL
ILscTbl::
cropOut(const TransformCfg_T& trfm, ILscTable& output) const
{
    return m_pImp->cropOut(trfm, output);
}

MBOOL
ILscTbl::
cropToGain(const TransformCfg_T& trfm, ILscTable& output, vector<MUINT16*> &vecGain, MUINT32 mode) const
{
    return m_pImp->cropToGain(trfm, output, vecGain, mode);
}

MBOOL
ILscTbl::
mergeAndCrop(ILscTable& baseTbl, ILscTable& output)
{
    return m_pImp->mergeAndCrop(baseTbl, output);
}

MBOOL
ILscTbl::
convert(ILscTable& output) const
{
    return m_pImp->convert(output);
}

MBOOL
ILscTbl::
getRaTbl(MUINT32 u4Ratio, ILscTable& output) const
{
    return m_pImp->getRaTbl(u4Ratio, output);
}

MBOOL
ILscTbl::
toBuf(ILscBuf& buf) const
{
    return m_pImp->toBuf(buf);
}
/*******************************************************************************
 * shadingAlign
 *******************************************************************************/
MINT32
ILscTbl::
shadingAlign(const ILscTbl& golden, const ILscTbl& unit, const ILscTbl& input, ILscTbl& output, MUINT8 u1Rot)
{
    MINT32 i4Ret = 0;

    SHADIND_ALIGN_CONF rSdAlignCfg;
    MUINT8* gWorkinBuffer = NULL;

    // check input golden gain, unit gain, and golden hw table
    if (ILscTable::GAIN_FIXED != golden.getType() ||
        ILscTable::GAIN_FIXED != unit.getType())
    {
        LSC_ERR("Not supported types (%d,%d,%d)", golden.getType(), unit.getType(), input.getType());
        return -2;
    }

    gWorkinBuffer = new (std::nothrow) MUINT8[SHADIND_FUNC_WORKING_BUFFER_SIZE];
    if (!gWorkinBuffer)
    {
        LSC_ERR("Fail to allocate gWorkinBuffer");
        return -1;
    }
    else
    {
        ::memset(gWorkinBuffer, 0, SHADIND_FUNC_WORKING_BUFFER_SIZE*sizeof(MUINT8));
        LSC_LOG("gWorkinBuffer(%p)", gWorkinBuffer);
    }

    Config rCfg = input.getConfig();
    Config rGolgenCfg = golden.getConfig();
    Config rUnitCfg = unit.getConfig();
    ILscTbl tmpGolden = golden;
    ILscTbl tmpUnit = unit;
    ILscTbl tmpInput = input;

    output.setConfig(rCfg);

    rSdAlignCfg.working_buff_addr   = (void*) gWorkinBuffer;
    rSdAlignCfg.working_buff_size   = SHADIND_FUNC_WORKING_BUFFER_SIZE;

    rSdAlignCfg.golden.img_width    = rCfg.i4ImgWd;
    rSdAlignCfg.golden.img_height   = rCfg.i4ImgHt;
    rSdAlignCfg.golden.offset_x     = 0;
    rSdAlignCfg.golden.offset_y     = 0;
    rSdAlignCfg.golden.crop_width   = rCfg.i4ImgWd;
    rSdAlignCfg.golden.crop_height  = rCfg.i4ImgHt;
    rSdAlignCfg.golden.bayer        = (BAYER_ID_T)golden.getBayer();
    rSdAlignCfg.golden.grid_x       = rGolgenCfg.i4GridX;
    rSdAlignCfg.golden.grid_y       = rGolgenCfg.i4GridY;
    rSdAlignCfg.golden.lwidth       = 0;
    rSdAlignCfg.golden.lheight      = 0;
    rSdAlignCfg.golden.ratio_idx    = 0;
    rSdAlignCfg.golden.grgb_same    = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.golden.data_type    = SHADING_TYPE_GAIN;
    rSdAlignCfg.golden.table        = (MUINT32*)tmpGolden.editData();

    rSdAlignCfg.cali.img_width      = rCfg.i4ImgWd;
    rSdAlignCfg.cali.img_height     = rCfg.i4ImgHt;
    rSdAlignCfg.cali.offset_x       = 0;
    rSdAlignCfg.cali.offset_y       = 0;
    rSdAlignCfg.cali.crop_width     = rCfg.i4ImgWd;
    rSdAlignCfg.cali.crop_height    = rCfg.i4ImgHt;
    rSdAlignCfg.cali.bayer          = (BAYER_ID_T)unit.getBayer();
    rSdAlignCfg.cali.grid_x         = rUnitCfg.i4GridX;
    rSdAlignCfg.cali.grid_y         = rUnitCfg.i4GridY;
    rSdAlignCfg.cali.lwidth         = 0;
    rSdAlignCfg.cali.lheight        = 0;
    rSdAlignCfg.cali.ratio_idx      = 0;
    rSdAlignCfg.cali.grgb_same      = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.cali.data_type      = SHADING_TYPE_GAIN;
    rSdAlignCfg.cali.table          = (MUINT32*)tmpUnit.editData();

    rSdAlignCfg.input.img_width     = rCfg.i4ImgWd;
    rSdAlignCfg.input.img_height    = rCfg.i4ImgHt;
    rSdAlignCfg.input.offset_x      = 0;
    rSdAlignCfg.input.offset_y      = 0;
    rSdAlignCfg.input.crop_width    = rCfg.i4ImgWd;
    rSdAlignCfg.input.crop_height   = rCfg.i4ImgHt;
    rSdAlignCfg.input.bayer         = (BAYER_ID_T)BAYER_B;
    rSdAlignCfg.input.grid_x        = rCfg.i4GridX;             // Golden Coef
    rSdAlignCfg.input.grid_y        = rCfg.i4GridY;             // Golden Coef
    rSdAlignCfg.input.lwidth        = 0;
    rSdAlignCfg.input.lheight       = 0;
    rSdAlignCfg.input.ratio_idx     = 0;
    rSdAlignCfg.input.grgb_same     = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.input.data_type     = SHADING_TYPE_COEFF;       // coef
    rSdAlignCfg.input.table         = (MUINT32*)tmpInput.editData();     // Golden Coef

    rSdAlignCfg.output.img_width    = rCfg.i4ImgWd;
    rSdAlignCfg.output.img_height   = rCfg.i4ImgHt;
    rSdAlignCfg.output.offset_x     = 0;                        // crop
    rSdAlignCfg.output.offset_y     = 0;                        // crop
    rSdAlignCfg.output.crop_width   = rCfg.i4ImgWd;             // crop
    rSdAlignCfg.output.crop_height  = rCfg.i4ImgHt;             // crop
    rSdAlignCfg.output.bayer        = (BAYER_ID_T)BAYER_B;
    rSdAlignCfg.output.grid_x       = rCfg.i4GridX;             // Golden Coef
    rSdAlignCfg.output.grid_y       = rCfg.i4GridY;             // Golden Coef
    rSdAlignCfg.output.lwidth       = 0;
    rSdAlignCfg.output.lheight      = 0;
    rSdAlignCfg.output.ratio_idx    = 0;
    rSdAlignCfg.output.grgb_same    = SHADING_GRGB_SAME_YES;
    rSdAlignCfg.output.data_type    = SHADING_TYPE_COEFF;       // coef
    rSdAlignCfg.output.table        = (MUINT32*)output.editData();         // Golden Coef

    switch (u1Rot)
    {
        default:
        case 0:
            rSdAlignCfg.afn = SHADING_AFN_R0D;
        break;
        case 1:
            rSdAlignCfg.afn = SHADING_AFN_R180D;
        break;
        case 2:
            rSdAlignCfg.afn = SHADING_AFN_MIRROR;
        break;
        case 3:
            rSdAlignCfg.afn = SHADING_AFN_FLIP;
        break;
    }

    //_LogShadingSpec("golden", rSdAlignCfg.golden);
    //_LogShadingSpec("cali",   rSdAlignCfg.cali);
    //_LogShadingSpec("input",  rSdAlignCfg.input);
    //_LogShadingSpec("output", rSdAlignCfg.output);

    LSC_RESULT result = S_LSC_CONVERT_OK;

    g_algo_mutex.lock();
    result = shading_align_golden(rSdAlignCfg);
    g_algo_mutex.unlock();

    if (S_LSC_CONVERT_OK != result)
    {
        LSC_ERR("Align Error(%d)", result);
        i4Ret = -1;
    }
    else
    {
        LSC_LOG("Align done.");
    }

    delete [] gWorkinBuffer;

    return i4Ret;
}
