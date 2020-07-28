#ifndef __SONY_IMX519_PDAF_LIBRARY_WRAPPER_H__
#define __SONY_IMX519_PDAF_LIBRARY_WRAPPER_H__

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <string>

#include "pd_buf_common.h"

using namespace std;


#define D_IMX519_ALL_PIXEL_SIZE_W (4656)
#define D_IMX519_ALL_PIXEL_SIZE_H (3496)

/* ============================================================
 * Define PD block number.
 * Please reference kernel driver setting of register of AREA_MODE(0x38A3 in imx519).
 * ============================================================ */
#define D_IMX519_PD_BLK_NUM_W (8)
#define D_IMX519_PD_BLK_NUM_H (6)

/* ============================================================
 *  Define the characteristic of calibration data which is stroed in EEPROM.
 *  Please reference calibration data in EEPROM or OTP.
 * ============================================================ */
#define D_IMX519_DCC_IN_EEPROM_BLK_NUM_W (8)
#define D_IMX519_DCC_IN_EEPROM_BLK_NUM_H (6)
#define D_IMX519_DCC_IN_EEPROM_BLK_NUM_TOTAL (D_IMX519_DCC_IN_EEPROM_BLK_NUM_W*D_IMX519_DCC_IN_EEPROM_BLK_NUM_H)

/* ============================================================
 *  Define knots setting of SlopeOffset for PD library.
 *
 *  SlopeOffset blocks' center address is described by PD_AREA which is described in all-pixel mode coordinate.
 *
 *  The knots(block) number of DCC calibration data stroed in EEPROM or OTP and SlopeOffset block setting of PD library should be the same.
 *  If these two values are not the same, please implement down-sampleing or up-scaling method.
 * ============================================================ */
#define D_IMX519_SLOPE_OFFSET_BLK_NUM_W (8)
#define D_IMX519_SLOPE_OFFSET_BLK_NUM_H (6)
#define D_IMX519_SLOPE_OFFSET_BLK_NUM_TOTAL (D_IMX519_SLOPE_OFFSET_BLK_NUM_W*D_IMX519_SLOPE_OFFSET_BLK_NUM_H)

/* ============================================================
 *  Define in pd_imx519mipiraw.cpp  m_u2Reg_AdDaPair_PDWin
 * ============================================================ */
#define D_IMX519_PD_AREA_OFFSET_X (90)
#define D_IMX519_PD_AREA_OFFSET_Y (80)
#define D_IMX519_PD_AREA_SIZE_X (560)
#define D_IMX519_PD_AREA_SIZE_Y (554)
//#define D_IMX519_PD_AREA_SIZE_X   ((D_IMX519_ALL_PIXEL_SIZE_W-2*D_IMX519_PD_AREA_OFFSET_X)/D_IMX519_SLOPE_OFFSET_BLK_NUM_W)
//#define D_IMX519_PD_AREA_SIZE_Y   ((D_IMX519_ALL_PIXEL_SIZE_H-2*D_IMX519_PD_AREA_OFFSET_Y)/D_IMX519_SLOPE_OFFSET_BLK_NUM_H)

/* ============================================================
 *  Define OK/NG knot number.
 *  Define threshold line in each knot.
 * ============================================================ */
#define D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W (5)
#define D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H (5)
#define D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL (D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W*D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H)
#define D_IMX519_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM (2)

/* ============================================================
 *  Define relationship between AF motor driver IC and PD library.
 *  During DCC calibrataion 10bit control code is used.
 *  If 12bit-Driver IC is used, 2bit left shift after defocus
 * ============================================================ */
#define D_IMX519_AF_MOTOR_DRV_CONT_CODE_RESOLUTION (10)
#define D_IMX519_EXTRA_SHIFT_BIT (10-D_IMX519_AF_MOTOR_DRV_CONT_CODE_RESOLUTION)

class SonyIMX519PdafLibraryWrapper
{
private :
    /* ============================================================
     *  Operations.
     * ============================================================ */
    // Copy constructor is disallowed.
    SonyIMX519PdafLibraryWrapper( SonyIMX519PdafLibraryWrapper const&);

    // Copy-assignment operator is disallowed.
    SonyIMX519PdafLibraryWrapper& operator=( SonyIMX519PdafLibraryWrapper const&);

