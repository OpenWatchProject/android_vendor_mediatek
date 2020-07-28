#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr.h"

class PD_S5K3M3MIPIRAW : protected PDBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    MUINT16 *m_PDBuf;
    MUINT32  m_PDBufXSz;
    MUINT32  m_PDBufYSz;
    MUINT32  m_PDBufSz;
    MBOOL m_bDebugEnable;

protected :
    /**
    * @brief checking current sensor is supported or not.
    */
    MBOOL IsSupport( SPDProfile_t &iPdProfile);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_S5K3M3MIPIRAW();
    ~PD_S5K3M3MIPIRAW();

    static PDBufMgr* getInstance();



    /**
    * @brief convert PD data buffer format.
    */
    MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth=12, PD_AREA_T *ptrPDRegion=NULL);
    /**
    * @brief get m_PDXSz, m_PDYSz, and m_PDBufSz.
    */
    MBOOL GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz);
};


