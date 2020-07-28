#include <FreeRTOS.h>
#include <task.h>

#include "mcps_prof.h"

#include <audio_log_hal.h>

#ifdef __cplusplus
extern "C" {
#endif


void mcps_prof_init(struct mcps_profile_t *mcps_profile) {
    AUD_LOG_D("%s()\n", __FUNCTION__);
    mcps_profile->window_frames = 1;
    mcps_profile->lib_max_ul_mcps = 0;
    mcps_profile->lib_max_dl_mcps = 0;
    mcps_profile->lib_avg_ul_mcps = 0;
    mcps_profile->lib_avg_dl_mcps = 0;
    mcps_profile->lib_total_ul_mcps = 0;
    mcps_profile->lib_total_dl_mcps = 0;
    mcps_profile->lib_total_ul_count = 0;
    mcps_profile->lib_total_dl_count = 0;
    mcps_profile->lib_peak_ul_count = 0;
    mcps_profile->lib_peak_dl_count = 0;
    mcps_profile->lib_total_window_ul_count = 0;
    mcps_profile->lib_total_window_dl_count = 0;
    mcps_profile->lib_total_window_ul_mcps = 0;
    mcps_profile->lib_total_window_dl_mcps = 0;
}

void mcps_prof_start(struct mcps_profile_t *mcps_profile) {
    taskENTER_CRITICAL();
    mcps_profile->start_ccount = xthal_get_ccount();
}

void mcps_prof_stop(struct mcps_profile_t *mcps_profile, uint8_t scene) {
    mcps_profile->stop_ccount = xthal_get_ccount();
    mcps_profile->cycle_diff = mcps_profile->stop_ccount > mcps_profile->start_ccount ?
                               mcps_profile->stop_ccount - mcps_profile->start_ccount : 0xFFFFFFFF - mcps_profile->start_ccount + mcps_profile->stop_ccount;
    mcps_profile->cur_mcps = CALL_CYCLE_COUNT_TO_MCPS(mcps_profile->cycle_diff);
    if (scene == SCENE_UL) {
        /* Update frame frame info */
        mcps_profile->lib_total_ul_count++;
        mcps_profile->lib_total_window_ul_count++;
        mcps_profile->lib_total_window_ul_mcps += mcps_profile->cur_mcps;
        if (mcps_profile->lib_total_window_ul_count == mcps_profile->window_frames) {
            float window_avg_ul_mcps = (float)mcps_profile->lib_total_window_ul_mcps / mcps_profile->window_frames;
            /*  Update max mcps info */
            if (window_avg_ul_mcps > mcps_profile->lib_max_ul_mcps) {
                mcps_profile->lib_max_ul_mcps = window_avg_ul_mcps;
                mcps_profile->lib_peak_ul_count = mcps_profile->lib_total_ul_count;
            }
            AUD_LOG_VV("UL profiling, UL frame reset (window avg = ,%f, window max = ,%f,)\n", window_avg_ul_mcps, mcps_profile->lib_max_ul_mcps);
            mcps_profile->lib_total_window_ul_mcps = 0;
            mcps_profile->lib_total_window_ul_count = 0;
        }

        /* Update UL avg of total frame */
        mcps_profile->lib_total_ul_mcps += mcps_profile->cur_mcps;
        mcps_profile->lib_avg_ul_mcps = (float)mcps_profile->lib_total_ul_mcps / mcps_profile->lib_total_ul_count;

        AUD_LOG_D("UL profiling, process = ,%d, mcps (window: %d/%d), ul_avg = ,%f, total_ul_count = ,%d, lib_max_ul_mcps = ,%f, peak_ul_count = ,%d,\n",
                  mcps_profile->cur_mcps, mcps_profile->lib_total_window_ul_count + 1, mcps_profile->window_frames,
                  mcps_profile->lib_avg_ul_mcps, mcps_profile->lib_total_ul_count, mcps_profile->lib_max_ul_mcps, mcps_profile->lib_peak_ul_count);
    } else if (scene == SCENE_DL) {
        /* Update frame frame info */
        mcps_profile->lib_total_dl_count++;
        mcps_profile->lib_total_window_dl_count++;
        mcps_profile->lib_total_window_dl_mcps += mcps_profile->cur_mcps;
        if (mcps_profile->lib_total_window_dl_count == mcps_profile->window_frames) {
            float window_avg_dl_mcps = (float)mcps_profile->lib_total_window_dl_mcps / mcps_profile->window_frames;
            /* Update max mcps info */
            if (window_avg_dl_mcps > mcps_profile->lib_max_dl_mcps) {
                mcps_profile->lib_max_dl_mcps = window_avg_dl_mcps;
                mcps_profile->lib_peak_dl_count = mcps_profile->lib_total_dl_count;
            }
            AUD_LOG_VV("DL profiling, DL frame reset (window avg = ,%f, window max = ,%f,)\n", window_avg_dl_mcps, mcps_profile->lib_max_dl_mcps);
            mcps_profile->lib_total_window_dl_mcps = 0;
            mcps_profile->lib_total_window_dl_count = 0;
        }

        /* Update DL avg */
        mcps_profile->lib_total_dl_mcps += mcps_profile->cur_mcps;
        mcps_profile->lib_avg_dl_mcps = (float)mcps_profile->lib_total_dl_mcps / mcps_profile->lib_total_dl_count;

        AUD_LOG_D("DL profiling, process = ,%d, mcps (window: %d/%d), dl_avg = ,%f, total_dl_count = ,%d, lib_max_dl_mcps = ,%f, peak_dl_count = ,%d,\n",
                  mcps_profile->cur_mcps, mcps_profile->lib_total_window_dl_count + 1, mcps_profile->window_frames,
                  mcps_profile->lib_avg_dl_mcps, mcps_profile->lib_total_dl_count,
                  mcps_profile->lib_max_dl_mcps, mcps_profile->lib_peak_dl_count);
    } else {
        AUD_LOG_D("WARN: wrong scene when measure process mcps\n");
    }
    taskEXIT_CRITICAL();
}

void mcps_prof_show(struct mcps_profile_t *mcps_profile) {
    if (mcps_profile->lib_total_ul_count != 0) {
        AUD_LOG_D("MCPS profiling result: UL avg/max = %f / %f mcps (total mcps = %d, total count = %d, peak count = %d, window size = %d)\n",
              mcps_profile->lib_avg_ul_mcps, mcps_profile->lib_max_ul_mcps,
              mcps_profile->lib_total_ul_mcps, mcps_profile->lib_total_ul_count, mcps_profile->lib_peak_ul_count, mcps_profile->window_frames);
    }

    if (mcps_profile->lib_total_dl_count != 0) {
        AUD_LOG_D("MCPS profiling result: DL avg/max = %f / %f mcps (total mcps = %d, total count = %d, peak count = %d, window size = %d)\n",
              mcps_profile->lib_avg_dl_mcps, mcps_profile->lib_max_dl_mcps,
              mcps_profile->lib_total_dl_mcps, mcps_profile->lib_total_dl_count, mcps_profile->lib_peak_dl_count, mcps_profile->window_frames);
    }
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