    // debug print
    MBOOL DbgDumpSetting();

    /* ============================================================
     *  to dynamic loading shared library
     * ============================================================ */
    MBOOL  m_bLdDn;
    void  *m_Libhandle;      // handle of shared library.
    void  *m_pfunGetVersion; // function pointer - get library version
    void  *m_pfunGetDefocus; // function pointer - get defocus

    /* ============================================================
     * input and output struct of 3rd party shared library
     * ============================================================ */
    void *m_pDefocusArgIn;
    void *m_pTransRectArgIn;

    /* ============================================================
     *  Calibration related data buffer.
     *  Buffer controller is in this class.
     *
     *    m_aData_Slope :
     *       Defocus conversion coefficient(DCC) is stroed in EEPROM.
     *       According Sony's module calibration manual, AREA_MODE(0x31B0) can be configured as 16x12 or 8x6 during doing DCC calibration.
     *       Following arrarys' size is dependenced on AREA_MODE setting during doing DCC calibration.
     *
     *    m_aData_Offset :
     *       According Sony's porting guide, setting as 0.
     *
     *    m_aXAddress_Knot_Slope_Offset and m_aYAddress_Knot_Slope_Offset :
     *       There is total 192 or 48 knot points is configured during doing DCC calibration.
     *       The X and Y address is set as center coordinate of each blocks.
     * ============================================================ */
    MUINT32        m_u4SlopeOffsetDaNum;
    signed long    m_aData_Slope[D_IMX519_SLOPE_OFFSET_BLK_NUM_TOTAL]; // DCC
    signed long    m_aData_Offset[D_IMX519_SLOPE_OFFSET_BLK_NUM_TOTAL];
    unsigned short m_aXAddress_Knot_Slope_Offset[D_IMX519_SLOPE_OFFSET_BLK_NUM_W];
    unsigned short m_aYAddress_Knot_Slope_Offset[D_IMX519_SLOPE_OFFSET_BLK_NUM_H];

    /* ============================================================
     *  Defocus OK/NG threshold line setting.
     *  Buffer controller is in this class.
     * ============================================================ */
    void           *m_pDefocusOKNGThrLine;
    unsigned short  m_aXAddress_Knot_DefocusOKNG[D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W];
    unsigned short  m_aYAddress_Knot_DefocusOKNG[D_IMX519_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H];

    /* ============================================================
     *  Register setting :
     *  PD area information for fixed area mode on output image coordinates.
     * ============================================================ */
    MUINT16 m_u8PDArea_XOffset;
    MUINT16 m_u8PDArea_YOffset;
    MUINT16 m_u8PDArea_Width;
    MUINT16 m_u8PDArea_Height;

    /* ============================================================
     *  PD data and confidence data data buffer pointer.
     *  Buffer controller is in pd_buf_open class.
     * ============================================================ */
    MUINT8  *m_pPDCLDa;
    MUINT32  m_u4PDCLDaSz;

    /* ============================================================
     *  parameters
     * ============================================================ */
    MBOOL   m_bDmpSetting;
    MBOOL   m_bInitDn;
    MUINT8  m_u8DbgEn;
    MUINT8  m_u8SensMode;
    MUINT8  m_u8FiexibleWindowEn;
    MUINT8  m_u8WindowModeSelect; // 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Fiexible window, 3:Reserved.
    SPDProfile_t m_PdProfile;
protected :

public:
    /* ============================================================
     *  Ctor/Dtor.
     * ============================================================ */
    SonyIMX519PdafLibraryWrapper();

    ~SonyIMX519PdafLibraryWrapper();

    /* ============================================================
     *  Operations.
     * ============================================================ */
    MBOOL Init( SPDProfile_t &iPdProfile);

    MBOOL SetCaliData( MUINT32 u4CaliDaSz, MUINT8 *pCaliDaBuf, MUINT32 u4RegAddDaPairSz, MUINT16 *pRegAddDaPairBuf);

    MBOOL SetPDCLData( MUINT32 u4PDCLDaSz, MUINT8 *pPDCLDaBuf, MINT32 &oWinMode, MUINT32 &oWinEn);

    MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);

    MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData);
};

#endif //__SONY_IMX519_PDAF_LIBRARY_WRAPPER_H__
