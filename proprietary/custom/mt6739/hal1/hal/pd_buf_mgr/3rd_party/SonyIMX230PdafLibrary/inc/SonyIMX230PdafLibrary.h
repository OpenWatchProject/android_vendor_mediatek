
/****************************************************************
Copyright 2014 Sony Corporation
****************************************************************/

#ifndef __SONY_IMX230_PDAF_LIBRARY_H__
#define __SONY_IMX230_PDAF_LIBRARY_H__

#define D_SONY_PD_LIB_BUILD_VER						("140715a")


// For DensityOfPhasePix
#define D_SONY_PD_LIB_DENSITY_SENS_MODE0			(2304)
#define D_SONY_PD_LIB_DENSITY_SENS_MODE1			(2304)
#define D_SONY_PD_LIB_DENSITY_SENS_MODE2			(2304/2)
#define D_SONY_PD_LIB_DENSITY_SENS_MODE3			(2304/2)
#define D_SONY_PD_LIB_DENSITY_SENS_MODE4			(2304/4)

// For AdjCoeffSlope
#define D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0	(2304)
#define D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE1	(2304)
#define D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE2	(2304)
#define D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE3	(2304)
#define D_SONY_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE4	(2304)

#define D_SONY_PD_LIB_E_OK							(0)
#define D_SONY_PD_LIB_E_NG							(-1)

typedef struct
{
    unsigned long		MajorVersion;
    unsigned long		MinorVersion;
} SonyPdLibVersion_t;

typedef struct
{
    unsigned long		PointNum;
    unsigned long		*p_AnalogGain;
    unsigned long		*p_Confidence;
} DefocusOKNGThrLine_t;

typedef struct _SonyPdLibInputData_t
{
    signed long			PhaseDifference;
    unsigned long		ConfidenceLevel;
    unsigned short		XSizeOfImage;
    unsigned short		YSizeOfImage;
    unsigned short		XAddressOfWindowStart;
    unsigned short		YAddressOfWindowStart;
    unsigned short		XAddressOfWindowEnd;
    unsigned short		YAddressOfWindowEnd;
    unsigned short		XKnotNumSlopeOffset;
    unsigned short		YKnotNumSlopeOffset;
    signed long			*p_SlopeData;
    signed long			*p_OffsetData;
    unsigned short		*p_XAddressKnotSlopeOffset;
    unsigned short		*p_YAddressKnotSlopeOffset;
    signed long			AdjCoeffSlope;
    unsigned long		ImagerAnalogGain;
    unsigned short		XKnotNumDefocusOKNG;
    unsigned short		YKnotNumDefocusOKNG;
    DefocusOKNGThrLine_t	*p_DefocusOKNGThrLine;
    unsigned short		*p_XAddressKnotDefocusOKNG;
    unsigned short		*p_YAddressKnotDefocusOKNG;
    unsigned long		DensityOfPhasePix;
} SonyPdLibInputData_t;

typedef struct _SonyPdLibOutputData_t
{
    signed long			Defocus;
    signed char			DefocusConfidence;
    unsigned long		DefocusConfidenceLevel;
    signed long			PhaseDifference;
} SonyPdLibOutputData_t;

#ifdef __cplusplus
extern "C" {
#endif

#if defined __GNUC__
__attribute__ ((visibility ("default"))) void SonyPdLibGetVersion
#else
extern void SonyPdLibGetVersion
#endif
(
    SonyPdLibVersion_t	*pfa_SonyPdLibVersion
);

#if defined __GNUC__
__attribute__ ((visibility ("default"))) void SonyPdLibGetImagerRegDataOnPdaf
#else
extern void SonyPdLibGetImagerRegDataOnPdaf
#endif
(
    unsigned long		fa_SetTiming,
    unsigned long		*pfa_DataNum,
    unsigned long		*pfa_Address,
    unsigned long		*pfa_Data
);

#if defined __GNUC__
__attribute__ ((visibility ("default"))) signed long SonyPdLibGetDefocus
#else
extern signed long SonyPdLibGetDefocus
#endif
(
    SonyPdLibInputData_t	*pfa_SonyPdLibInputData,
    SonyPdLibOutputData_t	*pfa_SonyPdLibOutputData
);

#ifdef __cplusplus
}
#endif			/* __cplusplus */

#endif
