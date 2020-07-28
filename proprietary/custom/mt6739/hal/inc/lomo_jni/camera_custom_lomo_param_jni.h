#ifndef __CAMERA_CUSTOM_LOMO_PARAM_JNI_H__
#define __CAMERA_CUSTOM_LOMO_PARAM_JNI_H__

#include <stddef.h>
#include "MediaTypes.h"
#include "ispif.h"
#include "camera_custom_lomo_param.h"

using namespace NSIspTuning;
#define NASHVILLE_GGM_BR_JNI     "Nashville_gamma_BR_jni.h"
#define NASHVILLE_GGM_G_JNI       "Nashville_gamma_G_jni.h"
#define HEFE_GGM_BR_JNI               "Hefe_gamma_BR_jni.h"
#define HEFE_GGM_G_JNI                "Hefe_gamma_G_jni.h"
#define VALENCIA_GGM_BR_JNI       "Valencia_gamma_BR_jni.h"
#define VALENCIA_GGM_G_JNI         "Valencia_gamma_G_jni.h"
#define XPROII_GGM_BR_JNI            "XproII_gamma_BR_jni.h"
#define XPROII_GGM_G_JNI             "XproII_gamma_G_jni.h"
#define LOFI_GGM_BR_JNI               "Lofi_gamma_BR_jni.h"
#define LOFI_GGM_G_JNI                 "Lofi_gamma_G_jni.h"
#define SIERRA_GGM_BR_JNI           "Sierra_gamma_BR_jni.h"
#define SIERRA_GGM_G_JNI            "Sierra_gamma_G_jni.h"
#define KELVIN_GGM_BR_JNI           "Kelvin_gamma_BR_jni.h"
#define KELVIN_GGM_G_JNI             "Kelvin_gamma_G_jni.h"
#define WALDEN_GGM_BR_JNI         "walden_gamma_BR_jni.h"
#define WALDEN_GGM_G_JNI           "walden_gamma_G_jni.h"
#define F1977_GGM_BR_JNI             "F1977_gamma_BR_jni.h"
#define F1977_GGM_G_JNI              "F1977_gamma_G_jni.h"
#define LINEAR_GGM_BR_JNI             "Linear_gamma_BR_jni.h"
#define LINEAR_GGM_G_JNI              "Linear_gamma_G_jni.h"
                
MUINT32 LomoFilterGGMJni[LOMO_TYPE_NUM][CUSTOM_LOMO_GGM_CHANNEL_NUM][CUSTOM_LOMO_GGM_GAIN_NUM]=//const UINT32 LomoFilterGGMJni[LOMO_TYPE_NUM][CUSTOM_LOMO_GGM_CHANNEL_NUM][CUSTOM_LOMO_GGM_GAIN_NUM]=
{
   
    //Normal	G2C_SHADE_EN = 0;
        #include LINEAR_GGM_BR_JNI
        #include LINEAR_GGM_G_JNI
        
    //Nashville	G2C_SHADE_EN = 1;
        #include NASHVILLE_GGM_BR_JNI
        #include NASHVILLE_GGM_G_JNI
        
    //Hefe	G2C_SHADE_EN = 1;
        #include HEFE_GGM_BR_JNI
        #include HEFE_GGM_G_JNI
        
    //Valencia	G2C_SHADE_EN = 1;
        #include VALENCIA_GGM_BR_JNI
        #include VALENCIA_GGM_G_JNI
        
    //XproII	G2C_SHADE_EN = 1;
        #include XPROII_GGM_BR_JNI
        #include XPROII_GGM_G_JNI
        
    //Lofi	G2C_SHADE_EN = 1;
        #include LOFI_GGM_BR_JNI
        #include LOFI_GGM_G_JNI
        
    //Sierra	G2C_SHADE_EN = 1;
        #include SIERRA_GGM_BR_JNI
        #include SIERRA_GGM_G_JNI
        
    //Kelvin	G2C_SHADE_EN = 1;
        #include KELVIN_GGM_BR_JNI
        #include KELVIN_GGM_G_JNI
        
    //Walden	G2C_SHADE_EN = 1;
        #include WALDEN_GGM_BR_JNI
        #include WALDEN_GGM_G_JNI
        
    //F1977	G2C_SHADE_EN = 1;
        #include F1977_GGM_BR_JNI
        #include F1977_GGM_G_JNI
    
};

#endif
