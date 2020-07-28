
/****************************************************************
Copyright 2015-2016 Sony Corporation
****************************************************************/

#ifndef __SONY_IMX386_PDAF_LIBRARY_H__
#define __SONY_IMX386_PDAF_LIBRARY_H__

#define D_SONY_PD_LIB_BUILD_VER						("20160224_1820")

#define D_SONY_PD_LIB_INITIALIZATION				(-1)

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

// For PhaseDifference
#define D_SONY_PD_ERROR_VALUE						(-64)

#define D_SONY_PD_LIB_E_OK							(0)
#define ENCWDDON									(2)		// NCW determines Defocus OK/NG by disable this functionality
#define EPDVALERR									(3)		// Input error value of phase difference data
#define EINXSOI										(40)	// XSizeOfImage Input out of range
#define EINYSOI										(41)	// YSizeOfImage Input out of range
#define EINPDAFWX									(42)	// PDAFWindowsX Input out of range
#define EINPDAFWY									(43)	// PDAFWindowsY Input out of range
#define EINSO										(44)	// SlopeOffset Input out of range
#define EINACS										(45)	// AdjCoeffSlope Input out of range
#define EINSOXAK									(46)	// SlopeOffsetXAddressKnot Input out of range
#define EINSOYAK									(47)	// SlopeOffsetYAddressKnot Input out of range
#define EINVALDISCONFJ								(48)	// Invalid of Disable Confidence Judgement
#define EINVALDISIHC								(49)	// Invalid of Disable compensation relation with image height
#define EINDONTPN									(50)	// DefocusOKNGThrPointNum Input out of range
#define EINDONXAK									(51)	// DefocusOKNGXAddressKnot Input out of range
#define EINDONYAK									(52)	// DefocusOKNGYAddressKnot Input out of range
#define EINDOP										(53)	// DensityOfPhasePix Input out of range
#define ELDCL										(80)	// Low DefocusConfidenceLevel

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

typedef struct
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

typedef struct
{
    signed long			Defocus;
    signed char			DefocusConfidence;
    unsigned long		DefocusConfidenceLevel;
    signed long			PhaseDifference;
} SonyPdLibOutputData_t;

// ------- SonyPdLibGetVersion API
#ifdef __cplusplus
extern "C" {
#endif

#if defined __GNUC__
__attribute__ ((visibility ("default"))) void SonyPdLibGetVersion
#elif defined(_DLL)
__declspec( dllexport ) void SonyPdLibGetVersion
#else
extern void SonyPdLibGetVersion
#endif
(
    SonyPdLibVersion_t	*pfa_SonyPdLibVersion
);

// ------- SonyPdLibGetDefocus API
#if defined __GNUC__
__attribute__ ((visibility ("default"))) signed long SonyPdLibGetDefocus
#elif defined(_DLL)
__declspec( dllexport ) signed long SonyPdLibGetDefocus
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
