#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_imx519mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx519mipiraw"

#define USE_FLEXIBLE_WIN_CFG 1

#define DBG_MSG_BUF_SZ 1024

// Define PDAF supporting sensor mode.
typedef enum
{
    MODE_NO_PDAF =0,
    MODE_FULL,
    MODE_BINNING,

} IMX519_PDAF_SUPPORT_MODE_T;


MUINT32 PD_IMX519MIPIRAW::m_u4Reg_AdDaPair_Sz = (sizeof(MUINT16)*68/sizeof(MUINT8));

MUINT16 PD_IMX519MIPIRAW::m_u2Reg_AdDaPair[68] =
{
    0x0101, 0x0000,
    0x0220, 0x0000,
    0x0221, 0x0000,
    0x0344, 0x0000,
    0x0345, 0x0000,
    0x0346, 0x0000,
    0x0347, 0x0000,
    0x0348, 0x0000,
    0x0349, 0x0000,
    0x034A, 0x0000,
    0x034B, 0x0000,
    0x034C, 0x0000,
    0x034D, 0x0000,
    0x034E, 0x0000,
    0x034F, 0x0000,
    0x0381, 0x0000,
    0x0383, 0x0000,
    0x0385, 0x0000,
    0x0387, 0x0000,
    0x0401, 0x0000,
    0x0404, 0x0000,
    0x0405, 0x0000,
    0x0408, 0x0000,
    0x0409, 0x0000,
    0x040A, 0x0000,
    0x040B, 0x0000,
    0x040C, 0x0000,
    0x040D, 0x0000,
    0x040E, 0x0000,
    0x040F, 0x0000,
    0x0900, 0x0000,
    0x0901, 0x0000,
    0x3F42, 0x0000,
    0x3F43, 0x0000,
};

MUINT32 PD_IMX519MIPIRAW::m_u4Reg_AdDaPair_PDWin_Flexible_Sz = (sizeof(MUINT16)*146/sizeof(MUINT8));

MUINT16 PD_IMX519MIPIRAW::m_u2Reg_AdDaPair_PDWin_Flexible_All[146] =
{
    0x38A3, 0xFFFF, /*AREA MODE*/
    0x38AC, 0xFFFF, /*Flexible area enable, 0:disable, 1:enable*/
    0x38AD, 0xFFFF,
    0x38AE, 0xFFFF,
    0x38AF, 0xFFFF,
    0x38B0, 0xFFFF,
    0x38B1, 0xFFFF,
    0x38B2, 0xFFFF,
    0x38B3, 0xFFFF,
    0x38B4, 0xFFFF, /*Area setting : Win 0*/
    0x38B5, 0xFFFF,
    0x38B6, 0xFFFF,
    0x38B7, 0xFFFF,
    0x38B8, 0xFFFF,
    0x38B9, 0xFFFF,
    0x38BA, 0xFFFF,
    0x38BB, 0xFFFF,
    0x38BC, 0xFFFF, /*Area setting : Win 1*/
    0x38BD, 0xFFFF,
    0x38BE, 0xFFFF,
    0x38BF, 0xFFFF,
    0x38C0, 0xFFFF,
    0x38C1, 0xFFFF,
    0x38C2, 0xFFFF,
    0x38C3, 0xFFFF,
    0x38C4, 0xFFFF, /*Area setting : Win 2*/
    0x38C5, 0xFFFF,
    0x38C6, 0xFFFF,
    0x38C7, 0xFFFF,
    0x38C8, 0xFFFF,
    0x38C9, 0xFFFF,
    0x38CA, 0xFFFF,
    0x38CB, 0xFFFF,
    0x38CC, 0xFFFF, /*Area setting : Win 3*/
    0x38CD, 0xFFFF,
    0x38CE, 0xFFFF,
    0x38CF, 0xFFFF,
    0x38D0, 0xFFFF,
    0x38D1, 0xFFFF,
    0x38D2, 0xFFFF,
    0x38D3, 0xFFFF,
    0x38D4, 0xFFFF, /*Area setting : Win 4*/
    0x38D5, 0xFFFF,
    0x38D6, 0xFFFF,
    0x38D7, 0xFFFF,
    0x38D8, 0xFFFF,
    0x38D9, 0xFFFF,
    0x38DA, 0xFFFF,
    0x38DB, 0xFFFF,
    0x38DC, 0xFFFF, /*Area setting : Win 5*/
    0x38DD, 0xFFFF,
    0x38DE, 0xFFFF,
    0x38DF, 0xFFFF,
    0x38E0, 0xFFFF,
    0x38E1, 0xFFFF,
    0x38E2, 0xFFFF,
    0x38E3, 0xFFFF,
    0x38E4, 0xFFFF, /*Area setting : Win 6*/
    0x38E5, 0xFFFF,
    0x38E6, 0xFFFF,
    0x38E7, 0xFFFF,
    0x38E8, 0xFFFF,
    0x38E9, 0xFFFF,
    0x38EA, 0xFFFF,
    0x38EB, 0xFFFF,
    0x38EC, 0xFFFF, /*Area setting : Win 7*/
    0x38ED, 0xFFFF,
    0x38EE, 0xFFFF,
    0x38EF, 0xFFFF,
    0x38F0, 0xFFFF,
    0x38F1, 0xFFFF,
    0x38F2, 0xFFFF,
    0x38F3, 0xFFFF,
};

