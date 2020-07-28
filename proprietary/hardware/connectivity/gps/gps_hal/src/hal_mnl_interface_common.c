#include "hal_mnl_interface_common.h"
#include "mtk_lbs_utility.h"
#if 0
#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[gpshal]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gpshal] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gpshal] ERR: ", __VA_ARGS__);
#else
#define LOG_TAG "gpshal_dump"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
#endif
#if 0
void dump_gps_location(gps_location in) {
    LOGD("===== dump_gps_location ====");
    LOGD("flags=0x%x", in.flags);
    LOGD("lat=%f", in.lat);
    LOGD("lng=%f", in.lng);
    LOGD("alt=%f", in.alt);
    LOGD("speed=%f", in.speed);
    LOGD("bearing=%f", in.bearing);
    LOGD("accuracy=%f", in.accuracy);
    LOGD("timestamp=%lld", in.timestamp);
}

void dump_gnss_sv(gnss_sv in) {
    LOGD("===== dump_gnss_sv ====");
    LOGD("prn=%d", in.svid);
    LOGD("constellation=%d", in.constellation);
    LOGD("snr=%f", in.c_n0_dbhz);
    LOGD("elevation=%f", in.elevation);
    LOGD("azimuth=%f", in.azimuth);
    LOGD("flags=%d", in.flags);
}

void dump_gnss_sv_info(gnss_sv_info in) {
    int i = 0;
    LOGD("===== dump_gnss_sv_info ====");
    LOGD("num_svs=%d", in.num_svs);
    for (i = 0; i < in.num_svs; i++) {
        LOGD("i=%d", i);
        dump_gnss_sv(in.sv_list[i]);
    }
}

void dump_gps_measurement(gps_measurement in) {
    LOGD("===== dump_gps_measurement ====");
    LOGD("flags=0x%x", in.flags);
    LOGD("prn=%d", in.prn);
    LOGD("time_offset_ns=%f", in.time_offset_ns);
    LOGD("state=0x%x", in.state);
    LOGD("received_gps_tow_ns=%lld", in.received_gps_tow_ns);
    LOGD("received_gps_tow_uncertainty_ns=%lld", in.received_gps_tow_uncertainty_ns);
    LOGD("c_n0_dbhz=%f", in.c_n0_dbhz);
    LOGD("pseudorange_rate_mps=%f", in.pseudorange_rate_mps);
    LOGD("pseudorange_rate_uncertainty_mps=%f", in.pseudorange_rate_uncertainty_mps);
    LOGD("accumulated_delta_range_state=0x%x", in.accumulated_delta_range_state);
    LOGD("accumulated_delta_range_m=%f", in.accumulated_delta_range_m);
    LOGD("accumulated_delta_range_uncertainty_m=%f", in.accumulated_delta_range_uncertainty_m);
    LOGD("pseudorange_m=%f", in.pseudorange_m);
    LOGD("pseudorange_uncertainty_m=%f", in.pseudorange_uncertainty_m);
    LOGD("code_phase_chips=%f", in.code_phase_chips);
    LOGD("code_phase_uncertainty_chips=%f", in.code_phase_uncertainty_chips);
    LOGD("carrier_frequency_hz=%f", in.carrier_frequency_hz);
    LOGD("carrier_cycles=%lld", in.carrier_cycles);
    LOGD("carrier_phase=%f", in.carrier_phase);
    LOGD("carrier_phase_uncertainty=%f", in.carrier_phase_uncertainty);
    LOGD("loss_of_lock=%d", in.loss_of_lock);
    LOGD("bit_number=%d", in.bit_number);
    LOGD("time_from_last_bit_ms=%d", in.time_from_last_bit_ms);
    LOGD("doppler_shift_hz=%f", in.doppler_shift_hz);
    LOGD("doppler_shift_uncertainty_hz=%f", in.doppler_shift_uncertainty_hz);
    LOGD("multipath_indicator=%d", in.multipath_indicator);
    LOGD("snr_db=%f", in.snr_db);
    LOGD("elevation_deg=%f", in.elevation_deg);
    LOGD("elevation_uncertainty_deg=%f", in.elevation_uncertainty_deg);
    LOGD("azimuth_deg=%f", in.azimuth_deg);
    LOGD("azimuth_uncertainty_deg=%f", in.azimuth_uncertainty_deg);
    LOGD("used_in_fix=%d", in.used_in_fix);
}

