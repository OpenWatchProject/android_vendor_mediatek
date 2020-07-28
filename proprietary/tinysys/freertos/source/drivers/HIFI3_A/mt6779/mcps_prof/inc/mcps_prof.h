/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef MTK_MCPS_PROF_H
#define MTK_MCPS_PROF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define CALL_CYCLE_COUNT_TO_MCPS(cycle) ((((uint32_t)(cycle)) * 50) / 1000000)

enum PROFILE_SCENE {
    SCENE_UL = 0,
    SCENE_DL,
    SCENE_NUM,
};

typedef struct mcps_profile_t {
    float lib_max_ul_mcps;
    float lib_max_dl_mcps;
    float lib_avg_ul_mcps;
    float lib_avg_dl_mcps;
    uint32_t window_frames;
    uint32_t start_ccount;
    uint32_t stop_ccount;
    uint32_t cycle_diff;
    uint32_t cur_mcps;
    uint32_t lib_total_ul_count;
    uint32_t lib_total_dl_count;
    uint32_t lib_peak_ul_count;
    uint32_t lib_peak_dl_count;
    uint32_t lib_total_window_ul_count;
    uint32_t lib_total_window_dl_count;
    uint32_t lib_total_window_ul_mcps;
    uint32_t lib_total_window_dl_mcps;
    uint32_t lib_total_ul_mcps;
    uint32_t lib_total_dl_mcps;
} mcps_profile_t;


void mcps_prof_init(struct mcps_profile_t *mcps_profile);
void mcps_prof_start(struct mcps_profile_t *mcps_profile);
void mcps_prof_stop(struct mcps_profile_t *mcps_profile, uint8_t scene);
void mcps_prof_show(struct mcps_profile_t *mcps_profile);



#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif /* end of MTK_MCPS_PROF_H */