MUINT16 PD_IMX519MIPIRAW::m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[150] =
{
    0x0000, 0x00,
};

// Reference in sensor kernel driver
MUINT32 PD_IMX519MIPIRAW::m_u4Reg_AdDaPair_PDWin_Fix_Sz = (sizeof(MUINT16)*34/sizeof(MUINT8));

MUINT16 PD_IMX519MIPIRAW::m_u2Reg_AdDaPair_PDWin_Fix[34] =
{
    /*AREA MODE, 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Flexible area*/
    0x38A3, 0x01,
    /*Fixed area mode*/
    0x38A4, 0x00,
    0x38A5, 0x5A, //0x5A=90
    0x38A6, 0x00,
    0x38A7, 0x50, //0x50=80
    0x38A8, 0x02,
    0x38A9, 0x30, //0x230=560
    0x38AA, 0x02,
    0x38AB, 0x2A, //0x22A=554
    /*Disable flexible area*/
    0x38ac, 0x00,
    0x38ad, 0x00,
    0x38ae, 0x00,
    0x38af, 0x00,
    0x38b0, 0x00,
    0x38b1, 0x00,
    0x38b2, 0x00,
    0x38b3, 0x00,
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDBufMgrOpen*
PD_IMX519MIPIRAW::
getInstance()
{
    static PD_IMX519MIPIRAW singleton;
    return &singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX519MIPIRAW::
PD_IMX519MIPIRAW()
{
    memset( &m_tPDProfile, 0, sizeof(m_tPDProfile));
    pLibWrapper     = new SonyIMX519PdafLibraryWrapper;
    m_bDebugEnable  = MFALSE;
    m_CurrMode      = MODE_NO_PDAF;
    m_WinMode       = 3;  // 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Fiexible window, 3:Reserved.
    m_WinEn         = 0;
    m_LastWinCfgIdx = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX519MIPIRAW::
~PD_IMX519MIPIRAW()
{
    if( pLibWrapper)
        delete pLibWrapper;

    pLibWrapper = NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AREA_DISABLE 0
#define AREA_ENABLE 1

MVOID
PD_IMX519MIPIRAW::
SetRegAdDaPairBuf( MBOOL    en,
                   MINT32   i4RegRoiIdx,
                   MINT32   i4XStart,
                   MINT32   i4YStart,
                   MINT32   i4XEnd,
                   MINT32   i4YEnd,
                   MUINT16 *cmdPairBuf,
                   MUINT32 &cmdPairBufSz)
{
#define FIRST_AREAEN_REG_SETTING_OFFSET 1*2
#define FIRST_AREA_REG_SETTING_OFFSET 9*2
#define NEXT_REG_SETTING_OFFSET 2

    MUINT32 offset = 0;

    if( en)
    {
        /**************************
         * Flexible Mode setting
         **************************/
        offset = 1; //offset to register setting

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset]!=0x02)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = 0x02;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }

        /**************************
         * Coordinate setting
         **************************/
        MUINT8 xStart_H = (i4XStart>>8) & 0x1f;
        MUINT8 xStart_L = (i4XStart   ) & 0xff;

        MUINT8 yStart_H = (i4YStart>>8) & 0xf;
        MUINT8 yStart_L = (i4YStart   ) & 0xff;

        MUINT8 xEnd_H   = (i4XEnd>>8) & 0x1f;
        MUINT8 xEnd_L   = (i4XEnd   ) & 0xff;

        MUINT8 yEnd_H   = (i4YEnd>>8) & 0xf;
        MUINT8 yEnd_L   = (i4YEnd   ) & 0xff;


        /**************************
         * Register setting
         **************************/
        offset = FIRST_AREA_REG_SETTING_OFFSET + 8*i4RegRoiIdx*2 + 1; //offset to register setting

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset]!=xStart_H)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = xStart_H;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if(m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != xStart_L)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = xStart_L;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != yStart_H)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = yStart_H;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != yStart_L)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = yStart_L;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != xEnd_H)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = xEnd_H;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != xEnd_L)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = xEnd_L;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != yEnd_H)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = yEnd_H;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
        offset += NEXT_REG_SETTING_OFFSET;

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != yEnd_L)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = yEnd_L;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }

        //
        offset = FIRST_AREAEN_REG_SETTING_OFFSET + i4RegRoiIdx*2 + 1; //offset to register setting
        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != AREA_ENABLE)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = AREA_ENABLE;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }
    }
    else
    {
        offset = FIRST_AREAEN_REG_SETTING_OFFSET + i4RegRoiIdx*2 + 1; //offset to register setting

        if( m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] != AREA_DISABLE)
        {
            // update local buffer
            m_u2Reg_AdDaPair_PDWin_Flexible_All[offset] = AREA_DISABLE;
            // update command buffer for sensor driver
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset-1]; // register's address
            cmdPairBuf[cmdPairBufSz++] = m_u2Reg_AdDaPair_PDWin_Flexible_All[offset  ]; // register's data
        }

    }

}

