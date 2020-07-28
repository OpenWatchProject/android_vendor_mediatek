#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr.h"

class PD_IMX362MIPIRAW : protected PDBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    MUINT32  m_PDBufSz;
    MUINT16 *m_PDBuf;
    MUINT32  m_PDXSz; //pixels in two byte.
    MUINT32  m_PDYSz; //lines

protected :
    /**
    * @brief checking current sensor is supported or not.
    */
    MBOOL IsSupport( SPDProfile_t &iPdProfile);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_IMX362MIPIRAW();
    ~PD_IMX362MIPIRAW();

    static PDBufMgr* getInstance();

    /**
    * @brief convert PD data buffer format.
    */
    MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth=12, PD_AREA_T *ptrPDRegion=NULL);
    /**
    * @brief output PDO information
    */
    using PDBufMgr::GetLRBufferInfo;
    MBOOL GetLRBufferInfo( MINT32 i4CurSensorMode, SPDOHWINFO_T &oPDOhwInfo);
    /**
    * @brief output DualPD VC information
    */
    MBOOL GetDualPDVCInfo( MINT32 i4CurSensorMode, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode);
};