void dump_gps_clock(gps_clock in) {
    LOGD("===== dump_gps_clock ====");
    LOGD("flags=0x%x", in.flags);
    LOGD("leap_second=%d", in.leap_second);
    LOGD("type=%d", in.type);
    LOGD("time_ns=%lld", in.time_ns);
    LOGD("time_uncertainty_ns=%f", in.time_uncertainty_ns);
    LOGD("full_bias_ns=%lld", in.full_bias_ns);
    LOGD("bias_ns=%f", in.bias_ns);
    LOGD("bias_uncertainty_ns=%f", in.bias_uncertainty_ns);
    LOGD("drift_nsps=%f", in.drift_nsps);
    LOGD("drift_uncertainty_nsps=%f", in.drift_uncertainty_nsps);
}

void dump_gps_data(gps_data in) {
    size_t i = 0;
    LOGD("===== dump_gps_data ====");
    LOGD("measurement_count=%u", in.measurement_count);
    for (i = 0; i < in.measurement_count; i++) {
        LOGD("i=%u", i);
        dump_gps_measurement(in.measurements[i]);
    }
    dump_gps_clock(in.clock);
}

void dump_gps_nav_msg(gps_nav_msg in) {
    LOGD("===== dump_gps_nav_msg ====");
    LOGD("prn=%d", in.prn);
    LOGD("type=%d", in.type);
    LOGD("status=0x%x", in.status);
    LOGD("message_id=%d", in.message_id);
    LOGD("submessage_id=%d", in.submessage_id);
    LOGD("data_length=%u", in.data_length);
}

void dump_gnss_data(gnss_data in) {
    size_t i = 0;
    LOGD("===== dump_gnss_data ====");
    LOGD("measurement_count=%u", in.measurement_count);
    for (i = 0; i < in.measurement_count; i++) {
        LOGD("i=%u", i);
        dump_gnss_measurement(in.measurements[i]);
    }
    dump_gnss_clock(in.clock);
}

void dump_gnss_measurement(gnss_measurement in) {
    LOGD("===== dump_gnss_measurement ====");
    LOGD("flags=0x%x", in.flags);
    LOGD("svid=%d", in.svid);
    LOGD("constellation=0x%x", in.constellation);
    LOGD("time_offset_ns=%f", in.time_offset_ns);
    LOGD("state=0x%x", in.state);
    LOGD("received_gps_tow_ns=%lld", in.received_sv_time_in_ns);
    LOGD("received_gps_tow_uncertainty_ns=%lld", in.received_sv_time_uncertainty_in_ns);
    LOGD("c_n0_dbhz=%f", in.c_n0_dbhz);
    LOGD("pseudorange_rate_mps=%f", in.pseudorange_rate_mps);
    LOGD("pseudorange_rate_uncertainty_mps=%f", in.pseudorange_rate_uncertainty_mps);
    LOGD("accumulated_delta_range_state=0x%x", in.accumulated_delta_range_state);
    LOGD("accumulated_delta_range_m=%f", in.accumulated_delta_range_m);
    LOGD("accumulated_delta_range_uncertainty_m=%f", in.accumulated_delta_range_uncertainty_m);;
    LOGD("carrier_frequency_hz=%f", in.carrier_frequency_hz);
    LOGD("carrier_cycles=%lld", in.carrier_cycles);
    LOGD("carrier_phase=%f", in.carrier_phase);
    LOGD("carrier_phase_uncertainty=%f", in.carrier_phase_uncertainty);
    LOGD("multipath_indicator=%d", in.multipath_indicator);
    LOGD("snr_db=%f", in.snr_db);
}

void dump_gnss_clock(gnss_clock in) {
    LOGD("===== dump_gnss_clock ====");
    LOGD("flags=0x%x", in.flags);
    LOGD("leap_second=%d", in.leap_second);
    LOGD("time_ns=%lld", in.time_ns);
    LOGD("time_uncertainty_ns=%f", in.time_uncertainty_ns);
    LOGD("full_bias_ns=%lld", in.full_bias_ns);
    LOGD("bias_ns=%f", in.bias_ns);
    LOGD("bias_uncertainty_ns=%f", in.bias_uncertainty_ns);
    LOGD("drift_nsps=%f", in.drift_nsps);
    LOGD("drift_uncertainty_nsps=%f", in.drift_uncertainty_nsps);
    LOGD("hw_clock_discontinuity_count=%d", in.hw_clock_discontinuity_count);
}

void dump_gnss_nav_msg(gnss_nav_msg in) {
    LOGD("===== dump_gnss_nav_msg ====");
    LOGD("svid=%d", in.svid);
    LOGD("type=%d", in.type);
    LOGD("status=0x%x", in.status);
    LOGD("message_id=%d", in.message_id);
    LOGD("submessage_id=%d", in.submessage_id);
    LOGD("data_length=%u", in.data_length);
}
#endif