MBOOL
PD_IMX519MIPIRAW::
GetPDWinSetting( MVOID* arg1, MVOID* arg2, MVOID* arg3)
{
    MBOOL ret = MFALSE;

    S_CONFIG_DATA_OPEN_T *cfgRoi = (S_CONFIG_DATA_OPEN_T *)arg1;

    AAA_LOGD_IF( m_bDebugEnable, "%s+ numROIs(%d)", __FUNCTION__, cfgRoi->numROIs);

#ifdef USE_FLEXIBLE_WIN_CFG

    cfgRoi->cfgNum = 0;

    memset( m_u2Reg_AdDaPair_PDWin_Flexible_Cmd, 0, sizeof(MUINT16)*150);


#define MAX_FLEXIBLE_WIN_SIZE 8
    // Maxinum flexible window number is 8. Limiting window number as 7 for timing control.
    cfgRoi->numROIs = ((MAX_FLEXIBLE_WIN_SIZE-1)<cfgRoi->numROIs) ? MAX_FLEXIBLE_WIN_SIZE-1 : cfgRoi->numROIs;

    // roi index for configuring last window
    m_LastWinCfgIdx = (m_LastWinCfgIdx<(cfgRoi->numROIs-1))       ? (cfgRoi->numROIs-1) : (m_LastWinCfgIdx+1);
    m_LastWinCfgIdx = ((MAX_FLEXIBLE_WIN_SIZE-1)<m_LastWinCfgIdx) ? (cfgRoi->numROIs-1) : m_LastWinCfgIdx;

    /*********************************************************
     * Generating registers' setting for flexible window mode
     *********************************************************/
    MUINT32 idxCmdPair = 0;
    //
    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[idxCmdPair++] = 0x0104; // register's address
    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[idxCmdPair++] = 0x1; // register's data
    //
    for( MUINT32 i=0, roi_idx=0; i<MAX_FLEXIBLE_WIN_SIZE; i++)
    {
        if( (roi_idx<(cfgRoi->numROIs-1)) || ((roi_idx==(cfgRoi->numROIs-1)) && (i==m_LastWinCfgIdx)))
        {
            /**************************
             * Configured ROI setting
             **************************/
            MINT32 i4XStart = cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4X;
            MINT32 i4YStart = cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4Y;
            MINT32 i4XEnd   = cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4X + cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4W;
            MINT32 i4YEnd   = cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4Y + cfgRoi->ROI[roi_idx].sRoiInfo.sPDROI.i4H;

            SetRegAdDaPairBuf( AREA_ENABLE, i, i4XStart, i4YStart, i4XEnd, i4YEnd, m_u2Reg_AdDaPair_PDWin_Flexible_Cmd, idxCmdPair);

            /**************************
             * output for sw flow control
             **************************/
            cfgRoi->ROI[roi_idx].cfgInfo = i; // !! MUST !!
            cfgRoi->cfgNum |= 1<<i;           // !! MUST !!

            roi_idx ++;
        }
        else
        {
            SetRegAdDaPairBuf( AREA_DISABLE, i, 0, 0, 0, 0, m_u2Reg_AdDaPair_PDWin_Flexible_Cmd, idxCmdPair);
        }
        AAA_LOGD_IF( m_bDebugEnable, "%d cfgNum(0x%x) , roi_idx = %d", i, cfgRoi->cfgNum,roi_idx);

    }
    //
    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[idxCmdPair++] = 0x0104; // register's address
    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[idxCmdPair++] = 0x0; // register's data

    //
    *((MUINT32 *)arg2)  = (sizeof(MUINT16)*idxCmdPair/sizeof(MUINT8));
    *((MUINT16 **)arg3) = &(m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[0]);

    // Debug log
    if( m_bDebugEnable)
    {


        AAA_LOGD_IF( m_bDebugEnable,
                     "DUMP_REG_ALL:(0x%04x/0x%04x)",
                     m_u2Reg_AdDaPair_PDWin_Flexible_All[0],
                     m_u2Reg_AdDaPair_PDWin_Flexible_All[1]);

        for(int i=2; i<146;)
        {
            //
            char  dbgMsgBuf[DBG_MSG_BUF_SZ];
            char* ptrMsgBuf = dbgMsgBuf;
            int   stored_byte_num = 0;

            //
            int val = snprintf( ptrMsgBuf, DBG_MSG_BUF_SZ-stored_byte_num, "DUMP_REG_ALL:");
            if( 0<val)
            {
                stored_byte_num += val;
                ptrMsgBuf += val;
            }

            //
            for(int j=0; j<8 && i<146; j++, i+=2)
            {
                int val = snprintf( ptrMsgBuf,
                                    DBG_MSG_BUF_SZ-stored_byte_num,
                                    "(0x%04x/0x%04x)",
                                    m_u2Reg_AdDaPair_PDWin_Flexible_All[i],
                                    m_u2Reg_AdDaPair_PDWin_Flexible_All[i+1]);
                if( 0<val)
                {
                    stored_byte_num += val;
                    ptrMsgBuf += val;
                }
            }
            AAA_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf);

        }

        //
        for(int i=0; i<150;)
        {
            //
            char  dbgMsgBuf[DBG_MSG_BUF_SZ];
            char* ptrMsgBuf = dbgMsgBuf;
            int   stored_byte_num = 0;

            //
            int val = snprintf( ptrMsgBuf, DBG_MSG_BUF_SZ-stored_byte_num, "DUMP_REG_CMD:");
            if( 0<val)
            {
                stored_byte_num += val;
                ptrMsgBuf += val;
            }

            //
            for(int j=0; j<32 && i<150; j++, i+=2)
            {
                int val = snprintf( ptrMsgBuf,
                                    DBG_MSG_BUF_SZ-stored_byte_num,
                                    "(0x%04x/0x%04x)",
                                    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[i],
                                    m_u2Reg_AdDaPair_PDWin_Flexible_Cmd[i+1]);
                if( 0<val)
                {
                    stored_byte_num += val;
                    ptrMsgBuf += val;
                }
            }
            AAA_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf);
        }
    }

    //
    ret = MTRUE;
