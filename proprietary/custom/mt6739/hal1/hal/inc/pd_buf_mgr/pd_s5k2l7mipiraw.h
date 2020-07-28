#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr.h"

class PD_S5K2L7MIPIRAW : protected PDBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.

protected :
    /**
    * @brief checking current sensor is supported or not.
    */
    MBOOL IsSupport( SPDProfile_t &iPdProfile);
    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_S5K2L7MIPIRAW();
    ~PD_S5K2L7MIPIRAW();

    static PDBufMgr* getInstance();

    /**
    * @brief convert PD data buffer format.
    */
    MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData=MFALSE);
    /**
    * @brief output DualPD VC information
    */
    MBOOL GetDualPDVCInfo( MINT32 i4CurSensorMode, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode);
};

