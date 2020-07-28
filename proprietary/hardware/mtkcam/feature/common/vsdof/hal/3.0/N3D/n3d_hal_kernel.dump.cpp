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

#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/rapidjson/stringbuffer.h>
#include <vsdof/hal/rapidjson/document.h>     // rapidjson's DOM-style API
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/rapidjson/filewritestream.h>

using namespace rapidjson;

#define IS_DUMP_ENALED ( __isDump && NULL != __dumpHint )

void
N3D_HAL_KERNEL::setDumpConfig(bool dump, TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint)
{
    __isDump = dump;
    __dumpHint = dumpHint;
}

void __addIntValue(Value &v, const char *key, int value, Document::AllocatorType& allocator)
{
    v.AddMember(StringRef(key, strlen(key)), value, allocator);
}

void __addFloatValue(Value &v, const char *key, float value, Document::AllocatorType& allocator)
{
    v.AddMember(StringRef(key, strlen(key)), value, allocator);
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
        Document document(kObjectType);
        Document::AllocatorType& allocator = document.GetAllocator();

        #define AddIntValue(key, value) document.AddMember(key, Value().SetInt(value), allocator)
        #define AddFloatValue(key, value) document.AddMember(key, Value().SetFloat(value), allocator)

        AddIntValue("scenario", __algoInitInfo.scenario);

        //iio_main
        Value iio_main_value(kObjectType);
        __addIntValue(iio_main_value, "inp_w", __algoInitInfo.iio_main.inp_w, allocator);
        __addIntValue(iio_main_value, "inp_h", __algoInitInfo.iio_main.inp_h, allocator);
        __addIntValue(iio_main_value, "out_w", __algoInitInfo.iio_main.out_w, allocator);
        __addIntValue(iio_main_value, "out_h", __algoInitInfo.iio_main.out_h, allocator);
        __addIntValue(iio_main_value, "src_w", __algoInitInfo.iio_main.src_w, allocator);
        __addIntValue(iio_main_value, "src_h", __algoInitInfo.iio_main.src_h, allocator);
        __addIntValue(iio_main_value, "wpe_w", __algoInitInfo.iio_main.wpe_w, allocator);
        __addIntValue(iio_main_value, "wpe_h", __algoInitInfo.iio_main.wpe_h, allocator);
        document.AddMember("iio_main", iio_main_value, allocator);

        Value iio_auxi_value(kObjectType);
        __addIntValue(iio_auxi_value, "inp_w", __algoInitInfo.iio_auxi.inp_w, allocator);
        __addIntValue(iio_auxi_value, "inp_h", __algoInitInfo.iio_auxi.inp_h, allocator);
        __addIntValue(iio_auxi_value, "out_w", __algoInitInfo.iio_auxi.out_w, allocator);
        __addIntValue(iio_auxi_value, "out_h", __algoInitInfo.iio_auxi.out_h, allocator);
        __addIntValue(iio_auxi_value, "src_w", __algoInitInfo.iio_auxi.src_w, allocator);
        __addIntValue(iio_auxi_value, "src_h", __algoInitInfo.iio_auxi.src_h, allocator);
        __addIntValue(iio_auxi_value, "wpe_w", __algoInitInfo.iio_auxi.wpe_w, allocator);
        __addIntValue(iio_auxi_value, "wpe_h", __algoInitInfo.iio_auxi.wpe_h, allocator);
        document.AddMember("iio_auxi", iio_auxi_value, allocator);

        AddIntValue("geo_info", __algoInitInfo.geo_info);

        Value geoValues(kArrayType);
        STEREO_KERNEL_GEO_INFO_STRUCT geo_img[MAX_GEO_LEVEL];
        for(int level = 0; level < MAX_GEO_LEVEL; level++) {
            STEREO_KERNEL_GEO_INFO_STRUCT &geoInfo = __algoInitInfo.geo_img[level];
            if(0 == geoInfo.size) {
                break;
            }

            Value value(kObjectType);
            value.AddMember("size", Value().SetInt(geoInfo.size), allocator);

            Value img_main_value(kObjectType);
            __addIntValue(img_main_value, "width", geoInfo.img_main.width, allocator);
            __addIntValue(img_main_value, "height", geoInfo.img_main.height, allocator);
            __addIntValue(img_main_value, "depth", geoInfo.img_main.depth, allocator);
            __addIntValue(img_main_value, "stride", geoInfo.img_main.stride, allocator);
            __addIntValue(img_main_value, "format", geoInfo.img_main.format, allocator);
            __addIntValue(img_main_value, "act_width", geoInfo.img_main.act_width, allocator);
            __addIntValue(img_main_value, "act_height", geoInfo.img_main.act_height, allocator);
            __addIntValue(img_main_value, "offset_x", geoInfo.img_main.offset_x, allocator);
            __addIntValue(img_main_value, "offset_y", geoInfo.img_main.offset_y, allocator);
            value.AddMember("img_main", img_main_value, allocator);

            Value img_auxi_value(kObjectType);
            __addIntValue(img_auxi_value, "width", geoInfo.img_auxi.width, allocator);
            __addIntValue(img_auxi_value, "height", geoInfo.img_auxi.height, allocator);
            __addIntValue(img_auxi_value, "depth", geoInfo.img_auxi.depth, allocator);
            __addIntValue(img_auxi_value, "stride", geoInfo.img_auxi.stride, allocator);
            __addIntValue(img_auxi_value, "format", geoInfo.img_auxi.format, allocator);
            __addIntValue(img_auxi_value, "act_width", geoInfo.img_auxi.act_width, allocator);
            __addIntValue(img_auxi_value, "act_height", geoInfo.img_auxi.act_height, allocator);
            __addIntValue(img_auxi_value, "offset_x", geoInfo.img_auxi.offset_x, allocator);
            __addIntValue(img_auxi_value, "offset_y", geoInfo.img_auxi.offset_y, allocator);
            value.AddMember("img_auxi", img_auxi_value, allocator);

            geoValues.PushBack(value.Move(), allocator);
        }
        document.AddMember("geo_img", geoValues.Move(), allocator);

        Value pho_img_value(kObjectType);
        __addIntValue(pho_img_value, "width", __algoInitInfo.pho_img.width, allocator);
        __addIntValue(pho_img_value, "height", __algoInitInfo.pho_img.height, allocator);
        __addIntValue(pho_img_value, "depth", __algoInitInfo.pho_img.depth, allocator);
        __addIntValue(pho_img_value, "stride", __algoInitInfo.pho_img.stride, allocator);
        __addIntValue(pho_img_value, "format", __algoInitInfo.pho_img.format, allocator);
        __addIntValue(pho_img_value, "act_width", __algoInitInfo.pho_img.act_width, allocator);
        __addIntValue(pho_img_value, "act_height", __algoInitInfo.pho_img.act_height, allocator);
        __addIntValue(pho_img_value, "offset_x", __algoInitInfo.pho_img.offset_x, allocator);
        __addIntValue(pho_img_value, "offset_y", __algoInitInfo.pho_img.offset_y, allocator);
        document.AddMember("pho_img", pho_img_value, allocator);

        AddFloatValue("fov_main.h", __algoInitInfo.fov_main[0]);
        AddFloatValue("fov_main.v", __algoInitInfo.fov_main[1]);

        AddFloatValue("fov_auxi.h", __algoInitInfo.fov_auxi[0]);
        AddFloatValue("fov_auxi.v", __algoInitInfo.fov_auxi[1]);
        AddFloatValue("baseline", __algoInitInfo.baseline);

        AddIntValue("system_cfg", __algoInitInfo.system_cfg);

        Value af_init_main_value(kObjectType);
        __addIntValue(af_init_main_value, "dac_mcr", __algoInitInfo.af_init_main.dac_mcr, allocator);
        __addIntValue(af_init_main_value, "dac_inf", __algoInitInfo.af_init_main.dac_inf, allocator);
        __addIntValue(af_init_main_value, "dac_str", __algoInitInfo.af_init_main.dac_str, allocator);
        __addIntValue(af_init_main_value, "dist_mcr", __algoInitInfo.af_init_main.dist_mcr, allocator);
        __addIntValue(af_init_main_value, "dist_inf", __algoInitInfo.af_init_main.dist_inf, allocator);
        document.AddMember("af_init_main", af_init_main_value, allocator);

        Value af_init_auxi_value(kObjectType);
        __addIntValue(af_init_auxi_value, "dac_mcr", __algoInitInfo.af_init_auxi.dac_mcr, allocator);
        __addIntValue(af_init_auxi_value, "dac_inf", __algoInitInfo.af_init_auxi.dac_inf, allocator);
        __addIntValue(af_init_auxi_value, "dac_str", __algoInitInfo.af_init_auxi.dac_str, allocator);
        __addIntValue(af_init_auxi_value, "dist_mcr", __algoInitInfo.af_init_auxi.dist_mcr, allocator);
        __addIntValue(af_init_auxi_value, "dist_inf", __algoInitInfo.af_init_auxi.dist_inf, allocator);
        document.AddMember("af_init_auxi", af_init_auxi_value, allocator);

        Value flow_main_value(kObjectType);
        __addIntValue(flow_main_value, "pixel_array_width", __algoInitInfo.flow_main.pixel_array_width, allocator);
        __addIntValue(flow_main_value, "pixel_array_height", __algoInitInfo.flow_main.pixel_array_height, allocator);
        __addIntValue(flow_main_value, "sensor_offset_x0", __algoInitInfo.flow_main.sensor_offset_x0, allocator);
        __addIntValue(flow_main_value, "sensor_offset_y0", __algoInitInfo.flow_main.sensor_offset_y0, allocator);
        __addIntValue(flow_main_value, "sensor_size_w0", __algoInitInfo.flow_main.sensor_size_w0, allocator);
        __addIntValue(flow_main_value, "sensor_size_h0", __algoInitInfo.flow_main.sensor_size_h0, allocator);
        __addIntValue(flow_main_value, "sensor_scale_w", __algoInitInfo.flow_main.sensor_scale_w, allocator);
        __addIntValue(flow_main_value, "sensor_scale_h", __algoInitInfo.flow_main.sensor_scale_h, allocator);
        __addIntValue(flow_main_value, "sensor_offset_x1", __algoInitInfo.flow_main.sensor_offset_x1, allocator);
        __addIntValue(flow_main_value, "sensor_offset_y1", __algoInitInfo.flow_main.sensor_offset_y1, allocator);
        __addIntValue(flow_main_value, "sensor_size_w1", __algoInitInfo.flow_main.sensor_size_w1, allocator);
        __addIntValue(flow_main_value, "sensor_size_h1", __algoInitInfo.flow_main.sensor_size_h1, allocator);
        __addIntValue(flow_main_value, "tg_offset_x", __algoInitInfo.flow_main.tg_offset_x, allocator);
        __addIntValue(flow_main_value, "tg_offset_y", __algoInitInfo.flow_main.tg_offset_y, allocator);
        __addIntValue(flow_main_value, "tg_size_w", __algoInitInfo.flow_main.tg_size_w, allocator);
        __addIntValue(flow_main_value, "tg_size_h", __algoInitInfo.flow_main.tg_size_h, allocator);
        __addIntValue(flow_main_value, "rrz_offset_x", __algoInitInfo.flow_main.rrz_offset_x, allocator);
        __addIntValue(flow_main_value, "rrz_offset_y", __algoInitInfo.flow_main.rrz_offset_y, allocator);
        __addIntValue(flow_main_value, "rrz_usage_width", __algoInitInfo.flow_main.rrz_usage_width, allocator);
        __addIntValue(flow_main_value, "rrz_usage_height", __algoInitInfo.flow_main.rrz_usage_height, allocator);
        __addIntValue(flow_main_value, "rrz_out_width", __algoInitInfo.flow_main.rrz_out_width, allocator);
        __addIntValue(flow_main_value, "rrz_out_height", __algoInitInfo.flow_main.rrz_out_height, allocator);
        __addIntValue(flow_main_value, "mdp_rotate", __algoInitInfo.flow_main.mdp_rotate, allocator);
        __addIntValue(flow_main_value, "mdp_offset_x", __algoInitInfo.flow_main.mdp_offset_x, allocator);
        __addIntValue(flow_main_value, "mdp_offset_y", __algoInitInfo.flow_main.mdp_offset_y, allocator);
        __addIntValue(flow_main_value, "mdp_usage_width", __algoInitInfo.flow_main.mdp_usage_width, allocator);
        __addIntValue(flow_main_value, "mdp_usage_height", __algoInitInfo.flow_main.mdp_usage_height, allocator);
        document.AddMember("flow_main", flow_main_value, allocator);

        Value flow_auxi_value(kObjectType);
        __addIntValue(flow_auxi_value, "pixel_array_width", __algoInitInfo.flow_auxi.pixel_array_width, allocator);
        __addIntValue(flow_auxi_value, "pixel_array_height", __algoInitInfo.flow_auxi.pixel_array_height, allocator);
        __addIntValue(flow_auxi_value, "sensor_offset_x0", __algoInitInfo.flow_auxi.sensor_offset_x0, allocator);
        __addIntValue(flow_auxi_value, "sensor_offset_y0", __algoInitInfo.flow_auxi.sensor_offset_y0, allocator);
        __addIntValue(flow_auxi_value, "sensor_size_w0", __algoInitInfo.flow_auxi.sensor_size_w0, allocator);
        __addIntValue(flow_auxi_value, "sensor_size_h0", __algoInitInfo.flow_auxi.sensor_size_h0, allocator);
        __addIntValue(flow_auxi_value, "sensor_scale_w", __algoInitInfo.flow_auxi.sensor_scale_w, allocator);
        __addIntValue(flow_auxi_value, "sensor_scale_h", __algoInitInfo.flow_auxi.sensor_scale_h, allocator);
        __addIntValue(flow_auxi_value, "sensor_offset_x1", __algoInitInfo.flow_auxi.sensor_offset_x1, allocator);
        __addIntValue(flow_auxi_value, "sensor_offset_y1", __algoInitInfo.flow_auxi.sensor_offset_y1, allocator);
        __addIntValue(flow_auxi_value, "sensor_size_w1", __algoInitInfo.flow_auxi.sensor_size_w1, allocator);
        __addIntValue(flow_auxi_value, "sensor_size_h1", __algoInitInfo.flow_auxi.sensor_size_h1, allocator);
        __addIntValue(flow_auxi_value, "tg_offset_x", __algoInitInfo.flow_auxi.tg_offset_x, allocator);
        __addIntValue(flow_auxi_value, "tg_offset_y", __algoInitInfo.flow_auxi.tg_offset_y, allocator);
        __addIntValue(flow_auxi_value, "tg_size_w", __algoInitInfo.flow_auxi.tg_size_w, allocator);
        __addIntValue(flow_auxi_value, "tg_size_h", __algoInitInfo.flow_auxi.tg_size_h, allocator);
        __addIntValue(flow_auxi_value, "rrz_offset_x", __algoInitInfo.flow_auxi.rrz_offset_x, allocator);
        __addIntValue(flow_auxi_value, "rrz_offset_y", __algoInitInfo.flow_auxi.rrz_offset_y, allocator);
        __addIntValue(flow_auxi_value, "rrz_usage_width", __algoInitInfo.flow_auxi.rrz_usage_width, allocator);
        __addIntValue(flow_auxi_value, "rrz_usage_height", __algoInitInfo.flow_auxi.rrz_usage_height, allocator);
        __addIntValue(flow_auxi_value, "rrz_out_width", __algoInitInfo.flow_auxi.rrz_out_width, allocator);
        __addIntValue(flow_auxi_value, "rrz_out_height", __algoInitInfo.flow_auxi.rrz_out_height, allocator);
        __addIntValue(flow_auxi_value, "mdp_rotate", __algoInitInfo.flow_auxi.mdp_rotate, allocator);
        __addIntValue(flow_auxi_value, "mdp_offset_x", __algoInitInfo.flow_auxi.mdp_offset_x, allocator);
        __addIntValue(flow_auxi_value, "mdp_offset_y", __algoInitInfo.flow_auxi.mdp_offset_y, allocator);
        __addIntValue(flow_auxi_value, "mdp_usage_width", __algoInitInfo.flow_auxi.mdp_usage_width, allocator);
        __addIntValue(flow_auxi_value, "mdp_usage_height", __algoInitInfo.flow_auxi.mdp_usage_height, allocator);
        document.AddMember("flow_auxi", flow_auxi_value, allocator);

        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        writer.SetFormatOptions(kFormatSingleLineArray);
        document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        __initInfoJSON = std::string(sb.GetString());
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
    FILE *fp = fopen(dumpPath, "wb");
    if(NULL == fp) {
        MY_LOGE("Cannot dump N3D proc info to %s, error: %s", dumpPath, strerror(errno));
        return;
    }

    Document document(kObjectType);
    Document::AllocatorType& allocator = document.GetAllocator();

    #define AddIntValue(key, value) document.AddMember(key, Value().SetInt(value), allocator)

    document.AddMember("runtime_cfg", __algoProcInfo.runtime_cfg, allocator);

    Value af_main_value(kObjectType);
    __addIntValue(af_main_value, "dac_i", __algoProcInfo.af_main.dac_i, allocator);
    __addIntValue(af_main_value, "dac_v", __algoProcInfo.af_main.dac_v, allocator);
    __addFloatValue(af_main_value, "dac_c", __algoProcInfo.af_main.dac_c, allocator);
    Value af_main_dac_w(kArrayType);
    for(int i = 0; i < 4; i++) {
        af_main_dac_w.PushBack(Value().SetInt(__algoProcInfo.af_main.dac_w[i]), allocator);
    }
    af_main_value.AddMember("dac_w", af_main_dac_w.Move(), allocator);
    document.AddMember("af_main", af_main_value, allocator);

    Value af_auxi_value(kObjectType);
    __addIntValue(af_auxi_value, "dac_i", __algoProcInfo.af_auxi.dac_i, allocator);
    __addIntValue(af_auxi_value, "dac_v", __algoProcInfo.af_auxi.dac_v, allocator);
    __addFloatValue(af_auxi_value, "dac_c", __algoProcInfo.af_auxi.dac_c, allocator);
    Value af_auxi_dac_w(kArrayType);
    for(int i = 0; i < 4; i++) {
        af_auxi_dac_w.PushBack(Value().SetInt(__algoProcInfo.af_auxi.dac_w[i]), allocator);
    }
    af_auxi_value.AddMember("dac_w", af_auxi_dac_w.Move(), allocator);
    document.AddMember("af_auxi", af_auxi_value, allocator);

    Value eisValues(kArrayType);
    for(int i = 0; i < 4; i++) {
        eisValues.PushBack(Value().SetInt(__algoProcInfo.eis[i]), allocator);
    }
    document.AddMember("eis", eisValues.Move(), allocator);

    char writeBuffer[1024];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    PrettyWriter<FileWriteStream> prettyWriter(os);
    prettyWriter.SetFormatOptions(kFormatSingleLineArray);
    document.Accept(prettyWriter);

    fflush(fp);
    fclose(fp);
    fp = NULL;
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