#else
    *((MUINT32 *)arg2)  = m_u4Reg_AdDaPair_PDWin_Fix_Sz;
    *((MUINT16 **)arg3) = &(m_u2Reg_AdDaPair_PDWin_Fix[0]);

    if( m_bDebugEnable)
    {
        //
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;
        int   stored_byte_num = 0;

        //
        int val = snprintf( ptrMsgBuf, DBG_MSG_BUF_SZ-stored_byte_num, "DUMP_REG_CMD:");
        if( 0<val)
        {
            stored_byte_num += val;
            ptrMsgBuf += val;
        }

        for(int i=0; i<34; i+=2)
        {

            int val = snprintf( ptrMsgBuf,
                                DBG_MSG_BUF_SZ-stored_byte_num,
                                "(0x%04x/0x%04x)",
                                m_u2Reg_AdDaPair_PDWin_Fix[i],
                                m_u2Reg_AdDaPair_PDWin_Fix[i+1]);
            if( 0<val)
            {
                stored_byte_num += val;
                ptrMsgBuf += val;
            }


        }
        AAA_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf);

    }


    //
    ret = MTRUE;
#endif

    AAA_LOGD_IF( m_bDebugEnable, "%s cfgNum(0x%x), Cmd buffer size (%d) Byte-", __FUNCTION__, cfgRoi->cfgNum, *((MUINT32 *)arg2));

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //
    memcpy( &m_tPDProfile, &iPdProfile, sizeof(SPDProfile_t));


    // reset parameters
    m_LastWinCfgIdx = 0;
    for( MUINT32 i=1; i<146; i+=2)
    {
        m_u2Reg_AdDaPair_PDWin_Flexible_All[i] = 0xFFFF;

    }
    for( MUINT32 i=0; i<146; i+=2)
    {
        AAA_LOGD("%s: reset addr(0x%x) da(0x%x)", __FUNCTION__, m_u2Reg_AdDaPair_PDWin_Flexible_All[i], m_u2Reg_AdDaPair_PDWin_Flexible_All[i+1]);
    }


    // enable/disable debug log
    m_bDebugEnable = property_get_int32("vendor.debug.af_mgr.enable", 0);

    // all-pixel mode.
    if( iPdProfile.uImgXsz==4656 && iPdProfile.uImgYsz==3496)
    {
        ret = MTRUE;
        m_CurrMode = MODE_FULL;
    }
    else if(iPdProfile.uImgXsz==4656 && iPdProfile.uImgYsz==2616)
    {
        ret = MTRUE;
        m_CurrMode = MODE_FULL;
    }
    // binning mode
    else if(iPdProfile.uImgXsz==2328 && iPdProfile.uImgYsz==1728)
    {
        ret = MTRUE;
        m_CurrMode = MODE_BINNING;
    }
    else if(iPdProfile.uImgXsz==1920 && iPdProfile.uImgYsz==1080)
    {
        ret = MTRUE;
        m_CurrMode = MODE_FULL;
    }
    else
    {
        m_CurrMode = MODE_NO_PDAF;
    }

    AAA_LOGD("%s:%d, CurMode:%d, ImgSZ:(%d, %d)\n", __FUNCTION__, ret, m_CurrMode, iPdProfile.uImgXsz, iPdProfile.uImgYsz);

    if( pLibWrapper && (m_CurrMode!=MODE_NO_PDAF))
    {
        pLibWrapper->Init( iPdProfile);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
PD_IMX519MIPIRAW::
GetPDCalSz()
{
    // output number of calibration data in byte
    return (D_IMX519_DCC_IN_EEPROM_BLK_NUM_W*D_IMX519_DCC_IN_EEPROM_BLK_NUM_H*sizeof(signed short));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
GetPDInfo2HybridAF( MINT32  i4InArySz, MINT32 *i4OutAry )
{
    MBOOL ret = MTRUE;

    // rest
    memset( i4OutAry, 0, i4InArySz*sizeof(MINT32));

    // i4InArySz should be 10 which is defined by core PD algorithm.
    if( i4InArySz==10 && i4OutAry)
    {
        // output block size setting for Hybrid AF to crop PD alayzing area.
        switch( m_CurrMode)
        {
            case MODE_FULL :
                // all pixel mode
                i4OutAry[0] = 4656/D_IMX519_PD_BLK_NUM_W;
                i4OutAry[1] = 3496/D_IMX519_PD_BLK_NUM_H;
                break;
            case MODE_BINNING :
                // bining mode
                i4OutAry[0] = 2328/D_IMX519_PD_BLK_NUM_W;
                i4OutAry[1] = 1748/D_IMX519_PD_BLK_NUM_H;
                break;
            default :
                ret = MFALSE;
                AAA_LOGD("Current Sensor mode(%d) is not support PDAF\n", m_CurrMode);
                break;
        }
    }
    else
    {
        ret = MFALSE;
        AAA_LOGD("[%s] Fail, Sz=%d, Addr=%p\n", __FUNCTION__, i4InArySz, i4OutAry);
    }


    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
ExtractPDCL()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->SetPDCLData( m_databuf_size, m_databuf, m_WinMode, m_WinEn);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
ExtractCaliData()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->SetCaliData( m_calidatabuf_size, m_calidatabuf, m_u4Reg_AdDaPair_Sz, m_u2Reg_AdDaPair);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
PD_IMX519MIPIRAW::
GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetVersionOfPdafLibrary( tOutSWVer);

        if( tOutSWVer.MajorVersion==0 && tOutSWVer.MinorVersion==0)
        {
            AAA_LOGE("[%s] Please lincess IMX519 pdaf library from Sony FAE!!!", __FUNCTION__);
        }
        else
        {
            AAA_LOGD("[%s] IMX519 Sony pdaf library version %lu.%lu\n", __FUNCTION__, tOutSWVer.MajorVersion, tOutSWVer.MinorVersion);
        }
    }
    else
    {
        AAA_LOGE("[%s] Please do IMX519 pdaf porting!\n", __FUNCTION__);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData)
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetDefocus( iPDInputData, oPdOutputData);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX519MIPIRAW::
sendCommand( MUINT32 i4Cmd, MVOID *arg1, MVOID *arg2, MVOID *arg3, MVOID *arg4)
{
    MBOOL ret = MFALSE;
    switch( i4Cmd)
    {
        /******************************************************
         *
         * The input argument's type is define in pd_puf_open.
         *
         *            !!! Please MUST follow it !!!
         *
         ******************************************************/
        case PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST :
            *((MUINT32 *)arg1)  = m_u4Reg_AdDaPair_Sz;
            *((MUINT16 **)arg2) = &(m_u2Reg_AdDaPair[0]);
            ret = MTRUE;
            break;

        case PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING :
            ret = GetPDWinSetting( arg1, arg2, arg3);
            break;

        case PDBUFMGR_OPEN_CMD_GET_PD_WIN_MODE_SETTING :
#ifdef USE_FLEXIBLE_WIN_CFG
            *((MINT32 *)arg1)  = 0x02;
#else
            *((MINT32 *)arg1)  = 0x01;
#endif
            ret = MTRUE;
            break;

        case PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO :
        {
            MUINT32  sttBuf_sz = *((MUINT32 *)arg1);
            MUINT8  *sttBuf    = *((MUINT8  **)arg2);

            if( (sttBuf_sz>=2) && (sttBuf!=NULL))
            {
                *((MUINT32 *)arg3) =  sttBuf[0];
                *((MUINT32 *)arg4) = (sttBuf[1]>>6) & 0x3; // 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Fiexible window, 3:Reserved.
                ret = MTRUE;
            }
            else
            {
                *((MUINT32 *)arg3) = 0;
                *((MUINT32 *)arg4) = 3;
                ret = MFALSE;
            }
        }


        break;



        default:
            AAA_LOGD("[%s] Not valid command (%d)", __FUNCTION__, i4Cmd);
            break;
    }
    return ret;
}
