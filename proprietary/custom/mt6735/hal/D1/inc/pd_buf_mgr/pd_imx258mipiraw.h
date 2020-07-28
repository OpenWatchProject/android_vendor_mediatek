#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr.h"

class PD_IMX258MIPIRAW : protected PDBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    MUINT32  m_PDBufSz;
    MUINT16 *m_PDBuf;
    MBOOL m_bDebugEnable;
    MUINT16 m_SensorType; //HDR Type :2, Binning Type : 3

protected :
    /**
    * @brief checking current sensor is supported or not.
    */
    MBOOL IsSupport( SPDProfile_t &iPdProfile);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_IMX258MIPIRAW();
    ~PD_IMX258MIPIRAW();

    static PDBufMgr* getInstance();



    /**
    * @brief get PD calibration data size.
    */
    MINT32 GetPDCalSz();
    /**
    * @brief convert PD data buffer format.
    */
    MUINT16* ConvertPDBufFormat( MUINT32  i4Size, MUINT8 *ptrBufAddr, MUINT32 i4FrmCnt);

};


