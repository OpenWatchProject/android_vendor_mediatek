/*********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "n3d_hal_kernel.h"
#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>
#include <fstream>
#include <iomanip>

#define IS_DUMP_ENALED ( __isDump && NULL != __dumpHint )

void
N3D_HAL_KERNEL::setDumpConfig(bool dump, TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint)
{
    __isDump = dump;
    __dumpHint = dumpHint;
}

void
N3D_HAL_KERNEL::dumpN3DInitConfig()
{
    if( !IS_DUMP_ENALED ) {
        return;
    }

    const size_t DUMP_PATH_SIZE = 128;
    char dumpPath[DUMP_PATH_SIZE];
    genFileName_VSDOF_BUFFER(dumpPath, DUMP_PATH_SIZE, __dumpHint, "N3D_INIT_INFO.json");
    FILE *fp = fopen(dumpPath, "wb");
    if(NULL == fp) {
        MY_LOGE("Cannot dump N3D init info to %s, error: %s", dumpPath, strerror(errno));
        return;
    }

    if(0 == __initInfoJSON.size()) {
        json initInfoJson;

        initInfoJson["scenario"] = __algoInitInfo.scenario;

        //iio_main
        initInfoJson["iio_main"]["inp_w"] = __algoInitInfo.iio_main.inp_w;
        initInfoJson["iio_main"]["inp_h"] = __algoInitInfo.iio_main.inp_h;
        initInfoJson["iio_main"]["out_w"] = __algoInitInfo.iio_main.out_w;
        initInfoJson["iio_main"]["out_h"] = __algoInitInfo.iio_main.out_h;
        initInfoJson["iio_main"]["src_w"] = __algoInitInfo.iio_main.src_w;
        initInfoJson["iio_main"]["src_h"] = __algoInitInfo.iio_main.src_h;
        initInfoJson["iio_main"]["wpe_w"] = __algoInitInfo.iio_main.wpe_w;
        initInfoJson["iio_main"]["wpe_h"] = __algoInitInfo.iio_main.wpe_h;

        initInfoJson["iio_auxi"]["inp_w"] = __algoInitInfo.iio_auxi.inp_w;
        initInfoJson["iio_auxi"]["inp_h"] = __algoInitInfo.iio_auxi.inp_h;
        initInfoJson["iio_auxi"]["out_w"] = __algoInitInfo.iio_auxi.out_w;
        initInfoJson["iio_auxi"]["out_h"] = __algoInitInfo.iio_auxi.out_h;
        initInfoJson["iio_auxi"]["src_w"] = __algoInitInfo.iio_auxi.src_w;
        initInfoJson["iio_auxi"]["src_h"] = __algoInitInfo.iio_auxi.src_h;
        initInfoJson["iio_auxi"]["wpe_w"] = __algoInitInfo.iio_auxi.wpe_w;
        initInfoJson["iio_auxi"]["wpe_h"] = __algoInitInfo.iio_auxi.wpe_h;

        initInfoJson["geo_info"] =  __algoInitInfo.geo_info;

        initInfoJson["geo_img"] = json::array();
        for(int level = 0; level < MAX_GEO_LEVEL; level++) {
            STEREO_KERNEL_GEO_INFO_STRUCT &geoInfo = __algoInitInfo.geo_img[level];
            if(0 == geoInfo.size) {
                break;
            }

            initInfoJson["geo_img"][level]["size"] = geoInfo.size;

            initInfoJson["geo_img"][level]["img_main"]["width"]      = geoInfo.img_main.width;
            initInfoJson["geo_img"][level]["img_main"]["height"]     = geoInfo.img_main.height;
            initInfoJson["geo_img"][level]["img_main"]["depth"]      = geoInfo.img_main.depth;
            initInfoJson["geo_img"][level]["img_main"]["stride"]     = geoInfo.img_main.stride;
            initInfoJson["geo_img"][level]["img_main"]["format"]     = geoInfo.img_main.format;
            initInfoJson["geo_img"][level]["img_main"]["act_width"]  = geoInfo.img_main.act_width;
            initInfoJson["geo_img"][level]["img_main"]["act_height"] = geoInfo.img_main.act_height;
            initInfoJson["geo_img"][level]["img_main"]["offset_x"]   = geoInfo.img_main.offset_x;
            initInfoJson["geo_img"][level]["img_main"]["offset_y"]   = geoInfo.img_main.offset_y;

            initInfoJson["geo_img"][level]["img_auxi"]["width"]      = geoInfo.img_auxi.width;
            initInfoJson["geo_img"][level]["img_auxi"]["height"]     = geoInfo.img_auxi.height;
            initInfoJson["geo_img"][level]["img_auxi"]["depth"]      = geoInfo.img_auxi.depth;
            initInfoJson["geo_img"][level]["img_auxi"]["stride"]     = geoInfo.img_auxi.stride;
            initInfoJson["geo_img"][level]["img_auxi"]["format"]     = geoInfo.img_auxi.format;
            initInfoJson["geo_img"][level]["img_auxi"]["act_width"]  = geoInfo.img_auxi.act_width;
            initInfoJson["geo_img"][level]["img_auxi"]["act_height"] = geoInfo.img_auxi.act_height;
            initInfoJson["geo_img"][level]["img_auxi"]["offset_x"]   = geoInfo.img_auxi.offset_x;
            initInfoJson["geo_img"][level]["img_auxi"]["offset_y"]   = geoInfo.img_auxi.offset_y;
        }

        initInfoJson["pho_img"]["width"]      = __algoInitInfo.pho_img.width;
        initInfoJson["pho_img"]["height"]     = __algoInitInfo.pho_img.height;
        initInfoJson["pho_img"]["depth"]      = __algoInitInfo.pho_img.depth;
        initInfoJson["pho_img"]["stride"]     = __algoInitInfo.pho_img.stride;
        initInfoJson["pho_img"]["format"]     = __algoInitInfo.pho_img.format;
        initInfoJson["pho_img"]["act_width"]  = __algoInitInfo.pho_img.act_width;
        initInfoJson["pho_img"]["act_height"] = __algoInitInfo.pho_img.act_height;
        initInfoJson["pho_img"]["offset_x"]   = __algoInitInfo.pho_img.offset_x;
        initInfoJson["pho_img"]["offset_y"]   = __algoInitInfo.pho_img.offset_y;

        initInfoJson["fov_main.h"] = __algoInitInfo.fov_main[0];
        initInfoJson["fov_main.v"] = __algoInitInfo.fov_main[1];

        initInfoJson["fov_auxi.h"] = __algoInitInfo.fov_auxi[0];
        initInfoJson["fov_auxi.v"] = __algoInitInfo.fov_auxi[1];
        initInfoJson["baseline"]   = __algoInitInfo.baseline;

        initInfoJson["system_cfg"] = __algoInitInfo.system_cfg;

        initInfoJson["af_init_main"]["dac_mcr"]  = __algoInitInfo.af_init_main.dac_mcr;
        initInfoJson["af_init_main"]["dac_inf"]  = __algoInitInfo.af_init_main.dac_inf;
        initInfoJson["af_init_main"]["dac_str"]  = __algoInitInfo.af_init_main.dac_str;
        initInfoJson["af_init_main"]["dist_mcr"] = __algoInitInfo.af_init_main.dist_mcr;
        initInfoJson["af_init_main"]["dist_inf"] = __algoInitInfo.af_init_main.dist_inf;

        initInfoJson["af_init_auxi"]["dac_mcr"]  = __algoInitInfo.af_init_auxi.dac_mcr;
        initInfoJson["af_init_auxi"]["dac_inf"]  = __algoInitInfo.af_init_auxi.dac_inf;
        initInfoJson["af_init_auxi"]["dac_str"]  = __algoInitInfo.af_init_auxi.dac_str;
        initInfoJson["af_init_auxi"]["dist_mcr"] = __algoInitInfo.af_init_auxi.dist_mcr;
        initInfoJson["af_init_auxi"]["dist_inf"] = __algoInitInfo.af_init_auxi.dist_inf;

        initInfoJson["flow_main"]["pixel_array_width"]  = __algoInitInfo.flow_main.pixel_array_width;
        initInfoJson["flow_main"]["pixel_array_height"] = __algoInitInfo.flow_main.pixel_array_height;
        initInfoJson["flow_main"]["sensor_offset_x0"]   = __algoInitInfo.flow_main.sensor_offset_x0;
        initInfoJson["flow_main"]["sensor_offset_y0"]   = __algoInitInfo.flow_main.sensor_offset_y0;
        initInfoJson["flow_main"]["sensor_size_w0"]     = __algoInitInfo.flow_main.sensor_size_w0;
        initInfoJson["flow_main"]["sensor_size_h0"]     = __algoInitInfo.flow_main.sensor_size_h0;
        initInfoJson["flow_main"]["sensor_scale_w"]     = __algoInitInfo.flow_main.sensor_scale_w;
        initInfoJson["flow_main"]["sensor_scale_h"]     = __algoInitInfo.flow_main.sensor_scale_h;
        initInfoJson["flow_main"]["sensor_offset_x1"]   = __algoInitInfo.flow_main.sensor_offset_x1;
        initInfoJson["flow_main"]["sensor_offset_y1"]   = __algoInitInfo.flow_main.sensor_offset_y1;
        initInfoJson["flow_main"]["sensor_size_w1"]     = __algoInitInfo.flow_main.sensor_size_w1;
        initInfoJson["flow_main"]["sensor_size_h1"]     = __algoInitInfo.flow_main.sensor_size_h1;
        initInfoJson["flow_main"]["tg_offset_x"]        = __algoInitInfo.flow_main.tg_offset_x;
        initInfoJson["flow_main"]["tg_offset_y"]        = __algoInitInfo.flow_main.tg_offset_y;
        initInfoJson["flow_main"]["tg_size_w"]          = __algoInitInfo.flow_main.tg_size_w;
        initInfoJson["flow_main"]["tg_size_h"]          = __algoInitInfo.flow_main.tg_size_h;
        initInfoJson["flow_main"]["rrz_offset_x"]       = __algoInitInfo.flow_main.rrz_offset_x;
        initInfoJson["flow_main"]["rrz_offset_y"]       = __algoInitInfo.flow_main.rrz_offset_y;
        initInfoJson["flow_main"]["rrz_usage_width"]    = __algoInitInfo.flow_main.rrz_usage_width;
        initInfoJson["flow_main"]["rrz_usage_height"]   = __algoInitInfo.flow_main.rrz_usage_height;
        initInfoJson["flow_main"]["rrz_out_width"]      = __algoInitInfo.flow_main.rrz_out_width;
        initInfoJson["flow_main"]["rrz_out_height"]     = __algoInitInfo.flow_main.rrz_out_height;
        initInfoJson["flow_main"]["mdp_rotate"]         = __algoInitInfo.flow_main.mdp_rotate;
        initInfoJson["flow_main"]["mdp_offset_x"]       = __algoInitInfo.flow_main.mdp_offset_x;
        initInfoJson["flow_main"]["mdp_offset_y"]       = __algoInitInfo.flow_main.mdp_offset_y;
        initInfoJson["flow_main"]["mdp_usage_width"]    = __algoInitInfo.flow_main.mdp_usage_width;
        initInfoJson["flow_main"]["mdp_usage_height"]   = __algoInitInfo.flow_main.mdp_usage_height;

        initInfoJson["flow_auxi"]["pixel_array_width"]  = __algoInitInfo.flow_auxi.pixel_array_width;
        initInfoJson["flow_auxi"]["pixel_array_height"] = __algoInitInfo.flow_auxi.pixel_array_height;
        initInfoJson["flow_auxi"]["sensor_offset_x0"]   = __algoInitInfo.flow_auxi.sensor_offset_x0;
        initInfoJson["flow_auxi"]["sensor_offset_y0"]   = __algoInitInfo.flow_auxi.sensor_offset_y0;
        initInfoJson["flow_auxi"]["sensor_size_w0"]     = __algoInitInfo.flow_auxi.sensor_size_w0;
        initInfoJson["flow_auxi"]["sensor_size_h0"]     = __algoInitInfo.flow_auxi.sensor_size_h0;
        initInfoJson["flow_auxi"]["sensor_scale_w"]     = __algoInitInfo.flow_auxi.sensor_scale_w;
        initInfoJson["flow_auxi"]["sensor_scale_h"]     = __algoInitInfo.flow_auxi.sensor_scale_h;
        initInfoJson["flow_auxi"]["sensor_offset_x1"]   = __algoInitInfo.flow_auxi.sensor_offset_x1;
        initInfoJson["flow_auxi"]["sensor_offset_y1"]   = __algoInitInfo.flow_auxi.sensor_offset_y1;
        initInfoJson["flow_auxi"]["sensor_size_w1"]     = __algoInitInfo.flow_auxi.sensor_size_w1;
        initInfoJson["flow_auxi"]["sensor_size_h1"]     = __algoInitInfo.flow_auxi.sensor_size_h1;
        initInfoJson["flow_auxi"]["tg_offset_x"]        = __algoInitInfo.flow_auxi.tg_offset_x;
        initInfoJson["flow_auxi"]["tg_offset_y"]        = __algoInitInfo.flow_auxi.tg_offset_y;
        initInfoJson["flow_auxi"]["tg_size_w"]          = __algoInitInfo.flow_auxi.tg_size_w;
        initInfoJson["flow_auxi"]["tg_size_h"]          = __algoInitInfo.flow_auxi.tg_size_h;
        initInfoJson["flow_auxi"]["rrz_offset_x"]       = __algoInitInfo.flow_auxi.rrz_offset_x;
        initInfoJson["flow_auxi"]["rrz_offset_y"]       = __algoInitInfo.flow_auxi.rrz_offset_y;
        initInfoJson["flow_auxi"]["rrz_usage_width"]    = __algoInitInfo.flow_auxi.rrz_usage_width;
        initInfoJson["flow_auxi"]["rrz_usage_height"]   = __algoInitInfo.flow_auxi.rrz_usage_height;
        initInfoJson["flow_auxi"]["rrz_out_width"]      = __algoInitInfo.flow_auxi.rrz_out_width;
        initInfoJson["flow_auxi"]["rrz_out_height"]     = __algoInitInfo.flow_auxi.rrz_out_height;
        initInfoJson["flow_auxi"]["mdp_rotate"]         = __algoInitInfo.flow_auxi.mdp_rotate;
        initInfoJson["flow_auxi"]["mdp_offset_x"]       = __algoInitInfo.flow_auxi.mdp_offset_x;
        initInfoJson["flow_auxi"]["mdp_offset_y"]       = __algoInitInfo.flow_auxi.mdp_offset_y;
        initInfoJson["flow_auxi"]["mdp_usage_width"]    = __algoInitInfo.flow_auxi.mdp_usage_width;
        initInfoJson["flow_auxi"]["mdp_usage_height"]   = __algoInitInfo.flow_auxi.mdp_usage_height;

        __initInfoJSON = initInfoJson.dump(4);
    }

    fwrite(__initInfoJSON.c_str(), 1, __initInfoJSON.size(), fp);
    fflush(fp);
    fclose(fp);
    fp = NULL;
}

void
N3D_HAL_KERNEL::dumpN3DRuntimeConfig()
{
    if( !IS_DUMP_ENALED ) {
        return;
    }

    const size_t DUMP_PATH_SIZE = 128;
    char dumpPath[DUMP_PATH_SIZE];
    genFileName_VSDOF_BUFFER(dumpPath, DUMP_PATH_SIZE, __dumpHint, "N3D_PROC_INFO.json");
    json procInfoJson;

    procInfoJson["runtime_cfg"] = __algoProcInfo.runtime_cfg;

    procInfoJson["af_main"]["dac_i"] = __algoProcInfo.af_main.dac_i;
    procInfoJson["af_main"]["dac_v"] = __algoProcInfo.af_main.dac_v;
    procInfoJson["af_main"]["dac_c"] = __algoProcInfo.af_main.dac_c;
    procInfoJson["af_main"]["dac_w"] = json::array();
    for(int i = 0; i < 4; i++) {
        procInfoJson["af_main"]["dac_w"][i] = __algoProcInfo.af_main.dac_w[i];
    }

    procInfoJson["af_auxi"]["dac_i"] = __algoProcInfo.af_auxi.dac_i;
    procInfoJson["af_auxi"]["dac_v"] = __algoProcInfo.af_auxi.dac_v;
    procInfoJson["af_auxi"]["dac_c"] = __algoProcInfo.af_auxi.dac_c;
    procInfoJson["af_auxi"]["dac_w"] = json::array();
    for(int i = 0; i < 4; i++) {
        procInfoJson["af_auxi"]["dac_w"][i] = __algoProcInfo.af_auxi.dac_w[i];
    }

    procInfoJson["af_auxi"]["eis"] = json::array();
    for(int i = 0; i < 4; i++) {
        procInfoJson["af_auxi"]["eis"][i] = __algoProcInfo.eis[i];
    }

    std::ofstream of(dumpPath);
    of << std::setw(4) << procInfoJson;
}

std::string
N3D_HAL_KERNEL::__getN3DLogPath()
{
    if(__dumpHint) {
        const size_t DUMP_PATH_SIZE = 128;
        char path[DUMP_PATH_SIZE];
        genFileName_VSDOF_BUFFER(path, DUMP_PATH_SIZE, __dumpHint, "N3D_LOG");
        return std::string(path);
    }

    return std::string();
}

void
N3D_HAL_KERNEL::__dumpN3DLog(std::string dumpPath)
{
    if( !IS_DUMP_ENALED  ||
        0 == dumpPath.size() )
    {
        return;
    }

    FILE *fp = fopen(dumpPath.c_str(), "wb");
    if(NULL == fp) {
        MY_LOGE("Cannot dump N3D log to %s, error: %s", dumpPath.c_str(), strerror(errno));
        return;
    }

    __pStereoDrv->StereoKernelFeatureCtrl( STEREO_KERNEL_FEATURE_DEBUG, fp, (void *)"XXX_DEBUG_INFO" );
    fflush(fp);
    fclose(fp);
    fp = NULL;
}