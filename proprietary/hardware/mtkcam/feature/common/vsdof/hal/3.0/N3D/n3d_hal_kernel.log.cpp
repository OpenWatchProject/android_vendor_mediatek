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

using namespace StereoHAL;

#if (1==SINGLE_LINE_LOG)
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD(fmt, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI(fmt, ##arg)
#else
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD("[%s]" fmt, __func__, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI("[%s]" fmt, __func__, ##arg)
#endif
#define FAST_LOGW(fmt, arg...)  __fastLogger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FAST_LOGE(fmt, arg...)  __fastLogger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define FAST_LOG_PRINT  __fastLogger.print()

// Logger
void
N3D_HAL_KERNEL::__logInitInfo(const STEREO_KERNEL_SET_ENV_INFO_STRUCT &initInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= N3D Init Info =========");
    FAST_LOGD("[scenario]     %d", initInfo.scenario);

    // ALGORITHM INPUT and SbS OUTPUT
    FAST_LOGD("[iio_main.src_w]   %d", initInfo.iio_main.src_w);
    FAST_LOGD("[iio_main.src_h]   %d", initInfo.iio_main.src_h);
    FAST_LOGD("[iio_main.out_w]   %d", initInfo.iio_main.out_w);
    FAST_LOGD("[iio_main.out_h]   %d", initInfo.iio_main.out_h);
    FAST_LOGD("[iio_main.inp_w]   %d", initInfo.iio_main.inp_w);
    FAST_LOGD("[iio_main.inp_h]   %d", initInfo.iio_main.inp_h);
    FAST_LOGD("[iio_main.wpe_w]   %d", initInfo.iio_main.wpe_w);
    FAST_LOGD("[iio_main.wpe_h]   %d", initInfo.iio_main.wpe_h);

    FAST_LOGD("[iio_auxi.src_w]   %d", initInfo.iio_auxi.src_w);
    FAST_LOGD("[iio_auxi.src_h]   %d", initInfo.iio_auxi.src_h);
    FAST_LOGD("[iio_auxi.inp_w]   %d", initInfo.iio_auxi.inp_w);
    FAST_LOGD("[iio_auxi.inp_h]   %d", initInfo.iio_auxi.inp_h);
    FAST_LOGD("[iio_auxi.out_w]   %d", initInfo.iio_auxi.out_w);
    FAST_LOGD("[iio_auxi.out_h]   %d", initInfo.iio_auxi.out_h);
    FAST_LOGD("[iio_auxi.wpe_w]   %d", initInfo.iio_auxi.wpe_w);
    FAST_LOGD("[iio_auxi.wpe_h]   %d", initInfo.iio_auxi.wpe_h);

    __logAFInitInfo("[af_init_main]", initInfo.af_init_main);
    __logAFInitInfo("[af_init_auxi]", initInfo.af_init_auxi);

    // HWFE INPUT - the actual size for HWFE (after SRZ)
    FAST_LOGD("[geo_info]  %d", initInfo.geo_info);

    char logPrefix[32];
    int i = 0;
    for(i = 0; i < MAX_GEO_LEVEL; i++) {
        if(initInfo.geo_img[i].size <= 0) {
            continue;
        }

        FAST_LOGD("[geo_img][%d][size] %d", i, initInfo.geo_img[i].size);

        sprintf(logPrefix, "[geo_img][%d][main]", i);
        __logImgInfo(logPrefix, initInfo.geo_img[i].img_main);

        sprintf(logPrefix, "[geo_img][%d][auxi]", i);
        __logImgInfo(logPrefix, initInfo.geo_img[i].img_auxi);
    }

    // COLOR CORRECTION INPUT
    __logImgInfo("[pho_img]", initInfo.pho_img);

    // Learning
    FAST_LOGD("[fov_main]         h: %.1f v: %.1f", initInfo.fov_main[0], initInfo.fov_main[1]);
    FAST_LOGD("[fov_auxi]         h: %.1f v: %.1f", initInfo.fov_auxi[0], initInfo.fov_auxi[1]);
    FAST_LOGD("[baseline]         %.2f", initInfo.baseline);
    std::string s = std::bitset<32>(initInfo.system_cfg).to_string();
    FAST_LOGD("[system_cfg]       %s", s.c_str());

    // Learning Coordinates RE-MAPPING
    __logRemapInfo("[flow_main]", initInfo.flow_main);
    __logRemapInfo("[flow_auxi]", initInfo.flow_auxi);

    // OUTPUT after Initialization
    MUINT32 working_buffer_size ;
    FAST_LOGD("[working_buffer_size]  %d", initInfo.working_buffer_size);
    FAST_LOG_PRINT;
}

void
N3D_HAL_KERNEL::__logImgInfo(const char *prefix, const STEREO_KERNEL_IMG_INFO_STRUCT &imgInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    FAST_LOGD("%s[width]      %d", prefix, imgInfo.width);
    FAST_LOGD("%s[height]     %d", prefix, imgInfo.height);
    FAST_LOGD("%s[depth]      %d", prefix, imgInfo.depth);
    FAST_LOGD("%s[stride]     %d", prefix, imgInfo.stride);
    FAST_LOGD("%s[format]     %d", prefix, imgInfo.format);
    FAST_LOGD("%s[act_width]  %d", prefix, imgInfo.act_width);
    FAST_LOGD("%s[act_height] %d", prefix, imgInfo.act_height);
    FAST_LOGD("%s[offset_x]   %d", prefix, imgInfo.offset_x);
    FAST_LOGD("%s[offset_y]   %d", prefix, imgInfo.offset_y);
}

void
N3D_HAL_KERNEL::__logRemapInfo(const char *prefix, const STEREO_KERNEL_FLOW_INFO_STRUCT &remapInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    FAST_LOGD("%s[pixel_array_width]    %d", prefix, remapInfo.pixel_array_width);
    FAST_LOGD("%s[pixel_array_height]   %d", prefix, remapInfo.pixel_array_height);
    FAST_LOGD("%s[sensor_offset_x0]     %d", prefix, remapInfo.sensor_offset_x0);
    FAST_LOGD("%s[sensor_offset_y0]     %d", prefix, remapInfo.sensor_offset_y0);
    FAST_LOGD("%s[sensor_size_w0]       %d", prefix, remapInfo.sensor_size_w0);
    FAST_LOGD("%s[sensor_size_h0]       %d", prefix, remapInfo.sensor_size_h0);
    FAST_LOGD("%s[sensor_scale_w]       %d", prefix, remapInfo.sensor_scale_w);
    FAST_LOGD("%s[sensor_scale_h]       %d", prefix, remapInfo.sensor_scale_h);
    FAST_LOGD("%s[sensor_offset_x1]     %d", prefix, remapInfo.sensor_offset_x1);
    FAST_LOGD("%s[sensor_offset_y1]     %d", prefix, remapInfo.sensor_offset_y1);
    FAST_LOGD("%s[tg_offset_x]          %d", prefix, remapInfo.tg_offset_x);
    FAST_LOGD("%s[tg_offset_y]          %d", prefix, remapInfo.tg_offset_y);
    FAST_LOGD("%s[sensor_size_w1]       %d", prefix, remapInfo.sensor_size_w1);
    FAST_LOGD("%s[sensor_size_h1]       %d", prefix, remapInfo.sensor_size_h1);
    FAST_LOGD("%s[tg_size_w]            %d", prefix, remapInfo.tg_size_w);
    FAST_LOGD("%s[tg_size_h]            %d", prefix, remapInfo.tg_size_h);
    FAST_LOGD("%s[mdp_rotate]           %d", prefix, remapInfo.mdp_rotate);
    FAST_LOGD("%s[rrz_offset_x]         %d", prefix, remapInfo.rrz_offset_x);
    FAST_LOGD("%s[rrz_offset_y]         %d", prefix, remapInfo.rrz_offset_y);
    FAST_LOGD("%s[rrz_usage_width]      %d", prefix, remapInfo.rrz_usage_width);
    FAST_LOGD("%s[rrz_usage_height]     %d", prefix, remapInfo.rrz_usage_height);
    FAST_LOGD("%s[rrz_out_width]        %d", prefix, remapInfo.rrz_out_width);
    FAST_LOGD("%s[rrz_out_height]       %d", prefix, remapInfo.rrz_out_height);
}

void
N3D_HAL_KERNEL::__logAFInitInfo(const char *prefix, const STEREO_KERNEL_AF_INIT_STRUCT &afInitInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    FAST_LOGD("%s[dac_mcr]    %d", prefix, afInitInfo.dac_mcr);
    FAST_LOGD("%s[dac_inf]    %d", prefix, afInitInfo.dac_inf);
    FAST_LOGD("%s[dac_str]    %d", prefix, afInitInfo.dac_str);
    FAST_LOGD("%s[dist_mcr]   %d", prefix, afInitInfo.dist_mcr);
    FAST_LOGD("%s[dist_inf]   %d", prefix, afInitInfo.dist_inf);
}

void
N3D_HAL_KERNEL::__logTuningInfo(const char *prefix, const STEREO_KERNEL_TUNING_PARA_STRUCT &tuningInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    std::string s = std::bitset<32>(tuningInfo.p_tune).to_string();
    FAST_LOGD("%s[p_tune]   %s", prefix, s.c_str());
    s = std::bitset<32>(tuningInfo.l_tune).to_string();
    FAST_LOGD("%s[l_tune]   %s", prefix, s.c_str());
    FAST_LOG_PRINT;
}

void
N3D_HAL_KERNEL::__logSetProcInfo(const char *prefix, const STEREO_KERNEL_SET_PROC_INFO_STRUCT &setprocInfo)
{
    if(!__LOG_ENABLED) {
        return;
    }

    std::string s = std::bitset<10>(setprocInfo.runtime_cfg).to_string();
    FAST_LOGD("%s[runtime_cfg]        %s", prefix, s.c_str());

    // FAST_LOGD("%s[addr_mg]            %p %p %p", prefix, setprocInfo.addr_mg[0], setprocInfo.addr_mg[1], setprocInfo.addr_mg[2]);
    // FAST_LOGD("%s[addr_ag]            %p %p %p", prefix, setprocInfo.addr_ag[0], setprocInfo.addr_ag[1], setprocInfo.addr_ag[2]);

    if(__hasFEFM) {
        FAST_LOGD("%s[addr_me]            %p %p %p", prefix, setprocInfo.addr_me[0], setprocInfo.addr_me[1], setprocInfo.addr_me[2]);
        FAST_LOGD("%s[addr_ae]            %p %p %p", prefix, setprocInfo.addr_ae[0], setprocInfo.addr_ae[1], setprocInfo.addr_ae[2]);
        FAST_LOGD("%s[addr_rl]            %p %p %p", prefix, setprocInfo.addr_rl[0], setprocInfo.addr_rl[1], setprocInfo.addr_rl[2]);
        FAST_LOGD("%s[addr_lr]            %p %p %p", prefix, setprocInfo.addr_lr[0], setprocInfo.addr_lr[1], setprocInfo.addr_lr[2]);

        FAST_LOGD("%s[addr_mp]            %p", prefix, setprocInfo.addr_mp);
        FAST_LOGD("%s[addr_ap]            %p", prefix, setprocInfo.addr_ap);
    }

    FAST_LOGD("%s[dac_i#0]            %d", prefix, setprocInfo.af_main.dac_i);
    FAST_LOGD("%s[dac_v#0]            %d", prefix, setprocInfo.af_main.dac_v);
    FAST_LOGD("%s[dac_c#0]            %f", prefix, setprocInfo.af_main.dac_c);
    FAST_LOGD("%s[dac_w#0]            %d %d %d %d", prefix,
                                                  setprocInfo.af_main.dac_w[0], setprocInfo.af_main.dac_w[1],
                                                  setprocInfo.af_main.dac_w[2], setprocInfo.af_main.dac_w[3]);
    FAST_LOGD("%s[dac_i#1]            %d", prefix, setprocInfo.af_auxi.dac_i);
    FAST_LOGD("%s[dac_v#1]            %d", prefix, setprocInfo.af_auxi.dac_v);
    FAST_LOGD("%s[dac_c#1]            %f", prefix, setprocInfo.af_auxi.dac_c);
    FAST_LOGD("%s[dac_w#1]            %d %d %d %d", prefix,
                                                  setprocInfo.af_auxi.dac_w[0], setprocInfo.af_auxi.dac_w[1],
                                                  setprocInfo.af_auxi.dac_w[2], setprocInfo.af_auxi.dac_w[3]);

    if(setprocInfo.eis[2] > 0) {
        FAST_LOGD("%s[eis]            %d %d %d %d", prefix, setprocInfo.eis[0], setprocInfo.eis[1], setprocInfo.eis[2], setprocInfo.eis[3]);
    }

    if(E_N3D_WARP_WPE != N3D_WARP_HW) {
        if(!__DISABLE_GPU) {
            FAST_LOGD("%s[InputGB]            %p", prefix, setprocInfo.InputGB);
            FAST_LOGD("%s[OutputGB]           %p", prefix, setprocInfo.OutputGB);
            FAST_LOGD("%s[OutputGB_Mask]      %p", prefix, setprocInfo.OutputGB_Mask);
        } else {
            FAST_LOGD("%s[addr_ms]            %p", prefix, setprocInfo.addr_ms);
            FAST_LOGD("%s[addr_md]            %p", prefix, setprocInfo.addr_md);
            FAST_LOGD("%s[addr_mm]            %p", prefix, setprocInfo.addr_mm);

            FAST_LOGD("%s[addr_as]            %p", prefix, setprocInfo.addr_as);
            FAST_LOGD("%s[addr_ad]            %p", prefix, setprocInfo.addr_ad);
            FAST_LOGD("%s[addr_am]            %p", prefix, setprocInfo.addr_am);
        }
    } else {
        FAST_LOGD("%s[addr_mw]            %p %p %p", prefix, setprocInfo.addr_mw[0], setprocInfo.addr_mw[1], setprocInfo.addr_mw[2]);
        FAST_LOGD("%s[addr_aw]            %p %p %p", prefix, setprocInfo.addr_aw[0], setprocInfo.addr_aw[1], setprocInfo.addr_aw[2]);
    }

    FAST_LOG_PRINT;
}

void
N3D_HAL_KERNEL::__logResult(const char *prefix, const STEREO_KERNEL_RESULT_STRUCT &n3dResult)
{
    if(!__LOG_ENABLED) {
        return;
    }

    FAST_LOGD("%s[out_n]       %d %d %d %d %d %d",
            prefix, n3dResult.out_n[0], n3dResult.out_n[1], n3dResult.out_n[2], n3dResult.out_n[3], n3dResult.out_n[4], n3dResult.out_n[5]);
    FAST_LOGD("%s[out_p]       %p %p %p %p %p %p",
            prefix, n3dResult.out_p[0], n3dResult.out_p[1], n3dResult.out_p[2], n3dResult.out_p[3], n3dResult.out_p[4], n3dResult.out_p[5]);

    // MINT32 *verityOutputs = (MINT32*)n3dResult.out_p[STEREO_KERNEL_OUTPUT_VERIFY];
    // if(verityOutputs) {
    //     FAST_LOGD("%s[verify[0]]   %d", prefix, verityOutputs[0]);
    //     FAST_LOGD("%s[verify[1]]   %d", prefix, verityOutputs[1]);
    //     std::string s = std::bitset<4>(verityOutputs[2]).to_string();
    //     FAST_LOGD("%s[verify[2]]   %s", prefix, s.c_str());
    //     for(int i = 3; i < 14; i++) {
    //         FAST_LOGD("%s[verify[%d]]   %d", prefix, i, verityOutputs[i]);
    //     }
    // }

    FAST_LOG_PRINT;
}
