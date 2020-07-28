
#include <cutils/properties.h>

#include <dlfcn.h> /* dynamic loading shared library */

#include "SonyIMX338PdafLibraryWrapper.h"
#include <stdio.h>
#include <string.h>

extern "C" {
#include "SonyIMX338PdafLibrary.h"
#include "SonyIMX338PdafTransCoord.h"
}

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "SonyIMX338PdafLibraryWrapper"

#include <aaa_log.h>

#define S1714_TO_S3100(x) ((int)(-1)*((x)/(1<<14)))

typedef enum
{
    SENS_MODE0_NORMAL_ALL_PIXEL = 0,
    SENS_MODE1_HDR_ALL_PIXEL,
    SENS_MODE2_NORMAL_V2_BINNING,
    SENS_MODE3_HDR_V2_BINNING,
    SENS_MODE4_NORMAL_V4_BINNING,
    SENS_MODE_NO_PDAF

} IMAGE_SENSOR_MODE;


typedef void(*GetVersion_t)( PdLibVersion_t*);

typedef signed long(*GetDefocus_t)( PdLibInputData_t*, PdLibOutputData_t*);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SonyIMX338PdafLibraryWrapper::SonyIMX338PdafLibraryWrapper()
{
    // Initial parameters
    m_pDefocusArgIn       = NULL;
    m_pTransRectArgIn     = NULL;
    m_pDefocusOKNGThrLine = NULL;
    m_pPDCLDa             = NULL;
    m_u4SlopeOffsetDaNum  = 0;
    m_u8PDArea_XOffset    = 0;
    m_u8PDArea_YOffset    = 0;
    m_u8PDArea_Width      = 0;
    m_u8PDArea_Height     = 0;
    m_u4PDCLDaSz          = 0;
    m_bDmpSetting         = MTRUE;
    m_bInitDn             = MFALSE;
    m_u8DbgEn             = 0;
    m_u8FiexibleWindowEn  = 0;
    m_u8WindowModeSelect  = 3; //Reserved

    // Initial parameters of loading sharing library
    m_Libhandle       = NULL;
    m_pfunGetVersion  = NULL;
    m_pfunGetDefocus  = NULL;
    m_bLdDn           = MFALSE;
    m_u8SensMode      = SENS_MODE_NO_PDAF;

    const char *error = NULL;

    m_Libhandle = dlopen( "libSonyIMX338PdafLibrary.so", RTLD_NOW);
    error = dlerror();
    if( (m_Libhandle!=NULL)&&(error==NULL))
    {
        m_pfunGetVersion = dlsym( m_Libhandle, "PdLibGetVersion");
        error = dlerror();
        if( error==NULL)
        {
            m_pfunGetDefocus = dlsym( m_Libhandle, "PdLibGetDefocus");
            error = dlerror();
            if( error==NULL)
            {
                m_bLdDn = MTRUE;

                //check library version
                PdLibVersion_t ver;
                ((GetVersion_t)(m_pfunGetVersion))( &ver);
                AAA_LOGD("dlopen libSonyIMX338PdafLibrary.so done. ver=%lu.%lu", ver.MajorVersion, ver.MinorVersion);

                if( ver.MajorVersion==0 && ver.MinorVersion==0)
                {
                    AAA_LOGE("!! PDAF cannot work!! Please lincess PD library from Sony FAE");
                }
            }
            else
            {
                AAA_LOGE("dlsym method PdLibGetDefocus fail");
            }
        }
        else
        {
            AAA_LOGE("dlsym method PdLibGetVersion fail");

        }

        // dlopen success, but load lib fail : run dlclose
        if( m_bLdDn==MFALSE)
        {
            dlclose( m_Libhandle);
        }

    }
    else
    {
        AAA_LOGE("dlopen libSonyIMX338PdafLibrary.so fail");
    }

    if( m_bLdDn)
    {
        /*
         * shared library loading pass
         * initial related parameters
         */
        m_pDefocusArgIn       = new PdLibInputData_t;
        m_pDefocusOKNGThrLine = new DefocusOKNGThrLine_t[D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL];
        m_pTransRectArgIn     = new SonyPdLibSensorCoordSetting_t;

        memset( m_pDefocusArgIn,               0, sizeof(PdLibInputData_t));
        memset( m_pDefocusOKNGThrLine,         0, sizeof(DefocusOKNGThrLine_t)*D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL);
        memset( m_pTransRectArgIn,             0, sizeof(SonyPdLibSensorCoordSetting_t));
        memset( m_aData_Slope,                 0, sizeof(signed long)*D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL);
        memset( m_aData_Offset,                0, sizeof(signed long)*D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL);
        memset( m_aXAddress_Knot_Slope_Offset, 0, sizeof(MUINT16)*D_IMX338_SLOPE_OFFSET_BLK_NUM_W);
        memset( m_aYAddress_Knot_Slope_Offset, 0, sizeof(MUINT16)*D_IMX338_SLOPE_OFFSET_BLK_NUM_H);

        DefocusOKNGThrLine_t *pDefocusOKNGThrLine = (DefocusOKNGThrLine_t *)m_pDefocusOKNGThrLine;
        for( unsigned int i=0; i<D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL; i++)
        {
            pDefocusOKNGThrLine[i].PointNum = D_IMX338_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM;
            pDefocusOKNGThrLine[i].p_AnalogGain = new unsigned long [D_IMX338_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM];
            pDefocusOKNGThrLine[i].p_Confidence = new unsigned long [D_IMX338_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM];
        }
    }
    else
    {
        /* shared library loading fail */
        m_Libhandle      = NULL;
        m_pfunGetVersion = NULL;
        m_pfunGetDefocus = NULL;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SonyIMX338PdafLibraryWrapper::~SonyIMX338PdafLibraryWrapper()
{
    if( m_bLdDn)
    {
        const char *error = NULL;

        dlclose( m_Libhandle);
        error = dlerror();

        if( error==NULL)
        {
            AAA_LOGD("dlclose libSonyIMX338PdafLibrary.so done");
        }

        if( m_pDefocusArgIn)
            delete ((PdLibInputData_t *)m_pDefocusArgIn);

        if( m_pTransRectArgIn)
            delete ((SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn);

        if( m_pDefocusOKNGThrLine)
        {
            DefocusOKNGThrLine_t *pDefocusOKNGThrLine = (DefocusOKNGThrLine_t *)m_pDefocusOKNGThrLine;
            for( unsigned int i=0; i<D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL; i++)
            {
                if( pDefocusOKNGThrLine[i].p_AnalogGain)
                    delete pDefocusOKNGThrLine[i].p_AnalogGain;

                if( pDefocusOKNGThrLine[i].p_Confidence)
                    delete pDefocusOKNGThrLine[i].p_Confidence;

                pDefocusOKNGThrLine[i].p_AnalogGain = NULL;
                pDefocusOKNGThrLine[i].p_Confidence = NULL;
            }

            delete pDefocusOKNGThrLine;
        }

        m_Libhandle           = NULL;
        m_pfunGetVersion      = NULL;
        m_pfunGetDefocus      = NULL;
        m_pDefocusArgIn       = NULL;
        m_pDefocusOKNGThrLine = NULL;
        m_pTransRectArgIn     = NULL;
    }
    else
    {
        /* Do nothing*/
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL SonyIMX338PdafLibraryWrapper::DbgDumpSetting()
{
    if( m_bLdDn)
    {
        m_u8DbgEn = property_get_int32("vendor.debug.af_mgr.enable", 0);

        if( m_bDmpSetting)
        {
            PdLibInputData_t *in = (PdLibInputData_t *)m_pDefocusArgIn;

            SonyPdLibSensorCoordSetting_t *pSensorSetting = (SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn;

            AAA_LOGD(" ******** Dump Setting ********");
            AAA_LOGD("D_IMX338_DCC_IN_EEPROM_BLK_NUM_W                  : %d", D_IMX338_DCC_IN_EEPROM_BLK_NUM_W);
            AAA_LOGD("D_IMX338_DCC_IN_EEPROM_BLK_NUM_H                  : %d", D_IMX338_DCC_IN_EEPROM_BLK_NUM_H);
            AAA_LOGD("D_IMX338_DCC_IN_EEPROM_BLK_NUM_TOTAL              : %d", D_IMX338_DCC_IN_EEPROM_BLK_NUM_TOTAL);
            AAA_LOGD("D_IMX338_PD_AREA_OFFSET_X                         : %d", D_IMX338_PD_AREA_OFFSET_X);
            AAA_LOGD("D_IMX338_PD_AREA_OFFSET_Y                         : %d", D_IMX338_PD_AREA_OFFSET_Y);
            AAA_LOGD("D_IMX338_PD_AREA_SIZE_X                           : %d", D_IMX338_PD_AREA_SIZE_X);
            AAA_LOGD("D_IMX338_PD_AREA_SIZE_Y                           : %d", D_IMX338_PD_AREA_SIZE_Y);
            AAA_LOGD("D_IMX338_SLOPE_OFFSET_BLK_NUM_W                   : %d", D_IMX338_SLOPE_OFFSET_BLK_NUM_W);
            AAA_LOGD("D_IMX338_SLOPE_OFFSET_BLK_NUM_H                   : %d", D_IMX338_SLOPE_OFFSET_BLK_NUM_H);
            AAA_LOGD("D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL               : %d", D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL);
            AAA_LOGD("D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W          : %d", D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W);
            AAA_LOGD("D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H          : %d", D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H);
            AAA_LOGD("D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL      : %d", D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL);
            AAA_LOGD("D_IMX338_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM  : %d", D_IMX338_DEFOCUS_OKNG_THR_LINE_ONE_BLK_POINT_NUM);
            AAA_LOGD("D_IMX338_AF_MOTOR_DRV_CONT_CODE_RESOLUTION        : %d", D_IMX338_AF_MOTOR_DRV_CONT_CODE_RESOLUTION);
            AAA_LOGD("D_IMX338_EXTRA_SHIFT_BIT                          : %d", D_IMX338_EXTRA_SHIFT_BIT);
            AAA_LOGD("\n");
            AAA_LOGD(" ******** Dump Register Setting ********");
            AAA_LOGD("img_orientation_h     : %d", pSensorSetting->img_orientation_h      );
            AAA_LOGD("img_orientation_v     : %d", pSensorSetting->img_orientation_v      );
            AAA_LOGD("hdr_mode_en           : %d", pSensorSetting->hdr_mode_en            );
            AAA_LOGD("hdr_reso_redu_h       : %d", pSensorSetting->hdr_reso_redu_h        );
            AAA_LOGD("hdr_reso_redu_v       : %d", pSensorSetting->hdr_reso_redu_v        );
            //analog crop
            AAA_LOGD("x_add_sta             : %d", pSensorSetting->x_add_sta              );
            AAA_LOGD("y_add_sta             : %d", pSensorSetting->y_add_sta              );
            AAA_LOGD("x_add_end             : %d", pSensorSetting->x_add_end              );
            AAA_LOGD("y_add_end             : %d", pSensorSetting->y_add_end              );
            //
            AAA_LOGD("x_out_size            : %d", pSensorSetting->x_out_size             );
            AAA_LOGD("y_out_size            : %d", pSensorSetting->y_out_size             );
            AAA_LOGD("x_evn_inc             : %d", pSensorSetting->x_evn_inc              );
            AAA_LOGD("x_odd_inc             : %d", pSensorSetting->x_odd_inc              );
            AAA_LOGD("y_evn_inc             : %d", pSensorSetting->y_evn_inc              );
            AAA_LOGD("y_odd_inc             : %d", pSensorSetting->y_odd_inc              );
            AAA_LOGD("scale_mode            : %d", pSensorSetting->scale_mode             );
            AAA_LOGD("scale_m               : %d", pSensorSetting->scale_m                );
            AAA_LOGD("dig_crop_x_offset     : %d", pSensorSetting->dig_crop_x_offset      );
            AAA_LOGD("dig_crop_y_offset     : %d", pSensorSetting->dig_crop_y_offset      );
            AAA_LOGD("dig_crop_image_width  : %d", pSensorSetting->dig_crop_image_width   );
            AAA_LOGD("dig_crop_image_height : %d", pSensorSetting->dig_crop_image_height  );
            AAA_LOGD("binning_mode          : %d", pSensorSetting->binning_mode           );
            AAA_LOGD("binning_type_h        : %d", pSensorSetting->binning_type_h         );
            AAA_LOGD("binning_type_v        : %d", pSensorSetting->binning_type_v         );
            AAA_LOGD("force_fdsum           : %d", pSensorSetting->force_fdsum            );
            AAA_LOGD("pd_area_x_offset      : %d", m_u8PDArea_XOffset                     );
            AAA_LOGD("pd_area_y_offset      : %d", m_u8PDArea_YOffset                     );
            AAA_LOGD("pd_area_width         : %d", m_u8PDArea_Width                       );
            AAA_LOGD("pd_area_height        : %d", m_u8PDArea_Height                      );
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ******** Dump Library Setting ********");
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn,"  PhaseDifference : %ld (%.4lf)", in->PhaseDifference, 1.0f*in->PhaseDifference/16.f);
            AAA_LOGD_IF( m_u8DbgEn,"  ConfidenceLevel : %lu \n", in->ConfidenceLevel);
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," **** PDAF window \n");
            AAA_LOGD_IF( m_u8DbgEn,"  XSizeOfImage          : %hu", in->XSizeOfImage);
            AAA_LOGD_IF( m_u8DbgEn,"  YSizeOfImage          : %hu", in->YSizeOfImage);
            AAA_LOGD_IF( m_u8DbgEn,"  XAddressOfWindowStart : %hu", in->XAddressOfWindowStart);
            AAA_LOGD_IF( m_u8DbgEn,"  YAddressOfWindowStart : %hu", in->YAddressOfWindowStart);
            AAA_LOGD_IF( m_u8DbgEn,"  XAddressOfWindowEnd   : %hu", in->XAddressOfWindowEnd);
            AAA_LOGD_IF( m_u8DbgEn,"  YAddressOfWindowEnd   : %hu", in->YAddressOfWindowEnd);
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," **** Slope and offset (defocus vs phase difference)");
            AAA_LOGD_IF( m_u8DbgEn,"  XKnotNumSlopeOffset : %hu", in->XKnotNumSlopeOffset);
            AAA_LOGD_IF( m_u8DbgEn,"  YKnotNumSlopeOffset : %hu", in->YKnotNumSlopeOffset);
            for( unsigned int i=0; i<(in->XKnotNumSlopeOffset * in->YKnotNumSlopeOffset); i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_SlopeData[%3d] : %ld, p_OffsetData[%3d] : %ld", i, in->p_SlopeData[i], i, in->p_OffsetData[i]);
            }
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ** X address of SlopOffset knotsn");
            for( unsigned int i=0; i<in->XKnotNumSlopeOffset; i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_XAddressKnotSlopeOffset[%2u] : %d", i, in->p_XAddressKnotSlopeOffset[i]);
            }
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ** Y address of SlopOffset knots");
            for( unsigned int i=0; i<in->YKnotNumSlopeOffset; i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_YAddressKnotSlopeOffset[%2u] : %d", i, in->p_YAddressKnotSlopeOffset[i]);
            }
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," **** Adjustment coefficient of slope according to image sensor mode");
            AAA_LOGD_IF( m_u8DbgEn,"  AdjCoeffSlope : %ld", in->AdjCoeffSlope);
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ** Set image sensor analog gain");
            AAA_LOGD_IF( m_u8DbgEn,"  ImagerAnalogGain    : %lu", in->ImagerAnalogGain);
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," **** Defocus OK/NG Threshold setting");
            AAA_LOGD_IF( m_u8DbgEn,"  XKnotNumDefocusOKNG : %hu", in->XKnotNumDefocusOKNG);
            AAA_LOGD_IF( m_u8DbgEn,"  YKnotNumDefocusOKNG : %hu", in->YKnotNumDefocusOKNG);
            for( unsigned int i=0; i<(in->XKnotNumDefocusOKNG * in->YKnotNumDefocusOKNG); i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_DefocusOKNGThrLine[%u].PointNum        : %lu", i, in->p_DefocusOKNGThrLine[i].PointNum);

                for( unsigned int j=0; j<in->p_DefocusOKNGThrLine[i].PointNum; j++)
                {
                    AAA_LOGD_IF( m_u8DbgEn,"    p_DefocusOKNGThrLine[%u].p_AnalogGain[%u] : %lu", i, j, in->p_DefocusOKNGThrLine[i].p_AnalogGain[j]);
                    AAA_LOGD_IF( m_u8DbgEn,"    p_DefocusOKNGThrLine[%u].p_Confidence[%u] : %lu", i, j, in->p_DefocusOKNGThrLine[i].p_Confidence[j]);
                }
            }
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ** X address of OK/NG knots");
            for( unsigned int i=0; i<in->XKnotNumDefocusOKNG; i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_XAddressKnotDefocusOKNG[%u] : %u", i, in->p_XAddressKnotDefocusOKNG[i]);
            }

            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," ** Y address of OK/NG knots");
            for( unsigned int i=0; i<in->YKnotNumDefocusOKNG; i++)
            {
                AAA_LOGD_IF( m_u8DbgEn,"    p_YAddressKnotDefocusOKNG[%u] : %u", i, in->p_YAddressKnotDefocusOKNG[i]);
            }
            AAA_LOGD_IF( m_u8DbgEn,"\n");
            AAA_LOGD_IF( m_u8DbgEn," **** Phase Detection Pixel Density");
            AAA_LOGD_IF( m_u8DbgEn,"  DensityOfPhasePix : %lu", in->DensityOfPhasePix);
            AAA_LOGD_IF( m_u8DbgEn,"\n\n");

            // Dumg setting log one time.
            m_bDmpSetting = MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT SonyIMX338PdafLibraryWrapper::GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
    if( m_pfunGetVersion && m_bLdDn)
    {
        PdLibVersion_t ver;

        ((GetVersion_t)(m_pfunGetVersion))( &ver);

        tOutSWVer.MajorVersion = ver.MajorVersion;
        tOutSWVer.MinorVersion = ver.MinorVersion;

        AAA_LOGD( "%s = %d.%d", __FUNCTION__, (int)tOutSWVer.MajorVersion, (int)tOutSWVer.MinorVersion);
    }
    else
    {
        tOutSWVer.MajorVersion = 0;
        tOutSWVer.MinorVersion = 0;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL SonyIMX338PdafLibraryWrapper::Init( SPDProfile_t &iPdProfile)
{
    if( m_bLdDn)
    {
        PdLibInputData_t *in = (PdLibInputData_t *)m_pDefocusArgIn;

        /*
         * Initial parameters
         */
        m_bDmpSetting        = MTRUE;
        m_bInitDn            = MFALSE;
        m_u8FiexibleWindowEn = 0;
        m_u8WindowModeSelect = 3; //Reserved
        memset( in, 0, sizeof(PdLibInputData_t));

        /*
         * Sensor mode is defined as sensor scenario id.
         * The enum value is defined in IHalSensor.h.
         *       enum
         *       {
         *          SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
         *           SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
         *           SENSOR_SCENARIO_ID_NORMAL_VIDEO,
         *           SENSOR_SCENARIO_ID_SLIM_VIDEO1,
         *           SENSOR_SCENARIO_ID_SLIM_VIDEO2,
         *           SENSOR_SCENARIO_ID_CUSTOM1,
         *           SENSOR_SCENARIO_ID_CUSTOM2,
         *           SENSOR_SCENARIO_ID_CUSTOM3,
         *           SENSOR_SCENARIO_ID_CUSTOM4,
         *           SENSOR_SCENARIO_ID_CUSTOM5,
         *           SENSOR_SCENARIO_ID_UNNAMED_START = 0x100,
         *       };
         *
         *
         * AE target mode is defined as what kind of currently HDR mode is.
         * The enum value is defined in ae_param.h.
         *       typedef enum
         *       {
         *           AE_MODE_NORMAL,
         *           AE_MODE_IVHDR_TARGET,
         *           AE_MODE_MVHDR_TARGET,
         *           AE_MODE_ZVHDR_TARGET,
         *           AE_MODE_LE_FIX_TARGET,
         *           AE_MODE_SE_FIX_TARGET
         *       }eAETargetMODE;
         */
        AAA_LOGD("%s sensor_scenario:%d, ae_mode:%d", __FUNCTION__, iPdProfile.i4SensorMode, iPdProfile.AETargetMode);

        if( iPdProfile.uImgXsz==5344 && iPdProfile.uImgYsz==4016)
        {
            /* all pixel mode */
            m_u8SensMode = iPdProfile.AETargetMode==0 ? SENS_MODE0_NORMAL_ALL_PIXEL : SENS_MODE1_HDR_ALL_PIXEL ;
        }
        else if( iPdProfile.uImgXsz==2672 && iPdProfile.uImgYsz==2008)
        {
            /* V2 binning mode */
            m_u8SensMode = iPdProfile.AETargetMode==0 ? SENS_MODE2_NORMAL_V2_BINNING : SENS_MODE3_HDR_V2_BINNING ;
        }
        else if( iPdProfile.uImgXsz==1296 && iPdProfile.uImgYsz==736)
        {
            /* V4 binning mode */
            m_u8SensMode = iPdProfile.AETargetMode==0 ? SENS_MODE4_NORMAL_V4_BINNING : SENS_MODE_NO_PDAF ;
        }
        else
        {
            m_u8SensMode = SENS_MODE_NO_PDAF;
        }


        /*
         * Slope and offset (defocus vs phase difference)
         * Setting parameters are needed to be provided from camera module integrator.
         * In any image sensor mode, address is required to be converted into "all-pixel mode" address before scaling, cropping, mirroring and flipping.
         */
        for( MUINT32 i=0; i < D_IMX338_SLOPE_OFFSET_BLK_NUM_W; i++)
        {
            m_aXAddress_Knot_Slope_Offset[i] = (unsigned short)(D_IMX338_PD_AREA_OFFSET_X + (D_IMX338_PD_AREA_SIZE_X * i) + (D_IMX338_PD_AREA_SIZE_X / 2));
        }

        for( MUINT32 i=0; i < D_IMX338_SLOPE_OFFSET_BLK_NUM_H; i++)
        {
            m_aYAddress_Knot_Slope_Offset[i] = (unsigned short)(D_IMX338_PD_AREA_OFFSET_Y + (D_IMX338_PD_AREA_SIZE_Y * i) + (D_IMX338_PD_AREA_SIZE_Y / 2));
        }

        // Set the number of knots
        in->XKnotNumSlopeOffset = D_IMX338_SLOPE_OFFSET_BLK_NUM_W;
        in->YKnotNumSlopeOffset = D_IMX338_SLOPE_OFFSET_BLK_NUM_H;

        // Set x and y address of slope/offset knots
        in->p_XAddressKnotSlopeOffset = m_aXAddress_Knot_Slope_Offset;
        in->p_YAddressKnotSlopeOffset = m_aYAddress_Knot_Slope_Offset;

        // Set adjustment coefficient of slope according to image sensor mode
        switch( m_u8SensMode)
        {
        case( SENS_MODE1_HDR_ALL_PIXEL):
            in->AdjCoeffSlope = D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE1;
            break;
        case( SENS_MODE2_NORMAL_V2_BINNING):
            in->AdjCoeffSlope = D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE2;
            break;
        case( SENS_MODE3_HDR_V2_BINNING):
            in->AdjCoeffSlope = D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE3;
            break;
        case( SENS_MODE4_NORMAL_V4_BINNING):
            in->AdjCoeffSlope = D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE4;
            break;
        default: // In the case of default, it will set a 0 mode.
            in->AdjCoeffSlope = D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0;
            break;
        }

        /*
         * Defocus OK/NG threshold line.
         * Threshold lines' buffer is located during constract.
         */
        // Set the number of knots
        in->XKnotNumDefocusOKNG = D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_W;
        in->YKnotNumDefocusOKNG = D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_H;

        // Set threshold line
        DefocusOKNGThrLine_t *pDefocusOKNGThrLine = (DefocusOKNGThrLine_t *)m_pDefocusOKNGThrLine;
        for( unsigned int i=0; i<D_IMX338_DEFOCUS_OKNG_THR_LINE_BLK_NUM_TOTAL; i++)
        {
            if( pDefocusOKNGThrLine[i].PointNum==2)
            {
                // In case of threshold values according to confidence levels regardless of image sensor gain.
                pDefocusOKNGThrLine[i].p_AnalogGain[0] = 0;
                pDefocusOKNGThrLine[i].p_AnalogGain[1] = 0x07FFFFFF;
                pDefocusOKNGThrLine[i].p_Confidence[0] = 145;
                pDefocusOKNGThrLine[i].p_Confidence[1] = 145;
            }
            else if( pDefocusOKNGThrLine[i].PointNum==4)
            {
                // If you need to set threshold values with different confidence levels based on different gain.
                // It is OK if you may not covert current gain ( A_gain + D_gain ) as you like.

                // 4 points num with Analog gain.
                // Point 0 : gain 0 ==  0[dB] , threshold confidence level 0 == 150
                // Point 1 : gain 1 ==  6[dB] , threshold confidence level 1 == 160
                // Point 2 : gain 2 == 12[dB] , threshold confidence level 2 == 200
                // Point 3 : gain 3 == 24[dB] , threshold confidence level 3 == 230

                pDefocusOKNGThrLine[i].p_AnalogGain[0] = 0;    //  0[dB]
                pDefocusOKNGThrLine[i].p_AnalogGain[1] = 255;  //  6[dB]
                pDefocusOKNGThrLine[i].p_AnalogGain[2] = 383;  // 12[dB]
                pDefocusOKNGThrLine[i].p_AnalogGain[3] = 480;  // 24[dB]
                pDefocusOKNGThrLine[i].p_Confidence[0] = 150;
                pDefocusOKNGThrLine[i].p_Confidence[1] = 160;
                pDefocusOKNGThrLine[i].p_Confidence[2] = 200;
                pDefocusOKNGThrLine[i].p_Confidence[3] = 230;
            }
            else
            {
                for( unsigned int j=0; j<pDefocusOKNGThrLine[i].PointNum; j++)
                {
                    pDefocusOKNGThrLine[i].p_AnalogGain[j] = (40*j);
                    pDefocusOKNGThrLine[i].p_Confidence[j] = (10*j)+140;
                }
            }
        }
        in->p_DefocusOKNGThrLine = (DefocusOKNGThrLine_t *)m_pDefocusOKNGThrLine;


        // Set x and y address of OK/NG threshold line knots
        MUINT16 knot_size_okng_x = (D_IMX338_ALL_PIXEL_SIZE_W-1) / (in->XKnotNumDefocusOKNG-1);
        MUINT16 knot_size_okng_y = (D_IMX338_ALL_PIXEL_SIZE_H-1) / (in->YKnotNumDefocusOKNG-1);

        for( unsigned int i=0; i<in->XKnotNumDefocusOKNG; i++)
        {
            m_aXAddress_Knot_DefocusOKNG[i] = (unsigned short)(i*knot_size_okng_x);
        }

        for( unsigned int i=0; i<in->YKnotNumDefocusOKNG; i++)
        {
            m_aYAddress_Knot_DefocusOKNG[i] = (unsigned short)(i*knot_size_okng_y);
        }

        in->p_XAddressKnotDefocusOKNG = m_aXAddress_Knot_DefocusOKNG;
        in->p_YAddressKnotDefocusOKNG = m_aYAddress_Knot_DefocusOKNG;


        /*
         * Phase Detection Pixel Density
         * Please input "D_PD_LIB_DENSITY_SENS_MODE0" as a fixed value regardless of image sensor mode.
         */
        switch( m_u8SensMode)
        {
        case( SENS_MODE0_NORMAL_ALL_PIXEL) :
        case( SENS_MODE1_HDR_ALL_PIXEL) :
        case( SENS_MODE2_NORMAL_V2_BINNING) :
        case( SENS_MODE3_HDR_V2_BINNING) :
        case( SENS_MODE4_NORMAL_V4_BINNING) :
        default :
            in->DensityOfPhasePix = D_PD_LIB_DENSITY_SENS_MODE0;
            break;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL SonyIMX338PdafLibraryWrapper::SetCaliData( MUINT32 u4CaliDaSz, MUINT8 *pCaliDaBuf, MUINT32 u4RegAddDaPairSz, MUINT16 *pRegAddDaPairBuf)
{
    MBOOL ret = MFALSE;

    if( m_bLdDn)
    {
        m_u4SlopeOffsetDaNum = D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL;
        memset( m_aData_Slope,  0, sizeof(signed long)*D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL);
        memset( m_aData_Offset, 0, sizeof(signed long)*D_IMX338_SLOPE_OFFSET_BLK_NUM_TOTAL);

        if( pCaliDaBuf!=NULL)
        {
            MUINT32 i=0, j=0, k=0;

            ret = MTRUE;

            // check calibration data size in Byte.
            if( /* normal case */
                (u4CaliDaSz >= (m_u4SlopeOffsetDaNum*sizeof(signed short)))            &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_W ==  D_IMX338_SLOPE_OFFSET_BLK_NUM_W) &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_H ==  D_IMX338_SLOPE_OFFSET_BLK_NUM_H) )
            {
                AAA_LOGD("%s num:%d, sz:%d Byte, directly use DCC", __FUNCTION__, m_u4SlopeOffsetDaNum, u4CaliDaSz);

                for( j=0, k=0; j<D_IMX338_SLOPE_OFFSET_BLK_NUM_H; j++)
                {
                    for( i=0; i<D_IMX338_SLOPE_OFFSET_BLK_NUM_W; i++, k++)
                    {
                        unsigned long idx = j*D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*sizeof(signed short) + (2*i);

                        m_aData_Slope[k]  = 0xffff & ( pCaliDaBuf[idx]<<8 | pCaliDaBuf[idx+1]);
                        m_aData_Offset[k] = 0;
                    }
                }
            }
            else if( /* up-sampling */
                (u4CaliDaSz*4 >= (m_u4SlopeOffsetDaNum*sizeof(signed short)))            &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*2 ==  D_IMX338_SLOPE_OFFSET_BLK_NUM_W) &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_H*2 ==  D_IMX338_SLOPE_OFFSET_BLK_NUM_H) )
            {
                AAA_LOGD("%s num:%d, sz:%d Byte, up-sampling DCC", __FUNCTION__, m_u4SlopeOffsetDaNum, u4CaliDaSz);

                for( j=0, k=0; j<D_IMX338_SLOPE_OFFSET_BLK_NUM_H; j++)
                {
                    for( i=0; i<D_IMX338_SLOPE_OFFSET_BLK_NUM_W; i++, k++)
                    {
                        unsigned long idx = (j/2)*D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*sizeof(signed short) + (2*i/2);

                        m_aData_Slope[k]  = 0xffff & ( pCaliDaBuf[idx]<<8 | pCaliDaBuf[idx+1]);
                        m_aData_Offset[k] = 0;
                    }
                }


            }
            else if( /* down-sampling */
                (u4CaliDaSz >= 4*(m_u4SlopeOffsetDaNum*sizeof(signed short)))           &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_W == 2*D_IMX338_SLOPE_OFFSET_BLK_NUM_W) &&
                (D_IMX338_DCC_IN_EEPROM_BLK_NUM_H == 2*D_IMX338_SLOPE_OFFSET_BLK_NUM_H) )
            {
                AAA_LOGD("%s num:%d, sz:%d Byte, down-sampling DCC", __FUNCTION__, m_u4SlopeOffsetDaNum, u4CaliDaSz);

                for( j=0, k=0; j<D_IMX338_SLOPE_OFFSET_BLK_NUM_H; j++)
                {
                    for( i=0; i<D_IMX338_SLOPE_OFFSET_BLK_NUM_W; i++, k++)
                    {
                        unsigned long idx0 = (2*j  )*D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*sizeof(signed short) + (2*2*i);
                        unsigned long idx1 = (2*j+1)*D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*sizeof(signed short) + (2*2*i);

                        signed long val00 = 0xffff & ( pCaliDaBuf[idx0+0]<<8 | pCaliDaBuf[idx0+1]);
                        signed long val01 = 0xffff & ( pCaliDaBuf[idx0+2]<<8 | pCaliDaBuf[idx0+3]);
                        signed long val10 = 0xffff & ( pCaliDaBuf[idx1+0]<<8 | pCaliDaBuf[idx1+1]);
                        signed long val11 = 0xffff & ( pCaliDaBuf[idx1+2]<<8 | pCaliDaBuf[idx1+3]);

                        m_aData_Slope[k]  = ( val00 + val01 + val10 + val11 ) / 4;
                        m_aData_Offset[k] = 0;
                    }
                }
            }
            else
            {
                ret = MFALSE;
            }
        }

        if( ret==MFALSE)
        {
            AAA_LOGE("[%s] Cali data is not valid : %p sz=%lu(%u) W:%d(%d) H:%d(%d)",
                     __FUNCTION__,
                     pCaliDaBuf,
                     (unsigned long)(m_u4SlopeOffsetDaNum*sizeof(signed short)),
                     u4CaliDaSz,
                     D_IMX338_DCC_IN_EEPROM_BLK_NUM_W,
                     D_IMX338_SLOPE_OFFSET_BLK_NUM_W,
                     D_IMX338_DCC_IN_EEPROM_BLK_NUM_H,
                     D_IMX338_SLOPE_OFFSET_BLK_NUM_H);
        }


        /*
         * Slope and offset (defocus vs phase difference)
         * Setting parameters are needed to be provided from camera module integrator.
         */
        PdLibInputData_t *in = (PdLibInputData_t *)m_pDefocusArgIn;

        // Set slope
        in->p_SlopeData = m_aData_Slope;

        // Set offset
        in->p_OffsetData = m_aData_Offset;


        /*
         * extract parameter data for converting address from image sensor register data
         */
        if( ret)
        {
            MUINT8 reg_addr_0x3150=0, reg_addr_0x3151=0, reg_addr_0x3152=0, reg_addr_0x3153=0;
            MUINT8 reg_addr_0x3154=0, reg_addr_0x3155=0, reg_addr_0x3156=0, reg_addr_0x3157=0;

            /*
             * u4RegAddDaPairSz : Buffer size of address and data pair buffer in BYTE.
             * pairsz : Number of address and data pair (one pair in 4 BYTE(Address:2BYTE, Data:2BYTE))
             */
            MUINT32 pairsz = u4RegAddDaPairSz/sizeof(MUINT16)/2;
            SonyPdLibSensorCoordRegData_t RegData;
            memset( &RegData, 0, sizeof(SonyPdLibSensorCoordRegData_t));
            for( MUINT32 i=0; i<pairsz; i++)
            {
                MUINT32 idx = 2*i;
                switch( pRegAddDaPairBuf[idx])
                {
                case (0x0101) :
                    RegData.reg_addr_0x0101 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0220) :
                    RegData.reg_addr_0x0220 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0221) :
                    RegData.reg_addr_0x0221 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0344) :
                    RegData.reg_addr_0x0344 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0345) :
                    RegData.reg_addr_0x0345 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0346) :
                    RegData.reg_addr_0x0346 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0347) :
                    RegData.reg_addr_0x0347 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0348) :
                    RegData.reg_addr_0x0348 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0349) :
                    RegData.reg_addr_0x0349 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034a) :
                    RegData.reg_addr_0x034A = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034b) :
                    RegData.reg_addr_0x034B = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034c) :
                    RegData.reg_addr_0x034C = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034d) :
                    RegData.reg_addr_0x034D = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034e) :
                    RegData.reg_addr_0x034E = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x034f) :
                    RegData.reg_addr_0x034F = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0381) :
                    RegData.reg_addr_0x0381 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0383) :
                    RegData.reg_addr_0x0383 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0385) :
                    RegData.reg_addr_0x0385 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0387) :
                    RegData.reg_addr_0x0387 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0401) :
                    RegData.reg_addr_0x0401 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0404) :
                    RegData.reg_addr_0x0404 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0405) :
                    RegData.reg_addr_0x0405 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0408) :
                    RegData.reg_addr_0x0408 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0409) :
                    RegData.reg_addr_0x0409 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040a) :
                    RegData.reg_addr_0x040A = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040b) :
                    RegData.reg_addr_0x040B = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040c) :
                    RegData.reg_addr_0x040C = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040d) :
                    RegData.reg_addr_0x040D = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040e) :
                    RegData.reg_addr_0x040E = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x040f) :
                    RegData.reg_addr_0x040F = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0900) :
                    RegData.reg_addr_0x0900 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x0901) :
                    RegData.reg_addr_0x0901 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x300d) :
                    RegData.reg_addr_0x300D = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3150) :
                    reg_addr_0x3150 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3151) :
                    reg_addr_0x3151 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3152) :
                    reg_addr_0x3152 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3153) :
                    reg_addr_0x3153 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3154) :
                    reg_addr_0x3154 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3155) :
                    reg_addr_0x3155 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3156) :
                    reg_addr_0x3156 = pRegAddDaPairBuf[idx+1];
                    break;
                case (0x3157) :
                    reg_addr_0x3157 = pRegAddDaPairBuf[idx+1];
                    break;
                default :
                    break;
                }
            }

            m_u8PDArea_XOffset = ((reg_addr_0x3150&0x1F)<<8) | reg_addr_0x3151;
            m_u8PDArea_YOffset = ((reg_addr_0x3152&0x1F)<<8) | reg_addr_0x3153;
            m_u8PDArea_Width   = ((reg_addr_0x3154&0x1F)<<8) | reg_addr_0x3155;
            m_u8PDArea_Height  = ((reg_addr_0x3156&0x1F)<<8) | reg_addr_0x3157;

            if( SonyPdLibInterpretRegData( &RegData, (SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn)!=D_SONY_PD_LIB_REGDATA_IS_OK)
            {
                for( MUINT32 i=0; i<pairsz; i++)
                {
                    MUINT32 idx = 2*i;
                    AAA_LOGE("addr:%x da:%x", pRegAddDaPairBuf[idx], pRegAddDaPairBuf[idx+1]);
                }
                ret = MFALSE;
            }
        }
    }

    m_bInitDn = ret;

    if( !m_bInitDn)
    {
        AAA_LOGE("%s", __FUNCTION__);
    }
    return ret;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL SonyIMX338PdafLibraryWrapper::SetPDCLData( MUINT32 u4PDCLDaSz, MUINT8 *pPDCLDaBuf, MINT32 &oWinMode, MUINT32 &oWinEn)
{
    MBOOL ret = MFALSE;

    if( (u4PDCLDaSz>=965) && (pPDCLDaBuf!=NULL))
    {
        /*
         * Sensor output data size is {16*12*5(PD/CL data)+5(Header)}=965 Bytes, no matter what kind of sensor setting is.
         */
        m_pPDCLDa    = pPDCLDaBuf;
        m_u4PDCLDaSz = u4PDCLDaSz;

        if( (m_u8FiexibleWindowEn!=m_pPDCLDa[0]) || (m_u8WindowModeSelect!=((m_pPDCLDa[1]>>6)&0x3)))
        {
            m_u8FiexibleWindowEn =  m_pPDCLDa[0];
            m_u8WindowModeSelect = (m_pPDCLDa[1]>>6) & 0x3; // 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Fiexible window, 3:Reserved.
        }

        oWinMode = m_u8WindowModeSelect;
        oWinEn   = m_u8FiexibleWindowEn;

        ret = MTRUE;
    }
    else
    {
        AAA_LOGE("[%s] PDCL data is not valid : %p sz=%d", __FUNCTION__, pPDCLDaBuf, u4PDCLDaSz);

        /*
         * error.
         * reset parameters.
         */
        m_pPDCLDa    = NULL;
        m_u4PDCLDaSz = 0;
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL SonyIMX338PdafLibraryWrapper::GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData)
{
    memset( &oPdOutputData, 0, sizeof(SPDROIResult_T));

    if( m_pfunGetDefocus && m_bLdDn && m_bInitDn)
    {
        PdLibInputData_t  *in = (PdLibInputData_t *)m_pDefocusArgIn;

        /*
         * Phase different and confidence level data which is read out from data type 0x36.
         * First {       5 (        Header) +
         *         16*12*5 (PD and CL data) } =965 Bytes are vaild, no matter what AREA_MODE(0x31B0) is configured as.
         *
         * Format :
         *          Phase difference : s5.4
         *          Confidence level : u8.0
         */
        MUINT8 idxx=0, idxy=0;
        MBOOL  setting_vaild = MFALSE;
        if( (m_u8WindowModeSelect==0x0) || (m_u8WindowModeSelect==0x1))
        {
            // Host configure PD window mode as fixed window setting
            // m_u8WindowModeSelect : 0(16x12), 1(8x6)
            if( iPDInputData.mode==0x0)
            {
                MUINT32 roi_center_x = iPDInputData.ROI.sPDROI.i4X + iPDInputData.ROI.sPDROI.i4W/2;
                MUINT32 roi_center_y = iPDInputData.ROI.sPDROI.i4Y + iPDInputData.ROI.sPDROI.i4H/2;

                roi_center_x = (m_u8PDArea_XOffset<roi_center_x) ? (roi_center_x-m_u8PDArea_XOffset) : roi_center_x;
                roi_center_y = (m_u8PDArea_YOffset<roi_center_y) ? (roi_center_y-m_u8PDArea_YOffset) : roi_center_y;

                idxx = (MUINT8)(roi_center_x/m_u8PDArea_Width);
                idxy = (MUINT8)(roi_center_y/m_u8PDArea_Height);

                setting_vaild = MTRUE;
            }
        }
        else if( m_u8WindowModeSelect==0x2) // Sensor is configured as fiexible window mode
        {
            if( /* Host configure PD window mode as fiexible window setting */
                ( (0x0<iPDInputData.mode) && (iPDInputData.mode<=0xFF)    ) && // Checking host controlling mode
                ( iPDInputData.mode==m_u8WindowModeSelect                 ) && // Checking fiexible window flag is valid
                ( 0<=iPDInputData.cfgInfo && iPDInputData.cfgInfo<=7))   // Checking PD and CL data index
            {
                idxx = iPDInputData.cfgInfo;
                idxy = 0;

                setting_vaild = MTRUE;
            }
        }

        //
        if( setting_vaild==MFALSE)
        {
            AAA_LOGD( "Setting is not vaild, skip defocus flow. AREA_MODE(0x%x), FLEXIBLE_WIN_EN(0x%x), HOST_AREA_MODE(0x%x), AREA_INFO(0x%x)",
                      m_u8WindowModeSelect,
                      m_u8FiexibleWindowEn,
                      iPDInputData.mode,
                      iPDInputData.cfgInfo);

            return 0;
        }

        //
        MUINT8 *ptrPDCLDa = &m_pPDCLDa[5];
        MUINT16 idx = 5*(idxy*16+idxx);
        MUINT16 CL  = 0x0FF & ( ptrPDCLDa[idx]);
        MUINT16 PD  = 0x3FF & ( ((ptrPDCLDa[idx+1]<<2)&0x3FC) | ((ptrPDCLDa[idx+2]>>6)&0x3));

        // Phase difference data and confidence level
        in->PhaseDifference = (signed long) (PD>=512 ? PD-1024 : PD); // s5.4

        // Confidence level from image sensor.
        in->ConfidenceLevel = (unsigned long) (CL);   //u8.0


        /*
         * Convert address for PDAF Library
         * In any image sensor mode, address is required to be converted into all-pixel mode address before scaling, cropping, mirroring and flipping.
         */
        SonyPdLibRect_t ImgSz_I, ImgSz_O, WinAddr_I, WinAddr_O;

        ImgSz_I.sta.x = 0;
        ImgSz_I.sta.y = 0;
        ImgSz_I.end.x = ( iPDInputData.XSizeOfImage - ImgSz_I.sta.x - 1);
        ImgSz_I.end.y = ( iPDInputData.YSizeOfImage - ImgSz_I.sta.y - 1);
        ImgSz_O = SonyPdLibTransOutputRectToPdafRect( ImgSz_I, (SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn);

        if( m_u8WindowModeSelect==0x2)
        {
            AAA_LOGD_IF( m_u8DbgEn, "fiexible window mode : using configured window directly");
            // fiexible window mode : using configured window directly
            WinAddr_I.sta.x = iPDInputData.ROI.sPDROI.i4X;
            WinAddr_I.sta.y = iPDInputData.ROI.sPDROI.i4Y;
            WinAddr_I.end.x = iPDInputData.ROI.sPDROI.i4X + iPDInputData.ROI.sPDROI.i4W;
            WinAddr_I.end.y = iPDInputData.ROI.sPDROI.i4Y + iPDInputData.ROI.sPDROI.i4H;
            WinAddr_O = SonyPdLibTransOutputRectToPdafRect( WinAddr_I, (SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn);
        }
        else
        {
            // Fix window mode
            WinAddr_I.sta.x = m_u8PDArea_XOffset + idxx*m_u8PDArea_Width;
            WinAddr_I.sta.y = m_u8PDArea_YOffset + idxy*m_u8PDArea_Height;
            WinAddr_I.end.x = m_u8PDArea_XOffset + (idxx+1)*m_u8PDArea_Width;
            WinAddr_I.end.y = m_u8PDArea_YOffset + (idxy+1)*m_u8PDArea_Height;
            WinAddr_O = SonyPdLibTransOutputRectToPdafRect( WinAddr_I, (SonyPdLibSensorCoordSetting_t *)m_pTransRectArgIn);
        }


        /*
         * PDAF window
         *
         * Address is required to be converted into all-pixel mode address
         * before scaling, cropping, mirroring and flipping
         *
         * PDAF window information must be in synchronization with phase difference data and confidence level
         */
        // Set size
        if( ImgSz_O.end.x < ImgSz_O.sta.x)
        {
            signed int x  = ImgSz_O.end.x;
            ImgSz_O.end.x = ImgSz_O.sta.x;
            ImgSz_O.sta.x = x;
        }
        if( ImgSz_O.end.y < ImgSz_O.sta.y)
        {
            signed int y  = ImgSz_O.end.y;
            ImgSz_O.end.y = ImgSz_O.sta.y;
            ImgSz_O.sta.y = y;
        }
        in->XSizeOfImage = ImgSz_O.end.x - ImgSz_O.sta.x + 1;
        in->YSizeOfImage = ImgSz_O.end.y - ImgSz_O.sta.y + 1;
        AAA_LOGD_IF( m_u8DbgEn,
                     " ImgSz:(%d, %d)->(%d, %d)",
                     iPDInputData.XSizeOfImage,
                     iPDInputData.YSizeOfImage,
                     in->XSizeOfImage,
                     in->YSizeOfImage);

        // Set PDAF window
        AAA_LOGD_IF( m_u8DbgEn,
                     " ROI Address sta:(%d, %d)->(%d, %d), end:(%d, %d)->(%d, %d)",
                     WinAddr_I.sta.x,
                     WinAddr_I.sta.y,
                     WinAddr_O.sta.x,
                     WinAddr_O.sta.y,
                     WinAddr_I.end.x,
                     WinAddr_I.end.y,
                     WinAddr_O.end.x,
                     WinAddr_O.end.y);

        if( WinAddr_O.end.x < WinAddr_O.sta.x)
        {
            signed int x    = WinAddr_O.end.x;
            WinAddr_O.end.x = WinAddr_O.sta.x;
            WinAddr_O.sta.x = x;
        }
        if( WinAddr_O.end.y < WinAddr_O.sta.y)
        {
            signed int y    = WinAddr_O.end.y;
            WinAddr_O.end.y = WinAddr_O.sta.y;
            WinAddr_O.sta.y = y;
        }
        in->XAddressOfWindowStart = WinAddr_O.sta.x;
        in->YAddressOfWindowStart = WinAddr_O.sta.y;
        in->XAddressOfWindowEnd   = WinAddr_O.end.x;
        in->YAddressOfWindowEnd   = WinAddr_O.end.y;


        /*
         * Defocus OK/NG
         * Set image sensor analog gain
         * which must be in synchronization with phase difference data and confidence level.
         */
        in->ImagerAnalogGain = iPDInputData.afegain;

        /*
         * Dump library input setting one time.
         */
        DbgDumpSetting();

        /*
         * Execute
         */
        PdLibOutputData_t  out;
        memset( &out, 0, sizeof(PdLibOutputData_t));
        signed long err = ((GetDefocus_t)(m_pfunGetDefocus))( &in[0], &out);

        /*
         * Output to PD manager
         */
        if( err==D_PD_LIB_E_OK)
        {
            long defocus_14bit_shift = S1714_TO_S3100(out.Defocus);
            long defocus_extra_shift;
            int  extra_shift_bit = (D_IMX338_EXTRA_SHIFT_BIT)<0 ? (-1)*(D_IMX338_EXTRA_SHIFT_BIT) : (D_IMX338_EXTRA_SHIFT_BIT);

            if( (D_IMX338_EXTRA_SHIFT_BIT)<0)
            {
                defocus_extra_shift = defocus_14bit_shift << extra_shift_bit;
            }
            else
            {
                defocus_extra_shift = defocus_14bit_shift >> extra_shift_bit;
            }

            AAA_LOGD_IF( m_u8DbgEn, " Lib Output : ");
            AAA_LOGD_IF( m_u8DbgEn, "   Defocus                : %d",                    (int)out.Defocus);
            AAA_LOGD_IF( m_u8DbgEn, "   Defocus [14bit Shift]  : %d",                    (int)defocus_14bit_shift);
            AAA_LOGD_IF( m_u8DbgEn, "   Defocus [Extra Shift]  : %d (%dbit %s shifted)", (int)defocus_extra_shift, extra_shift_bit, (D_IMX338_EXTRA_SHIFT_BIT)<0 ? "left" : "right");
            AAA_LOGD_IF( m_u8DbgEn, "   DefocusConfidence      : %d",                    (int)out.DefocusConfidence);
            AAA_LOGD_IF( m_u8DbgEn, "   DefocusConfidenceLevel : %d",                    (int)out.DefocusConfidenceLevel);
            AAA_LOGD_IF( m_u8DbgEn, "   PhaseDifference        : %d (%.4lf)",            (int)out.PhaseDifference, 1.0f*in->PhaseDifference/16.f);

            //(1) normalize to fit API spec : output target position.
            oPdOutputData.DesLensPos      = (int)(iPDInputData.curLensPos + defocus_extra_shift);
            //(2) normalize to fit API spec : confidence level is in ragne 0~100.
            oPdOutputData.Confidence      = (int)(in->ConfidenceLevel*100/255);
            //(3) normalize to fit API spec : confidence level is in ragne 0~100.
            oPdOutputData.ConfidenceLevel = (int)(in->ConfidenceLevel*100/255);
            //(4) normalize to fit API spec : pixel base.
            oPdOutputData.PhaseDifference        = in->PhaseDifference*1000/16;
        }
        else
        {
            oPdOutputData.DesLensPos      = 0;
            oPdOutputData.Confidence      = 0;
            oPdOutputData.ConfidenceLevel = 0;
            oPdOutputData.PhaseDifference        = 0;
            AAA_LOGE(" * failed to SonyPdLibGetDefocus *");
        }


        AAA_LOGD_IF( m_u8DbgEn,
                     "ImgSz:(%4d,%4d) ROI:{(%4d,%4d),(%4d,%4d)} idx:(%2d,%2d) P:%4d C:%3d G:%4d, Res={Pos:%4d->%4d C:%3d CL:%3d PD:%5d}",
                     (int)in->XSizeOfImage,
                     (int)in->YSizeOfImage,
                     (int)in->XAddressOfWindowStart,
                     (int)in->YAddressOfWindowStart,
                     (int)in->XAddressOfWindowEnd,
                     (int)in->YAddressOfWindowEnd,
                     (int)idxx,
                     (int)idxy,
                     (int)in->PhaseDifference,
                     (int)in->ConfidenceLevel,
                     (int)in->ImagerAnalogGain,
                     (int)iPDInputData.curLensPos,
                     (int)oPdOutputData.DesLensPos,
                     (int)oPdOutputData.Confidence,
                     (int)oPdOutputData.ConfidenceLevel,
                     (int)oPdOutputData.PhaseDifference);
    }
    return 0;
}


