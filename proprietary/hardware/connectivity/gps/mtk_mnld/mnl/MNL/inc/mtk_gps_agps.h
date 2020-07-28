/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2008]. All rights reserved.
*
*************************************************************************/
#ifndef MTK_GPS_AGPS_H
#define MTK_GPS_AGPS_H


#ifdef __cplusplus
   extern "C" {
#endif

#include "mtk_gps_type.h"

#define MGPSID (32)
#if ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000 ) )
// for ADS1.x
#elif ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 400000 ) )
// for RVCT2.x or RVCT3.x
#else
#pragma pack(4)
#endif



#define GPS_EPH_WORD_MAX    24
#if 1//def AGPS_SUPPORT_GNSS
#define GLO_EPH_WORD_MAX    12
#define BDS_EPH_WORD_MAX    15
#define GAL_EPH_WORD_MAX    18
#endif

#define GPS_ALM_WORD_MAX    8
#if 1//def AGPS_SUPPORT_GNSS
#define GLO_ALM_WORD_MAX    6
#define BDS_ALM_WORD_MAX    8
#define GAL_ALM_WORD_MAX    19
#endif
#define SUPPORT_AGA

typedef struct
{
  UINT8 u1Arg1;
  UINT8 u1Arg2;
  UINT8 u1Arg3; //VERSION
  UINT8 u1Arg4; //C2K Flag
} MTK_GPS_AGPS_CMD_MODE_T;

typedef struct
{
#ifdef AGPS_SUPPORT_QOPMS
  float u1Delay;  // Response time in unit of sec.
                  // change  from interge sec to mini-sec. so the Required response time will be more accurate
                  // for example
                  //old : $PMTK293,16,0,0,51*<Check Sum>
                  //new: $PMTK293,15.296,0,0,51*<Check Sum>
#else
  UINT8 u1Delay;  // Response time in unit of sec.
#endif
  UINT32 u4HAcc;  // Horizontal accuracy in unit of meter.
  UINT16 u2VAcc;  // Vertical accuracy in unit of meter.
  UINT16 u2PRAcc; // Pseudorange accuracy in unit of meter.
  float  f4DelayEarlyFix; //[TS 36.355 R12] new item->responseTimeEarlyFix-r12
  UINT32 u4PeriodicMode;// For Moving Scenario, default 0:normal mod, 1:moving scenario, >1, reserved.
  UINT8 u1HAccConf;//(MSB only)Horizontal confidence [0~100]. 0 consider as "no information" and consider it as 90%.
  UINT8 u1VAccConf;//(MSB only)Confidence associated with the altitude[0~100].0 consider as "no information" and consider it as 90%.
  UINT8 u1PRAccConf;//(MSA only)MA accuracy confidence associated with the pseudorange error [0~100].0 consider as "no information" and consider it as 90%.
} MTK_GPS_AGPS_CMD_QOP_T;

typedef struct
{
  UINT8 u1SvId;
  UINT32 au4Word[GPS_EPH_WORD_MAX];
} MTK_GPS_ASSIST_EPH_T;

typedef struct
{
  UINT32 u1SvId;
  UINT32 au1Byte[55];
  UINT32 u1Lf;
} MTK_ASSIST_GLON_EPH_T;


typedef struct
{
  UINT8 u1SvId;
  UINT16 u2WeekNo;
  UINT32 au4Word[GPS_ALM_WORD_MAX];
} MTK_GPS_ASSIST_ALM_T;

typedef struct
{
  UINT16 u2WeekNo;
  double dfTow;        // sec
  double dfTowRms;     // ms
  double dfFS_Tow;     // sec, not used
  double dfFS_TowRms;  // ms, not used
  MTK_GPS_BOOL fgTimeComp;   /* TRUE: indicate ref time compensation from modem , FALSE: no ref time composate   */
  double   dfTimeDelay;      /* the value of ref time compensation */
  UINT8  gpsWeekCycleNumber; //INTEGER(0..7) -- GPS Acquisition Assistance Rel-10 Extension
} MTK_GPS_ASSIST_TIM_T;

typedef struct
{
  UINT16 u2WeekNo;
  double dfTow;        // sec
  double dfTowRms;     // ms
  double dfFTime;      // sec, Frame Time correspond to GPS TOW
  double dfFTimeRms;   // ms, Frame Time RMS accuracy
} MTK_GPS_ASSIST_FTA_T, MTK_GPS_AGPS_DT_FTIME_T;    // Fine Time Assistance

typedef struct
{
  double dfLat;        // Receiver Latitude in degrees
  double dfLon;        // Receiver Longitude in degrees
  double dfAlt;        // Receiver Altitude in meters
  float fAcc_Maj;     // semi-major RMS accuracy [m]
  float fAcc_Min;     // semi-minor RMS accuracy [m]
  UINT8 u1Maj_Bear;   // Bearing of semi-major axis in degrees
  float fAcc_Vert;    // Vertical RMS accuracy [m]
  UINT8 u1Confidence; // Position Confidence: range from 0 ~ 100 [%]
} MTK_GPS_ASSIST_LOC_T;

typedef struct
{
   double dUtc_hms;  // UTC: hhmmss.sss
   double dUtc_ymd;  //  UTC: yyyymmdd
   UINT8 u1FixType;  // the type of measurements performed by the MS [0: 2D or 1: 3D]
   double dfLat;     // latitude (degree)
   double dfLon;     // longitude (degree)
   INT16 i2Alt;      // altitude (m)
   float fUnc_SMaj;  // semi-major axis of error ellipse (m)
   float fUnc_SMin;  // semi-minor axis of error ellipse (m)
   UINT16 u2Maj_brg; // bearing of the semi-major axis (degrees) [0 - 179]
   float fUnc_Vert;  // Altitude uncertainty
   UINT8 u1Conf;     // The confidence by which the position is known to be within the shape description, expressed as a percentage. [0 ~ 100] (%)
   UINT16 u2HSpeed;  // Horizontal speed (km/hr)
   UINT16 u2Bearing; // Direction (degree) of the horizontal speed [0 ~ 359]
} MTK_GPS_AGPS_CMD_MA_LOC_T;
typedef struct
{
  double dfClkDrift;   // GPS Clock Frequency Error [nsec/sec]
  INT32 i4ClkRMSAcc;  // Frequency Measurement RSM Accuracy [nsec/sec]
  INT32 i4ClkAge;     // Age (sec) of the clock drift value since last estimated
} MTK_GPS_ASSIST_CLK_T;

typedef struct
{
  INT8 i1a0;         // Klobuchar - alpha 0  (seconds)           / (2^-30)
  INT8 i1a1;         // Klobuchar - alpha 1  (sec/semi-circle)   / (2^-27/PI)
  INT8 i1a2;         // Klobuchar - alpha 2  (sec/semi-circle^2) / (2^-24/PI^2)
  INT8 i1a3;         // Klobuchar - alpha 3  (sec/semi-circle^3) / (2^-24/PI^3)
  INT8 i1b0;         // Klobuchar - beta 0   (seconds)           / (2^11)
  INT8 i1b1;         // Klobuchar - beta 1   (sec/semi-circle)   / (2^14/PI)
  INT8 i1b2;         // Klobuchar - beta 2   (sec/semi-circle^2) / (2^16/PI^2)
  INT8 i1b3;         // Klobuchar - beta 3   (sec/semi-circle^3) / (2^16/PI^3)
} MTK_GPS_ASSIST_KLB_T;

typedef struct
{
  INT32 i4A1;         // UTC parameter A1 (seconds/second)/(2^-50)
  INT32 i4A0;         // UTC parameter A0 (seconds)/(2^-30)
  UINT8 u1Tot;        // UTC reference time of week (seconds)/(2^12)
  UINT8 u1WNt;        // UTC reference week number (weeks)
  INT8 i1dtLS;       // UTC time difference due to leap seconds before event (seconds)
  UINT8 u1WNLSF;      // UTC week number when next leap second event occurs (weeks)
  UINT8 u1DN;         // UTC day of week when next leap second event occurs (days)
  INT8 i1dtLSF;      // UTC time difference due to leap seconds after event (seconds)
} MTK_GPS_ASSIST_UCP_T;

typedef struct
{
  INT8 i1NumBad;        // Number of Bad Satellites listed
  UINT8 au1SvId[MTK_GPS_SV_MAX_PRN]; // A list of bad SV id
} MTK_GPS_ASSIST_BSV_T;


typedef struct
{
  UINT8 u1SV;          // SV PRN number (1 ~ 32) (0 means no data available)
  INT32 i4GPSTOW;      // TOW of last Acquisition Assistance data, Units 0.08 sec
  INT16 i2Dopp;        // Doppler value. Units 2.5 Hz
  INT8 i1DoppRate;    // Doppler rate of change. Units (1/42) Hz/s
  UINT8 u1DoppSR;      // Doppler search range. index. [0 ~ 4]
  UINT16 u2Code_Ph;     // C/A Code Phase chips [range 0..1022]
                    //    relative to the previous msec edge
  INT8 i1Code_Ph_Int; // Integer C/A Code msec into the GPS Data Bit
                    //    [range 0..19 msec]  (-1 if not known)
  INT8 i1GPS_Bit_Num; // GPS Data Bit Number, modulo 80 msec  [range 0..3]
                    //    (-1 if not known)
  UINT8 u1CodeSR;      // Code search range. index. [0 ~ 15]
  UINT8 u1Azim;        // Azimuth. Units 11.25 degrees
  UINT8 u1Elev;        // Elevation. Units 11.25 degrees
} MTK_GPS_ASSIST_ACQ_T;


#define RTCM_MAX_N_SAT 11
typedef struct
{
  UINT8 u1SatID;  // [1 - 32]
  UINT8 u1IODE;   // [0 - 255]
  UINT8 u1UDRE;   // [0 - 3]
  INT16 i2PRC;    // [-655.04 - 655.04], Units 0.32m
  INT8 i1RRC;    // [-4.064 - 4.064], Units 0.032m
} MTK_GPS_RTCM_SV_CORR_T;

typedef struct
{
  UINT32 u4Tow;     // the baseline time for the corrections are valid [0 - 604799]
  UINT8 u1Status;  // the status of the differential corrections [0 - 7]
  UINT8 u1NumSv;   // the number of satellites for which differential corrections are available [1 - 11]
  MTK_GPS_RTCM_SV_CORR_T arSVC[RTCM_MAX_N_SAT];
} MTK_GPS_ASSIST_DGP_T;

#define TOW_MAX_N_SAT 11
typedef struct
{
    UINT8 u1SatID;   // [1 - 32]
    UINT16 u2TLM;    // [0 - 16383]
    UINT8 u1Anti_s;  // [0 - 1]
    UINT8 u1Alert;   // [0 - 1]
    UINT8 u1Reserved;  // [0 - 3]
} MTK_GPS_TOW_SV_T;

typedef struct
{
  UINT16 u2WN;      // GPS week number (weeks)
  UINT32 u4Tow;     // GPS time of week  of the TLM message applied [0 - 604799]
  UINT8 u1NumSv;   // the number of satellites for which TOW assist are available [1 - 11]
  MTK_GPS_TOW_SV_T atSV[TOW_MAX_N_SAT];
} MTK_GPS_ASSIST_TOW_T;

typedef struct
{
  MTK_GPS_BOOL fgAcceptAlm;    // Satellite Almanac
  MTK_GPS_BOOL fgAcceptUcp;    // UTC Model
  MTK_GPS_BOOL fgAcceptKlb;    // Ionospheric Model
  MTK_GPS_BOOL fgAcceptEph;    // Navigation Model
  MTK_GPS_BOOL fgAcceptDgps;   // DGPS Corrections
  MTK_GPS_BOOL fgAcceptLoc;    // Reference Location
  MTK_GPS_BOOL fgAcceptTim;    // Reference Time
  MTK_GPS_BOOL fgAcceptAcq;    // Acquisition Assistance
  MTK_GPS_BOOL fgAcceptBsv;    // Real-Time Integrity
} MTK_GPS_AGPS_CMD_ACCEPT_MAP_T;

typedef struct
{
   UINT32 u4Frame;   // BTS Reference Frame number during which the location estimate was measured [0 - 65535]
   UINT16 u2WeekNo;  // the GPS week number for which the location estimate is valid
   UINT32 u4TowMS;   // the GPS TOW (ms) for which the location estimate is valid [0 - 604799999]
   UINT8 u1FixType;  // the type of measurements performed by the MS [0: 2D or 1: 3D]
   double dfLat;         // latitude (degree)
   double dfLon;         // longitude (degree)
   INT16 i2Alt;      // altitude (m)
   float fUnc_SMaj;      // semi-major axis of error ellipse (m)
   float fUnc_SMin;      // semi-minor axis of error ellipse (m)
   UINT16 u2Maj_brg; // bearing of the semi-major axis (degrees) [0 - 179]
   float fUnc_Vert;      // Altitude uncertainty
   UINT8 u1Conf;     // The confidence by which the position is known to be within the shape description, expressed as a percentage. [0 ~ 100] (%)
   UINT16 u2HSpeed;  // Horizontal speed (km/hr)
   UINT16 u2Bearing; // Direction (degree) of the horizontal speed [0 ~ 359]
} MTK_GPS_AGPS_DT_LOC_EST_T;


typedef struct               // Satellite Pseudorange Measurement Data
{
   UINT8 u1PRN_num;      // Satellite PRN number [1 - 32]
   UINT8 u1SNR;          // Satellite Signal to Noise Ratio [dBHz} (range 0-63)
   INT16 i2Dopp;         // Measured Doppler frequency [0.2 Hz] (range +/-6553.6)
   UINT16 u2Code_whole;   // Satellite Code phase measurement - whole chips
                    //   [C/A chips] (range 0..1022)
   UINT16 u2Code_fract;   // Satellite Code phase measurement - fractional chips
                    //   [2^-10 C/A chips] (range 0..1023)
   UINT8 u1Mul_Path_Ind; // Multipath indicator (range 0..3)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-7)
   UINT8 u1Range_RMS_Exp;// Pseudorange RMS error: Exponent (range 0..7)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-8)
   UINT8 u1Range_RMS_Man;// Pseudorange RMS error: Mantissa (range 0..7)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-8)

} MTK_GPS_AGPS_PRM_SV_DATA_T;        // Satellite Pseudorange Measurement Data

#define AGPS_RRLP_MAX_PRM 14
typedef struct
{
   UINT32 u4Frame;         // [0 - 65535]
   UINT8 u1NumValidMeas;  // Number of valid measurements available (0..NUM_CH)
   UINT32 u4GpsTow;        // Time of validity [ms] modulus 14400000
   MTK_GPS_AGPS_PRM_SV_DATA_T SV_Data[AGPS_RRLP_MAX_PRM];  // Satellite Pseudorange Measurement Data
} MTK_GPS_AGPS_DT_GPS_MEAS_T;     // RRLP Pseudorange Data

typedef struct
{
    UINT8 u1Type;
} MTK_GPS_AGPS_DT_FTIME_ERR_T;

typedef struct
{
    UINT16 u2AckCmd;
    UINT8 u1Flag;
} MTK_GPS_AGPS_DT_ACK_T;

typedef struct
{
    UINT8 u1Type;
} MTK_GPS_AGPS_DT_LOC_ERR_T;

typedef struct
{
    UINT16 u2BitMap;
                        //  bit0 0x0001  // almanac
                        //  bit1 0x0002  // UTC model
                        //  bit2 0x0004  // ionospheric model
                        //  bit3 0x0008  // navigation data
                        //  bit4 0x0010  // DGPS corrections
                        //  bit5 0x0020  // reference location
                        //  bit6 0x0040  // reference time
                        //  bit7 0x0080  // acquisition assistance
                        //  bit8 0x0100  // Real-Time integrity
} MTK_GPS_AGPS_DT_REQ_ASSIST_T;
typedef struct
{

    UINT8 u1FixType;
    UINT8 u1FixQuality;
    UINT8 u1SelectType;
    double dfWRTSeaLevel;
    float dfPDOP;
    float dfHDOP;
    float dfVDOP;

    UINT8 u1SatNumInUse;
    UINT8 SVInUsePRNs[MTK_GPS_SV_MAX_NUM];

    UINT8 u1SatNumInView;
    UINT8 u1SVInViewPRNs[MTK_GPS_SV_MAX_NUM];
    INT8  u1SVInViewEle[MTK_GPS_SV_MAX_NUM];
    UINT16 u1SVInViewAzi[MTK_GPS_SV_MAX_NUM];
    float u1SVInViewSNR[MTK_GPS_SV_MAX_NUM];

} MTK_GPS_AGPS_DT_LOC_EXTRA_T;
#if defined(AGPS_SUPPORT_GNSS)
//start for AGPS_SUPPORT_GNSS
/* for AGPS with GNSS supported GNSS assist bitmap   */

/* The BITMAP is define for PMTK760 coding operation of MODEM/AGPSD/MNL , but NOT the same as GNSS ID(3GPP protocol). */
#define BITMAP_GPS     0
#define BITMAP_GLONASS 1
#define BITMAP_GALILEO 2
#define BITMAP_BEIDOU  3


#define BITINDEX_TMOD    0
#define BITINDEX_DGNSS   1
#define BITINDEX_EPH     2
#define BITINDEX_RTI     3
#define BITINDEX_DBA     4
#define BITINDEX_AA      5
#define BITINDEX_ALM     6
#define BITINDEX_UTC     7
#define BITINDEX_AUX     8

#define BITMAP_TMOD    0x0001
#define BITMAP_DGNSS   0x0002
#define BITMAP_EPH     0x0004
#define BITMAP_RTI     0x0008
#define BITMAP_DBA     0x0010
#define BITMAP_AA      0x0020
#define BITMAP_ALM     0x0040
#define BITMAP_UTC     0x0080
#define BITMAP_AUX     0x0100

typedef struct
{
   //UINT16 u2BitMap;
   MTK_GPS_BOOL fgReqTime; //commonAssistDataReq: GNSS reference time request flag
   MTK_GPS_BOOL fgReqLoc;  //commonAssistDataReq: GNSS reference location request flag
   MTK_GPS_BOOL fgReqIon;  //commonAssistDataReq: GNSS reference Inospheris request flag
   MTK_GPS_BOOL fgReqEop;  //commonAssistDataReq: GNSS reference EarthOrentationParameters request flag
   UINT16       u2BitMap[GNSS_ID_MAX_NUM];  //GenerateAssistDataReq:  u2BitMap
                        //  bit0 0x0001  // Time mode Request
                        //  bit1 0x0002  // Differential Corrections Request
                        //  bit2 0x0004  // Navigation Model(Ephemeris) Request
                        //  bit3 0x0008  // Real-Time Integrity Request
                        //  bit4 0x0010  // Data Bit Assistance Request
                        //  bit5 0x0020  // Acquisition Assist Request
                        //  bit6 0x0040  // Almanac Request
                        //  bit7 0x0080  // UTC Model Request
                        //  bit8 0x0100  // Auxiliary Information Request
} MTK_AGNSS_DT_REQ_ASSIST_T;

typedef struct
{

   UINT8  u1GnssTimeID;    /* gnss system time ID:eque to GNSS-ID(,0-GPS,1-SBAS, 2-QZSS,3-Galileo,4-GLONASS, 5~16 reserved) */
   INT16 i2DayNo;         /* [0-32767] gnssDayNumber do not report DayNo to server */
   UINT32 u4Todms;        /* [0-3599999] in seconds */
   UINT16 u2GnssIDsInUse;   //in-use GNSSID bitmap :
                           //bit 0:gps
                           //bit 1:sbas
                           //bit 2:qzss
                           //bit 3:galileo
                           //bit 4:glonass
                           //bit 5~15:reserved

   UINT8 u1FixType;  // the type of measurements performed by the MS [0: 2D or 1: 3D]
   double dfLat;         // latitude (degree)
   double dfLon;         // longitude (degree)
   INT16 i2Alt;      // altitude (m)
   float fUnc_SMaj;      // semi-major axis of error ellipse (m)
   float fUnc_SMin;      // semi-minor axis of error ellipse (m)
   UINT16 u2Maj_brg; // bearing of the semi-major axis (degrees) [0 - 179]
   float fUnc_Vert;      // Altitude uncertainty
   UINT8 u1Conf;     // The confidence by which the position is known to be within the shape description, expressed as a percentage. [0 ~ 100] (%)
   UINT16 u2HSpeed;  // Horizontal speed (km/hr)
   UINT16 u2Bearing; // Direction (degree) of the horizontal speed [0 ~ 359]

   UINT16 u4Frame;
   UINT16 u2GnssIDsUsed;

} MTK_AGNSS_DT_LOC_EST_T;

typedef struct
{

   UINT16 u1PRN_num;               //
   MTK_GNSS_ID_ENUM eGnssID;       //in-use GNSSID  :
                           //bit 0:gps
                           //bit 1:sbas
                           //bit 2:qzss
                           //bit 3:galileo
                           //bit 4:glonass
                           //bit 5~15:reserved
   UINT8 u2SignalID;       //in-use signal ID  :
                           //now for GPS only support GNSS_SGN_ID_VALUE_GPS_L1C_A
                           //now for GLONASS only support GNSS_SGN_ID_VALUE_GLONASS_G1
                           //now for QZSS only support GNSS_SGN_ID_VALUE_QZSS_L1C_A

   UINT8  u1SNR;         //[0..63] SNR
   INT16  i2Dopp;        // [-32768..32767]
   UINT32 u4CodePh;      //[0..2097151] ms 21-bits with 2^-21 resolution
   UINT8  u1CodePhInt;   //[0~127] ms
   UINT8  u1Mul_Path_Ind;  //[0~3] refer to RRLP table A.9

   UINT8  u1CarryQualInd; //[0~3]
   UINT32 u1Adr;          //[0..33554431] 25-bits with 2^-10 resolution
   UINT8  u1Range_RMS_Man ;         //Pseudorange RMS Error mantissa
   UINT8  u1Range_RMS_Exp;         //Pseudorange RMS Error Exponent

   UINT8  u1SigBitmap;
}MTK_GPS_AGNSS_PRM_SV_DATA_T;
#define AGNSS_RRLP_MAX_PRM 100
typedef struct
{
  UINT8 u1NumValidGPSSig;
  UINT8 u1GPSSigIDs;
  UINT8 u1NumValidGLOSig;
  UINT8 u1GLOSigIDs;
}MTK_AGNSS_SIGNALIDS_T;

typedef struct
{

   UINT8  u1GnssTimeID;    /* gnss system time ID:eque to GNSS-ID(,0-GPS,1-SBAS, 2-QZSS,3-Galileo,4-GLONASS, 5~16 reserved) */
   INT16  i2DayNo;
   UINT32 u4Todms;        /* [0-3599999] in ms */
   UINT16 u2TodFrag;      /* [0-3999] with resolution of 250ns optional */
   UINT32 u4TodUnc ;         /* [0-127] with resolution of 250ns optional */
   UINT8  u1NumValidMeas;
   UINT8  u1NumValidGnss;
   UINT16 u2GnssIDsInUse;     //in-use GNSSID bitmap
   MTK_AGNSS_SIGNALIDS_T  u1SignalIDsInUse;   //in-use GNSSID bitmap :
   UINT8  u1CodePhAmb;     // [0~127 ]the codephase ambiguity in interge ms default set to 0¡£ optional
   MTK_GPS_AGNSS_PRM_SV_DATA_T SV_Data[AGNSS_RRLP_MAX_PRM];  // Satellite Pseudorange Measurement Data

} MTK_AGNSS_DT_MEAS_T;     // RRLP Pseudorange Data MTK_AGNSS_DT_REQ_ASSIST_T
//MTK_GPS_AGPS_DT_GPS_MEAS_T


/*=== GNSS Common Assistance Data ===*/
typedef struct
{
    UINT8    u1GnssTimeID;    /* gnss system time ID:eque to GNSS-ID(,0-GPS,1-SBAS, 2-QZSS,3-Galileo,4-GLONASS, 5~16 reserved) */
    /**
     * This field specifies the sequential number of days from the origin of the GNSS System Time as follows:
     * GPS, QZSS, SBAS  Days from January 6th 1980 00:00:00 UTC(USNO)
     * Galileo ¡V             TBD;
     * GLONASS ¡V         Days from January 1st 1996
     */
    UINT16   u2DN;           /* [0-32767] gnssDayNumber  */
    double   dfTod;          /* [0-86399.999] in seconds */
    double   dfTodRms;       /* K = [0..127], uncertainty r (microseconds) = C*(((1+x)^K)-1), C=0.5, x=0.14 */
    MTK_GPS_BOOL u1NotifyLeap;   /* [0~1] flag to notify leap second only present when gnss=GLONASS */
    MTK_GPS_BOOL fgTimeComp;   /* TRUE: indicate ref time compensation from modem , FALSE: no ref time composate   */
    double   dfTimeDelay;      /* the ref time compensation */
    UINT8    ganssDayCycleNumber;//INTEGER(0..7) -- GPS Acquisition Assistance Rel-10 Extension
} MTK_GNSS_ASSIST_TIM_T;

typedef MTK_GPS_ASSIST_LOC_T MTK_GNSS_ASSIST_LOC_T;

/* start for gnss ionospheric model */

//gnss Klobuchar model
typedef struct
{
    UINT8 u1DataID;              /*bit[0..1] : "11"generated by QZSS. aplicable within the area of QZSS */
                                 /*               "00"generated by GPS,GLONASS.  aplicable worldwild   */
    MTK_GPS_ASSIST_KLB_T rdata;  /*these field is simillar to GPS Klobuchar struct */

}MTK_GNSS_ASSIST_KLB_T;

//gnss NeQuick model
typedef struct
{
    UINT16  u2Ai0;  /* [0..4095] */
    UINT16  u2Ai1;  /* [0..4095] */
    UINT16  u2Ai2;  /* [0..4095] */
    /* optional field */
    /**
      * iono storm flag represent five region: [value 0: no disturbance, value 1: disturbance]
      *  region 1: for the northern region (60¢X<MODIP<90¢X)
      *  region 2: for the northern middle region (30¢X<MODIP<60¢X)
      *  region 3: for the equatorial region (-30¢X<MODIP<30¢X)
      *  region 4: for the southern middle region (-60¢X<MODIP<-30¢X)
      *  region 5: for the southern region (-90¢X<MODIP<-60¢X)
      */
    UINT8   u2StValidBit;  /*Stormflag valid bitmap to indicate if fgStormFlag is aviliable or not */
                              /*bit0 -> indicate fgStormFlag[0]  aviliable or not*/
                              /*bit1 -> indicate fgStormFlag[1]  aviliable or not*/
                              /*bit2 -> indicate fgStormFlag[2]  aviliable or not*/
                              /*bit3 -> indicate fgStormFlag[3]  aviliable or not*/
                              /*bit4 -> indicate fgStormFlag[4]  aviliable or not*/
    MTK_GPS_BOOL fgStormFlag[5];

}MTK_GNSS_ASSIST_NQK_T;

typedef struct
{
    UINT8 u1InoModel;            /*[0~7] 0: INO Klobuchar Model -used for GPS,GLONASS,QZSS */
                                 /*         1: NeQuick Model          - used for Galileo */
  union
  {
    MTK_GNSS_ASSIST_KLB_T rAKlb;  /*Klobuchar Model  */
    MTK_GNSS_ASSIST_NQK_T rANqk;  /*NeQuick    Model  */
 }data;

}MTK_GNSS_ASSIST_ION_T;

/* end for gnss ionospheric model */


/* start for gnss earth orientation parameters */
typedef struct
{
    UINT16  u2Teop;         /* [0..65535], EOP data reference time in seconds, scale factor 2^4 seconds */
    INT32   i4PmX;          /* [-1048576..1048575], X-axis polar motion value at reference time in arc-seconds, scale factor 2^(-20) arc-seconds */
    INT16   i2PmXdot;       /* [-16384..16383], X-axis polar motion drift at reference time in arc-seconds/day, scale factor 2^(-21) arc-seconds/day */
    INT32   i4PmY;          /* [-1048576..1048575], Y-axis polar motion value at reference time in arc-seconds, scale factor 2^(-20) arc-seconds */
    INT16   i2PmYdot;       /* [-16384..16383] Y-axis polar motion drift at reference time in arc-seconds/day, scale factor 2^(-21) arc-seconds/day */
    INT32   i4DeltaUT1;     /* [-1073741824..1073741823], UT1-UTC diff at reference time in seconds, scale factor 2^(-24) seconds */
    INT32   i4DeltaUT1dot;  /* [-262144..262143], the rate of UT1-UTC diff at reference time in seconds/day, scale factor 2^(-25) seconds/day */
} MTK_GNSS_ASSIST_EOP_T;
/* end for gnss earth orientation parameters */




/*=== GNSS Generic Assistance Data ===*/

/* start for gnss time model */
/**
 * in LPP, location server could provide up to 15 GNSS-GNSS system time offset
 * in RRC/RRLP, location server could provide up to 7 GNSS-GNSS system time offset
 * i.e. generic assist data is for GPS, time model could provide GPS-GLONASS time offset
 */
typedef struct
{
    /* note that RRC/RRLP tA0, tA1 range is larger than LPP, although scale factor is the same */
    MTK_GNSS_ID_ENUM eGnssID;
    UINT16        u2TmodTOW;  /* [0..65535], the reference time of week TOW , scale factor 2^4 seconds */
    INT32         i4Ta0;         /* [-67108864..67108863] for LPP, [-2147483648 .. 2147483647] for RRC/RRLP, the bias coefficient, scale factor 2^(-35) seconds */
    INT16         i2Ta1;         /* [-4096..4095] for LPP, [-8388608 .. 8388607] for RRC/RRLP, the drift coefficient, scale factor 2^(-51) seconds/second */
    INT8          i1Ta2;         /* [-64..63], the drift rate correction coefficient, scale factor 2^(-68) seconds/second^2 */
    MTK_GNSS_TO_ID_ENUM   eGnssToId; /* GPS, Galileo, QZSS, GLOANSS */
    /* optional field */
    UINT16        u2WeekNo;    /* [0..8191], the reference week */
    INT8          i1DeltaT;    /* [-128..127], the integer number of seconds of GNSS-GNSS time offset */
} MTK_GNSS_ASSIST_TMOD_T;


typedef MTK_GPS_ASSIST_EPH_T MTK_ASSIST_GPS_EPH_T;
//aGLONASS EPH
typedef struct
{
  UINT8 u1SvId;  // GLONASS SV PRN number 1~24
  UINT32 au4Word[GLO_EPH_WORD_MAX];
} MTK_GLO_ASSIST_EPH_T;

typedef struct
{
  UINT8 u1SvId;  // BD SV PRN number 1~30
  UINT32 au4Word[BDS_EPH_WORD_MAX];
} MTK_BDS_ASSIST_EPH_T;

#ifdef SUPPORT_AGA
typedef struct
{
  UINT8 u1SvId;  // Gallileo SV PRN number 1~30
  UINT32 au4Word[GAL_EPH_WORD_MAX];
} MTK_GAL_ASSIST_EPH_T;
#endif

typedef struct
{
    MTK_GNSS_ID_ENUM  eGnssID;

    union
    {
       MTK_GPS_ASSIST_EPH_T  rAGpsEph;  /* for GPS eph  */
       MTK_GLO_ASSIST_EPH_T  rAGloEph;  /* for GLO eph  */
       MTK_BDS_ASSIST_EPH_T   rABDEph;   /* for BD eph  */
      // MTK_QZS_ASSIST_EPH_T  rAQzsEph;     /* for GLONASS */
       MTK_GAL_ASSIST_EPH_T  rAGAEph;     /* for Gallileo */
    } data;
} MTK_GNSS_ASSIST_EPH_T;

//aGLONASS ALM
typedef struct
{
  UINT8 u1SvId;//GLONASS SV PRN:1~24
  UINT16 u2DayNum;
  UINT32 au4Word[GLO_ALM_WORD_MAX];
} MTK_GLO_ASSIST_ALM_T;

//ABD ALM
typedef struct
{
  UINT8 u1SvId;
  UINT16 u2DayNum;
  UINT32 au4Word[BDS_ALM_WORD_MAX];
} MTK_BDS_ASSIST_ALM_T;

#ifdef SUPPORT_AGA
typedef struct
{
  UINT8 u1SvId;
  UINT16 u2WeekNo;
  UINT8 au1Word[GAL_ALM_WORD_MAX];
} MTK_GAL_ASSIST_ALM_T;
#endif

typedef struct
{
    MTK_GNSS_ID_ENUM  eGnssID;

    union
    {
       MTK_GPS_ASSIST_ALM_T  rAGpsAlm;  /* for GPS alm  */
       MTK_GLO_ASSIST_ALM_T  rAGloAlm;  /* for GLONASS alm  */
       MTK_BDS_ASSIST_ALM_T  rABDAlm;   /* for BDS alm  */
       MTK_GAL_ASSIST_ALM_T  rAGAAlm;     /* for Gallileo */
    } data;
} MTK_GNSS_ASSIST_ALM_T;

/* start for gnss real time integrity */

typedef struct
{
   UINT16  u1SVID;
   UINT8  u1BSignalIDs;  /* identidy the bad signal or signals of a satellite, bit string representation, map to GNSS_SGN_ID_BITMAP_* */
} MTK_GNSS_ASSIST_BSIG_T;

typedef struct
{
    MTK_GNSS_ID_ENUM        eGnssID;
    UINT8                   u1BsvNum;
    MTK_GNSS_ASSIST_BSIG_T  rBsv[MGPSID];
} MTK_GNSS_ASSIST_BSV_T;
/* end for gnss real time integrity */


/* start for gnss acquisition assistance */
typedef struct
{
    MTK_GNSS_ID_ENUM  eGnssID;
    UINT16  u2SvId;         //,GPS:1~32 GLO:65~96
    UINT8   u1SigID;        /* GNSS type, map to GNSS_SGN_ID_VALUE_* */
    UINT8   u1Conf;         /* [0..100]  only for LPP */
    double  dfTod;          /* [0-86399.999] in seconds */
    INT16   i2Dopp0;        /* [-2048..2047], Doppler (0th order term) value for velocity, scale factor 0.5 m/s in th range from -1024 m/s to +1023.5 m/s */
    UINT8   i1Dopp1;        /* [0..63], i1DoppRate, Doppler (1th order term) value for acceleration, scale factor 1/210 m/s^2 in the range from -0.2 m/s^2 to +0.1 m/s^2 */
    UINT8   u1DoppSR;       /* [0..4], defined values: 2.5 m/s, 5 m/s, 10 m/s, 20 m/s, 40 m/s encoded as integer range 0-4 by 2^(-n)*40 m/s, n=0-4 */
    UINT16  u2CodePh;    /* [0..1022], scale factor 2^(-10) ms in the range from 0 to (1-2^(-10)) ms */
    UINT8   u1CodePhInt; /* [0..127], integer codephase, scale factor 1ms */
    UINT8   u1CodePhSR;  /* [0..31], map to value-to-searchwindow table (ms) */
    UINT16  u2Azim;         /* [0..511], azimuth angle a, x-degrees of satellite x<=a<x+0.703125, scale factor 0.703125 degrees */
    UINT8   u1Elev;         /* [0..127], elevation angle e, y-degrees of satellite y>=e<y+0.703125, scale factr 0.703125 degrees */
    /* optional field */
    MTK_GPS_BOOL fgCodePh1023;    /* only use if codePhase is 1022, codePhase value is 1023*2^(-10) = (1-2^(-10)) ms */
    /* if support dopplerUncertaintyExtR10, should ignore dopplerUncertainty field */
    MTK_GNSS_ACQ_DOPP_UNCERT_EXT_ENUM u1DopExtEnum; /* enumerated value map to 60 m/s, 80 m/s, 100 m/s, 120 ms, and No Information */
    UINT8   u1CodeSR;

} MTK_GNSS_ASSIST_ACQ_T;
/* end for gnss acquisition assistance */

/* start for gnss acquisition assistance */
typedef struct
{
   double GPSTOW;
   double dfGNSSTOD;
   MTK_GPS_BOOL AzElIncl;
   MTK_GPS_BOOL UseEph;
   unsigned char NumSV;
   UINT16  SV[24];         //,GPS:1~32 GLO:65~96
   UINT8   u1SigID[24];        /* GNSS type, map to GNSS_SGN_ID_VALUE_* */
   UINT8   u1Conf[24];         /* [0..100]  only for LPP */
   double  dfTod[24];          /* [0-86399.999] in seconds */
   INT16   Dopp[24];        /* [-2048..2047], Doppler (0th order term) value for velocity, scale factor 0.5 m/s in th range from -1024 m/s to +1023.5 m/s */
   UINT8   DoppRate[24];        /* [0..63], i1DoppRate, Doppler (1th order term) value for acceleration, scale factor 1/210 m/s^2 in the range from -0.2 m/s^2 to +0.1 m/s^2 */
   UINT8   DoppSR[24];       /* [0..4], defined values: 2.5 m/s, 5 m/s, 10 m/s, 20 m/s, 40 m/s encoded as integer range 0-4 by 2^(-n)*40 m/s, n=0-4 */
   UINT16  Code_Ph[24];    /* [0..1022], scale factor 2^(-10) ms in the range from 0 to (1-2^(-10)) ms */
   UINT8   Code_Ph_Int[24]; /* [0..127], integer codephase, scale factor 1ms */
   UINT8   CodePhSR[24];  /* [0..31], map to value-to-searchwindow table (ms) */
   UINT8   CodeSR[24];
   UINT16  Azim[24];         /* [0..511], azimuth angle a, x-degrees of satellite x<=a<x+0.703125, scale factor 0.703125 degrees */
   UINT8   Elev[24];         /* [0..127], elevation angle e, y-degrees of satellite y>=e<y+0.703125, scale factr 0.703125 degrees */
   /* optional field */
   MTK_GPS_BOOL fgCodePh1023[24];    /* only use if codePhase is 1022, codePhase value is 1023*2^(-10) = (1-2^(-10)) ms */
   /* if support dopplerUncertaintyExtR10, should ignore dopplerUncertainty field */
   MTK_GNSS_ACQ_DOPP_UNCERT_EXT_ENUM u1DopExtEnum[24]; /* enumerated value map to 60 m/s, 80 m/s, 100 m/s, 120 ms, and No Information */

} s_NA_GLO_AcqAss;

/* start for gnss acquisition assistance */
typedef struct
{
    double dfGNSSTOD;
    MTK_GPS_BOOL AzElIncl;
    MTK_GPS_BOOL UseEph;
    unsigned char NumSV;
    UINT16  SV[24];         //,GPS:1~32 GLO:65~96
    UINT8   u1SigID[24];        /* GNSS type, map to GNSS_SGN_ID_VALUE_* */
    UINT8   u1Conf[24];         /* [0..100]  only for LPP */
    double  dfTod[24];          /* [0-86399.999] in seconds */
    INT16   Dopp[24];        /* [-2048..2047], Doppler (0th order term) value for velocity, scale factor 0.5 m/s in th range from -1024 m/s to +1023.5 m/s */
    UINT8   DoppRate[24];        /* [0..63], i1DoppRate, Doppler (1th order term) value for acceleration, scale factor 1/210 m/s^2 in the range from -0.2 m/s^2 to +0.1 m/s^2 */
    UINT8   DoppSR[24];       /* [0..4], defined values: 2.5 m/s, 5 m/s, 10 m/s, 20 m/s, 40 m/s encoded as integer range 0-4 by 2^(-n)*40 m/s, n=0-4 */
    UINT16  Code_Ph[24];    /* [0..1022], scale factor 2^(-10) ms in the range from 0 to (1-2^(-10)) ms */
    UINT8   Code_Ph_Int[24]; /* [0..127], integer codephase, scale factor 1ms */
    UINT8   CodePhSR[24];  /* [0..31], map to value-to-searchwindow table (ms) */
    UINT8   CodeSR[24];
    UINT16  Azim[24];         /* [0..511], azimuth angle a, x-degrees of satellite x<=a<x+0.703125, scale factor 0.703125 degrees */
    UINT8   Elev[24];         /* [0..127], elevation angle e, y-degrees of satellite y>=e<y+0.703125, scale factr 0.703125 degrees */
    /* optional field */
    MTK_GPS_BOOL fgCodePh1023[24];    /* only use if codePhase is 1022, codePhase value is 1023*2^(-10) = (1-2^(-10)) ms */
    /* if support dopplerUncertaintyExtR10, should ignore dopplerUncertainty field */
    MTK_GNSS_ACQ_DOPP_UNCERT_EXT_ENUM u1DopExtEnum[24]; /* enumerated value map to 60 m/s, 80 m/s, 100 m/s, 120 ms, and No Information */
} MTK_GNSS_ASSIST_ACQ_RAW_DATA_T;
/* end for gnss acquisition assistance */

/* start for gnss acquisition assistance */
typedef struct
{
    UINT16  Code_Ph[24];    /* [0..1022], scale factor 2^(-10) ms in the range from 0 to (1-2^(-10)) ms */
    UINT8   Code_Ph_Int[24]; /* [0..127], integer codephase, scale factor 1ms */
    UINT8 Acq_GPS_Secs;
    MTK_GPS_BOOL AzElIncl;
    MTK_GPS_BOOL UseEph;
    double dfGNSSTOD;
    UINT8   CodeSR[24];
    unsigned char NumSV;
    UINT16  SV[24];         //,GPS:1~32 GLO:65~96
    UINT8   u1SigID;        /* GNSS type, map to GNSS_SGN_ID_VALUE_* */
    UINT8   u1Conf;         /* [0..100]  only for LPP */
    double  dfTod;          /* [0-86399.999] in seconds */
    INT16   Dopp[24];        /* [-2048..2047], Doppler (0th order term) value for velocity, scale factor 0.5 m/s in th range from -1024 m/s to +1023.5 m/s */
    UINT8   DoppRate[24];        /* [0..63], i1DoppRate, Doppler (1th order term) value for acceleration, scale factor 1/210 m/s^2 in the range from -0.2 m/s^2 to +0.1 m/s^2 */
    UINT8   DoppSR[24];       /* [0..4], defined values: 2.5 m/s, 5 m/s, 10 m/s, 20 m/s, 40 m/s encoded as integer range 0-4 by 2^(-n)*40 m/s, n=0-4 */
    UINT16  CodePh[24];    /* [0..1022], scale factor 2^(-10) ms in the range from 0 to (1-2^(-10)) ms */
    UINT8   CodePhInt[24]; /* [0..127], integer codephase, scale factor 1ms */
    UINT8   CodePhSR[24];  /* [0..31], map to value-to-searchwindow table (ms) */
    UINT16  Azim[24];         /* [0..511], azimuth angle a, x-degrees of satellite x<=a<x+0.703125, scale factor 0.703125 degrees */
    UINT8   Elev[24];         /* [0..127], elevation angle e, y-degrees of satellite y>=e<y+0.703125, scale factr 0.703125 degrees */
    /* optional field */
    MTK_GPS_BOOL fgCodePh1023;    /* only use if codePhase is 1022, codePhase value is 1023*2^(-10) = (1-2^(-10)) ms */
    /* if support dopplerUncertaintyExtR10, should ignore dopplerUncertainty field */
    MTK_GNSS_ACQ_DOPP_UNCERT_EXT_ENUM u1DopExtEnum; /* enumerated value map to 60 m/s, 80 m/s, 100 m/s, 120 ms, and No Information */
} MTK_GNSS_ASSIST_ACQ_DATA_T;
/* end for gnss acquisition assistance */

typedef struct
{
   UINT32 Code_whole;
   UINT32 Code_fract;

   UINT16 PRN_num;               //
   INT8 i1ChannelID;
   MTK_GNSS_ID_ENUM eGnssID;       //in-use GNSSID  :
                           //bit 0:gps
                           //bit 1:sbas
                           //bit 2:qzss
                           //bit 3:galileo
                           //bit 4:glonass
                           //bit 5~15:reserved
   UINT8 u2SignalID;       //in-use signal ID  :
                           //now for GPS only support GNSS_SGN_ID_VALUE_GPS_L1C_A
                           //now for GLONASS only support GNSS_SGN_ID_VALUE_GLONASS_G1
                           //now for QZSS only support GNSS_SGN_ID_VALUE_QZSS_L1C_A

   UINT8  SNR;         //[0..63] SNR
   INT16  Dopp;        // [-32768..32767]
   UINT32 u4CodePh;      //[0..2097151] ms 21-bits with 2^-21 resolution
   UINT8  u1CodePhInt;   //[0~127] ms
   UINT8  Mul_Path_Ind;  //[0~3] refer to RRLP table A.9

   UINT8  u1CarryQualInd; //[0~3]
   UINT32 u1Adr;          //[0..33554431] 25-bits with 2^-10 resolution
   UINT8  Range_RMS_Man ;         //Pseudorange RMS Error mantissa
   UINT8  Range_RMS_Exp;         //Pseudorange RMS Error Exponent
}MTK_IS801_SV_DATA_T;

typedef struct
{
   UINT8 Time_Ref;     //I4
   UINT8 TimeRefSrc;   //I4
   UINT8 NumValidMeas; //U4
   UINT32 Time_Tow;
   UINT8  u1GnssTimeID;    /* gnss system time ID:eque to GNSS-ID(,0-GPS,1-SBAS, 2-QZSS,3-Galileo,4-GLONASS, 5~16 reserved) */
   INT16  i2DayNo;        /* [0-] in day */
   UINT32 u4Todms;        /* [0-3599999] in ms */
   UINT16 u2TodFrag;      /* [0-3999] with resolution of 250ns optional */
   UINT32 u4TodUnc ;         /* [0-127] with resolution of 250ns optional */
   UINT8  u1NumValidMeas;
   UINT8  u1NumValidGnss;
   UINT16 u2GnssIDsInUse;     //in-use GNSSID bitmap
   MTK_AGNSS_SIGNALIDS_T  u1SignalIDsInUse;   //in-use GNSSID bitmap :
   UINT8  u1CodePhAmb;     // [0~127 ]the codephase ambiguity in interge ms default set to 0¡£ optional
   MTK_IS801_SV_DATA_T SV_Data[AGNSS_RRLP_MAX_PRM];  // Satellite Pseudorange Measurement Data

}s_API_IS801_GNSS_PRMeas;

/* start for gnss utc model */
/* for GPS UTC model */
/*
typedef struct
{
    INT32  i4UtcA1;         //[-8388608..8388607], scale factor 2^(-50) seconds/second
    INT32  i4UtcA0;         // [-2147483648..2147483647], scale factor 2^(-30) seconds
    UINT8  u1UtcTot;        // [0..255], scale factor 2^12 seconds
    UINT8  u1UtcWNt;        // [0..255], scale factor 1 week
    INT8   i1UtcDeltaTls;   //[-128..127], scale factor 1 second
    UINT8  u1UtcWNlsf;      // [0..255], scale factor 1 week
    INT8   i1UtcDN;         // [-128..127], scale factor 1 day
    INT8   i1UtcDeltaTlsf;  // [-128..127], scale factor 1 second
} MTK_GPS_ASSIST_UCP_T;
*/

/* for QZSS UTC model */
typedef struct
{
    INT16   i2UtcA0;         /* [-32768..32767], bias coefficient of GNSS time scale relative to UTC time scale, scale factor 2^(-35) seconds */
    INT16   i2UtcA1;         /* [-4096..4095], drift coefficient of GNSS time scale relative to UTC time scale, scale factor 2^(-51) seconds/second */
    INT8    i1UtcA2;         /* [-64..63], drift rate correction coefficient of GNSS time sacel relative to UTC time scale, scale factor 2^(-68) seconds/second^2 */
    INT8    i1UtcDeltaTls;   /* [-128..127], current or past leap second count, scale factor 1 second */
    UINT16  u2UtcTot;        /* [0..65535], time data reference time of week, scale factor 2^4 seconds */
    UINT16  u2UtcWNot;       /* [0..8191], time data reference week number, scale factor 1 week */
    UINT8   u1UtcWNlsf;      /* [0..255], leap second reference week number, scale factor 1 week */
    UINT8   u1UtcDN;         /* 4 bits field, leap second reference day number, scale factor 1 day */
    INT8    i1UtcDeltaTlsf;  /* [-128..127], current or future leap second count, scale factor 1 second */
} MTK_QZSS_ASSIST_UCP_T;

/* for GLONASS  UTC model */
typedef struct
{
    UINT16  u2nA;            /* [1..1461], callendar day number within four-year period beginning since the leap year, scale factor 1 day */
    INT32   i4tauC;          /* [-2147483648..2147483647], GLONASS time scale correction to UTC(SU), scale factor 2^(-31) seconds */
    //optional field                      /* mandatory present if GLONASS-M satellites are presnet in the current GLONASS constellation */
    INT16   i2b1;            /* [-1024..1023],default 0.  coefficient to determine delta UT1, scale factor 2^(-10) seconds */
    INT16   i2b2;            /* [-512..511],default 0.      coefficient to determind delta UT1, scale factor 2^(-16) seconds/msd */
    UINT8   u1kp;            /* 2 bits field, default 0.       notification of expected leap second correction */
} MTK_GLO_ASSIST_UCP_T;

/* for SBAS  UTC model */
typedef struct
{
    INT32  i4UtcA1wnt;       /* [-8388608..8388607], scale factor 2^(-50) seconds/second */
    INT32  i4UtcA0wnt;       /* [-2147483648..2147483647], scale factor 2^(-30) seconds */
    UINT8  u1UtcTot;         /* [0..255], scale factor 2^12 seconds */
    UINT8  u1UtcWNt;         /* [0..255], scale factor 1 week */
    INT8   i1UtcDeltaTls;    /* [-128..127], scale factor 1 second */
    UINT8  u1UtcWNlsf;       /* [0..255], scale factor 1 week */
    INT8   i1UtcDN;          /* [-128..127], scale factor 1 day */
    INT8   i1UtcDeltaTlsf;   /* [-128..127], scale factor 1 second */
    INT8   i1UtcStandardID;  /* [0..7], if GNSS-ID indicates SBAS, this field indicated the UTC stadard used for the SBAS network time indicated by SBAS-ID to UTC relation */
} MTK_SBAS_ASSIST_UCP_T;

/* for BDS UTC model */
typedef struct
{
    INT32  i4UtcA0;       /* [-2147483648..2147483647], scale factor 2^(-30) seconds */
    INT32  i4UtcA1;       /*[-8388608..8388607], scale factor 2^(-50) seconds/second */
    INT8   i1UtcDeltaTls;   /* [-128..127], scale factor 1 second */
    UINT8  u1UtcWNlsf;       /* [0..255], scale factor 1 week */
    UINT8  u1UtcDN;          /* [-128..127], scale factor 1 day */
    INT8   i1UtcDeltaTlsf;   /* [-128..127], scale factor 1 second */
} MTK_BDS_ASSIST_UCP_T;

#ifdef SUPPORT_AGA
typedef struct
{
   INT32  i4A1;         // UTC parameter A1 (seconds/second)/(2^-50)
   INT32  i4A0;         // UTC parameter A0 (seconds)/(2^-30)
   UINT32 u4Tot;        // UTC reference time of week (seconds)/(2^12)
   UINT8  u1WNt;        // UTC reference week number (weeks)
   INT8   i1dtLS;       // UTC time difference due to leap seconds before event (seconds)
   UINT8  u1WNLSF;      // UTC week number when next leap second event occurs (weeks)
   UINT8  u1DN;         // UTC day of week when next leap second event occurs (days)
   INT8   i1dtLSF;      // UTC time difference due to leap seconds after event (seconds)
} MTK_GAL_ASSIST_UCP_T;
#endif

typedef struct
{
    MTK_GNSS_UTC_TYPE_ENUM  u1UtcMd;

    union
    {
       MTK_GPS_ASSIST_UCP_T   utcModel1;  /* for GPS */
       MTK_QZSS_ASSIST_UCP_T  utcModel2;  /* for QZSS */
       MTK_GLO_ASSIST_UCP_T   utcModel3;  /* for GLONASS */
       MTK_SBAS_ASSIST_UCP_T utcModel4;   /* for SBAS */
       MTK_BDS_ASSIST_UCP_T utcModel5;    /* for BDS */
       MTK_GAL_ASSIST_UCP_T utcModel6;    /* for GALILEO */
    } data;
} MTK_GNSS_ASSIST_UCP_T;
/* end for gnss utc model */

/* start for gnss data bit assistance */
typedef struct
{
    MTK_GNSS_ID_ENUM  eGnssID;
    UINT16   u2GnssTOD;     /* [0..3599999] milli-second, reference time of the first bit of the data modulo 1 hour, scale factor 1 second */
   // UINT16   u2GnssTODms;   /* [0..999], fractional part of gnssTD, scale factor 1 milli-second */
    UINT16   u2SvId;
    UINT8    u1SigType;
    UINT16   u2DataBitsNum; /* data bit original max is 1024 bits */
    UINT16   u2Num; /* x th PMTK for this SV-signal */
    UINT16   u2NumIndex; /* Index; */
    UINT32   au4Word[16];/* 16Word*4-byte*8-bits = 2^9 =512 bit */
} MTK_GNSS_ASSIST_DBA_T;
/* end for gnss data bit assistance */

/* start for gnss aux info */
typedef struct
{
    UINT8   u1SvId;           // PRN GPS:1~32  GLO:1~24
    UINT8    u1SigAvai;      /* 8 bits field, indicate the ranging signals supported by the satellite indicated by svID */
    /* optional field */
    INT8    i1ChannelId;     /* indicate the GLONASS carrier frequency number of the satellite identified by svID */
                            /* for GLONASS[-7..13]. if there are no this parameter, this para is set "256": 0xff
                                                      for GPS there is no part for this para in PMTK765.So need to handle this para besides GLONASS  */
} MTK_GNSS_ASSIST_AUX_ELE_T;

typedef struct
{
    //MTK_GNSS_AUX_TYPE_ENUM eType;
    MTK_GNSS_ID_ENUM eGnssID;
    UINT8 u1GnssEleNum;
    MTK_GNSS_ASSIST_AUX_ELE_T  rGnssEle[GNSS_MAX_AUX_SAT_ELEMENT];

} MTK_GNSS_ASSIST_AUX_T;
/* end for gnss aux info */
/* for GNSS capbility   */

typedef struct
{
    MTK_GNSS_ID_ENUM eGnssID;
    UINT8            u1SigBitmap;
} MTK_GNSS_SIGNAL_T;

typedef struct
{
    MTK_GPS_BOOL       fgADR;
    MTK_GPS_BOOL       fgFta;
    MTK_GPS_BOOL       fgDgnss;
    UINT8              u1GnssNum;
    MTK_GNSS_SIGNAL_T  GnssSigIDs[GNSS_ID_MAX_NUM];

    MTK_GPS_AGNSS_PRM_SV_DATA_T SV_Data[AGNSS_RRLP_MAX_PRM];
} MTK_AGNSS_DT_CAPBILITY_T;
typedef struct
{

    UINT8    u1GnssTimeID;    /* gnss system time ID:eque to GNSS-ID(,0-GPS,1-SBAS, 2-QZSS,3-Galileo,4-GLONASS, 5~16 reserved) */
    /**
     * This field specifies the sequential number of days from the origin of the GNSS System Time as follows:
     * GPS, QZSS, SBAS  Days from January 6th 1980 00:00:00 UTC(USNO)
     * Galileo ¡V             TBD;
     * GLONASS ¡V         Days from January 1st 1996
     */
    UINT16   u2DN;           /* [0-32767] gnssDayNumber  */
    double   dfTod;          /* [0-86399.999] in seconds */
    double   dfTodRms;       /* K = [0..127], uncertainty r (microseconds) = C*(((1+x)^K)-1), C=0.5, x=0.14 */
    double   dfFTime;        // sec, Frame Time correspond to GNSS TOD
    double   dfFTimeRms;     // ms, Frame Time RMS accuracy
} MTK_GNSS_ASSIST_FTA_T, MTK_GNSS_AGPS_DT_FTIME_T;    // Fine Time Assistance
#endif
//end for AGPS_SUPPORT_GNSS
typedef struct
{
    UINT8    u1Type;                 /* 1: co-clock clock drift ;2:.... */
    UINT32   u4ClkErrRange;          /* ppb need to convert to ppm */
    //INT32   u4ClkErrRange;          /* ppb need to convert to ppm */
}MTK_GPS_ASSIST_FREQ_T;
typedef struct
{

    UINT8    u1FtaType;        /* Frame sync aiding type */
    UINT16   u2WeekNum;        /* [0-32767] gnssWeekNumber  */
    double   dfTow;            /* [0-604799.999] in seconds */
    double   dfTowOffset;      /* [0-999.999] in us */
    UINT8    u1ClkDriftFlag;   // flag of clock drift. 1:
    double   dfClkDrift;       // clock drift +/-20000 ppb (+/-20ppm)
    UINT8    errType;
} MTK_GNSS_ASSIST_FSYNC_T, MTK_GNSS_AGPS_DT_FSYNC_T;    // Fine Time Assistance

typedef struct
{
    UINT16 u2Cmd;  // PMTK command ID:
                       // please get the data arguements in the following corresponding data structure.
    union
    {
        MTK_GPS_AGPS_DT_ACK_T rAck;              // PMTK001
        MTK_GPS_AGPS_CMD_MODE_T rAgpsMode;       // PMTK290
        MTK_GPS_AGPS_DT_REQ_ASSIST_T rReqAssist; // PMTK730
        MTK_GPS_AGPS_DT_LOC_EST_T rLoc;          // PMTK731
        MTK_GPS_AGPS_DT_GPS_MEAS_T rPRM;         // PMTK732
        MTK_GPS_AGPS_DT_LOC_ERR_T rLocErr;       // PMTK733
        MTK_GPS_AGPS_DT_FTIME_T rFTime;          // PMTK734
        MTK_GPS_AGPS_DT_FTIME_ERR_T rFTimeErr;   // PMTK735
        MTK_GPS_AGPS_DT_LOC_EXTRA_T rLocExtra;     // PMTK742/743/744
#if defined(AGPS_SUPPORT_GNSS)
        MTK_AGNSS_DT_REQ_ASSIST_T  rGnssReqAssist; // PMTK760
        MTK_AGNSS_DT_LOC_EST_T     rGnssLoc;       // PMTK761
        MTK_AGNSS_DT_MEAS_T        rGnssPRM;       // PMTK763
        MTK_AGNSS_DT_CAPBILITY_T   rGnssCap;       // PMTK764
#endif
    } uData;
} MTK_GPS_AGPS_RESPONSE_T;



#if defined(AGPS_SUPPORT_GNSS)
typedef struct
{
    UINT8 u1Arg1;  // version of Query location parameters: 1:GNSS format  0:GPS format
    UINT8 u1Arg2;  // if is for early report? 1:early report  0:normal report
    UINT8 u1Arg3;
    UINT8 u1Arg4;
    UINT16 u1Arg5;
} MTK_GNSS_CMD_LOC_T;

typedef struct
{
    UINT8 u1Arg1;  // version of Query measurement parameters: 1:GNSS format  0:GPS format
    UINT8 u1Arg2;  // if is for early report? 1:early report  0:normal report
    UINT8 u1Arg3;  // multi freq measurment req?
    UINT16 u1Arg4; // gnss bitmap
} MTK_GNSS_CMD_MEAS_T;

typedef struct
{
    UINT8 u1Arg1;  // 1: Query parameters using GNSS format  0:GPS format
} MTK_GNSS_CMD_BITMAP_T;

typedef struct
{
    UINT8 u1Arg1;  // 1: Query parameters using GNSS format  0:GPS format
} MTK_GNSS_CMD_CAPB_T;
#endif
typedef struct
{
  UINT16    u2Cmd;  // PMTK command ID: if the PMTK command has data arguments,
                        // please assign the data in the following  corresponding data structure.
  union
  {
    MTK_GPS_AGPS_CMD_MODE_T rMode;              // PMTK290
    MTK_GPS_AGPS_CMD_ACCEPT_MAP_T rAcceptMap;   // PMTK292
    MTK_GPS_AGPS_CMD_QOP_T rQop;                // PMTK293
    MTK_GPS_ASSIST_EPH_T rAEph;             // PMTK710
    MTK_GPS_ASSIST_ALM_T rAAlm;             // PMTK711
    MTK_GPS_ASSIST_TIM_T rATim;             // PMTK712
    MTK_GPS_ASSIST_LOC_T rALoc;             // PMTK713
    MTK_GPS_ASSIST_CLK_T rAClk;             // PMTK714
    MTK_GPS_ASSIST_KLB_T rAKlb;             // PMTK715
    MTK_GPS_ASSIST_UCP_T rAUcp;             // PMTK716
    MTK_GPS_ASSIST_BSV_T rABsv;             // PMTK717
    MTK_GPS_ASSIST_ACQ_T rAAcq;             // PMTK718
    MTK_GPS_ASSIST_FTA_T rAFta;             // PMTK719
    MTK_GPS_ASSIST_DGP_T rARtcm;            // PMTK720
    MTK_GPS_ASSIST_TOW_T rATow;             // PMTK725
    MTK_GPS_AGPS_CMD_MA_LOC_T rAMA_Loc;     // PMTK739
#if defined(AGPS_SUPPORT_GNSS)
    MTK_GNSS_CMD_LOC_T  rLoc;             // PMTK485,1*   PMTK485,0*
    MTK_GNSS_CMD_MEAS_T rMeas;            // PMTK486,1*   PMTK486,0*
    MTK_GNSS_CMD_BITMAP_T rBitMap;        // PMTK487,1*   PMTK487,0*
    MTK_GNSS_CMD_CAPB_T  rCapb;                 // PMTK493,1* PMTK493,0*

    MTK_GNSS_ASSIST_TIM_T  rAGnssTim;           // PMTK752
    MTK_GNSS_ASSIST_TMOD_T rAGnssTmod;          // PMTK753
    MTK_GNSS_ASSIST_ION_T rAGnssIon;            // PMTK754
    MTK_GNSS_ASSIST_UCP_T rAGnssUcp;            // PMTK755
    MTK_GNSS_ASSIST_DBA_T rAGnssDba;            // PMTK756
    MTK_GNSS_ASSIST_BSV_T rAGnssBsv;            // PMTK757
    MTK_GNSS_ASSIST_ACQ_T rAGnssAcq;            // PMTK758
    MTK_GNSS_ASSIST_EOP_T rAGnssEop;            // PMTK759
    MTK_GNSS_ASSIST_AUX_T rAGnssAux;            // PMTK765
    MTK_GNSS_ASSIST_EPH_T rAGnssEph;            // PMTK710
    MTK_GNSS_ASSIST_ALM_T rAGnssAlm;            // PMTK713
    MTK_GNSS_ASSIST_LOC_T rAGnssLoc;            // PMTK712
    MTK_GNSS_ASSIST_FTA_T rAGnssFta;            // PMTK766
#endif
    MTK_GNSS_ASSIST_FSYNC_T rAGnssFsync;        // PMTK768
    MTK_GPS_ASSIST_FREQ_T rAGPSFreq;            // PMTK680
  } uData;
} MTK_GPS_AGPS_CMD_DATA_T;


#if ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000 ) )
// for ADS1.x
#elif ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 400000 ) )
// for RVCT2.x or RVCT3.x
#else
#pragma pack()
#endif

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_get_param
 * DESCRIPTION
 *  Get the current setting of the AGPS Agent
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_get_param (MTK_GPS_PARAM key, void* value, UINT16 srcMod, UINT16 dstMod);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_set_param
 * DESCRIPTION
 *  Change the behavior of the AGPS Agent
 * PARAMETERS
 *  key         [IN]   the configuration needs to be changed
 *  value       [IN]   the new setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_set_param (MTK_GPS_PARAM key, const void* value, UINT16 srcMod, UINT16 dstMod);
INT32
mtk_agps_set_param_with_payload_len (MTK_GPS_PARAM key, const void* value, UINT16 srcMod, UINT16 dstMod, int len);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_agps_disaptcher_callback
 * DESCRIPTION
 *  called by MNL when need send data
 * PARAMETERS
 *  type: msg type, length: payload length, data: payload pointer
 * RETURNS
 *  none
 *****************************************************************************/
INT32
mtk_gps_sys_agps_disaptcher_callback (UINT16 type, UINT16 length, char *data);

#ifdef __cplusplus
   }  /* extern "C" */
#endif

#endif


