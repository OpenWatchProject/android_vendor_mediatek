#ifndef __HAL_MNL_INTERFACE_COMMON_H__
#define __HAL_MNL_INTERFACE_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MNL_BUFF_SIZE           (16 * 1024)
#define HAL_MNL_INTERFACE_VERSION   1

//======================================================
// GPS HAL -> MNLD
//======================================================
#define MTK_HAL2MNL "mtk_hal2mnl"

typedef enum {
    HAL2MNL_HAL_REBOOT                      = 0,
    HAL2MNL_GPS_INIT                        = 101,
    HAL2MNL_GPS_START                       = 102,
    HAL2MNL_GPS_STOP                        = 103,
    HAL2MNL_GPS_CLEANUP                     = 104,
    HAL2MNL_GPS_INJECT_TIME                 = 105,
    HAL2MNL_GPS_INJECT_LOCATION             = 106,
    HAL2MNL_GPS_DELETE_AIDING_DATA          = 107,
    HAL2MNL_GPS_SET_POSITION_MODE           = 108,
    HAL2MNL_DATA_CONN_OPEN                  = 201,
    HAL2MNL_DATA_CONN_OPEN_WITH_APN_IP_TYPE = 202,
    HAL2MNL_DATA_CONN_CLOSED                = 203,
    HAL2MNL_DATA_CONN_FAILED                = 204,
    HAL2MNL_SET_SERVER                      = 301,
    HAL2MNL_SET_REF_LOCATION                = 302,
    HAL2MNL_SET_ID                          = 303,
    HAL2MNL_NI_MESSAGE                      = 401,
    HAL2MNL_NI_RESPOND                      = 402,
    HAL2MNL_UPDATE_NETWORK_STATE            = 501,
    HAL2MNL_UPDATE_NETWORK_AVAILABILITY     = 502,
    HAL2MNL_GPS_MEASUREMENT                 = 601,
    HAL2MNL_GPS_NAVIGATION                  = 602,
    HAL2MNL_VZW_DEBUG                       = 603,
    HAL2MNL_GNSS_CONFIG                     = 604,
    HAL2MNL_SV_BLACKLIST                    = 605,
    HAL2MNL_CORRECTION                      = 606,
    HAL2MNL_NFW_ACCESS                      = 607,
} hal2mnl_cmd;

typedef int gps_pos_mode;
#define GPS_POS_MODE_STANDALONE     0
#define GPS_POS_MODE_MSB            1

typedef int gps_pos_recurrence;
#define GPS_POS_RECURRENCE_PERIODIC     0
#define GPS_POS_RECURRENCE_SINGLE       1

typedef int ni_user_response_type;
#define NI_USER_RESPONSE_ACCEPT     1
#define NI_USER_RESPONSE_DENY       2
#define NI_USER_RESPONSE_NORESP     3

typedef int cell_type;
#define CELL_TYPE_GSM       1
#define CELL_TYPE_UMTS      2

typedef int agps_id_type;
#define AGPS_ID_TYPE_NONE       0
#define AGPS_ID_TYPE_IMSI       1
#define AGPS_ID_TYPE_MSISDN     2

typedef int network_type;
#define NETWORK_TYPE_MOBILE         0
#define NETWORK_TYPE_WIFI           1
#define NETWORK_TYPE_MOBILE_MMS     2
#define NETWORK_TYPE_MOBILE_SUPL    3
#define NETWORK_TYPE_MOBILE_DUN     4
#define NETWORK_TYPE_MOBILE_HIPRI   5
#define NETWORK_TYPE_WIMAX          6

typedef int apn_ip_type;
#define MTK_APN_IP_INVALID          0
#define MTK_APN_IP_IPV4             1
#define MTK_APN_IP_IPV6             2
#define MTK_APN_IP_IPV4V6           3

typedef int agps_type;
#define MTK_AGPS_TYPE_SUPL          1
#define MTK_AGPS_TYPE_C2K           2
#define MTK_AGPS_TYPE_SUPL_EIMS     3
#define MTK_AGPS_TYPE_SUPL_IMS      4

//======================================================
// MNLD -> GPS HAL
//======================================================
#define MTK_MNL2HAL "mtk_mnl2hal"

typedef enum {
    MNL2HAL_MNLD_REBOOT             = 1,
    MNL2HAL_LOCATION                = 101,
    MNL2HAL_GPS_STATUS              = 102,
    MNL2HAL_GPS_SV                  = 103,
    MNL2HAL_NMEA                    = 104,
    MNL2HAL_GPS_CAPABILITIES        = 105,
    MNL2HAL_GPS_MEASUREMENTS        = 106,
    MNL2HAL_GPS_NAVIGATION          = 107,
    MNL2HAL_GNSS_MEASUREMENTS       = 108,
    MNL2HAL_GNSS_NAVIGATION         = 109,
    MNL2HAL_REQUEST_WAKELOCK        = 201,
    MNL2HAL_RELEASE_WAKELOCK        = 202,
    MNL2HAL_REQUEST_UTC_TIME        = 301,
    MNL2HAL_REQUEST_DATA_CONN       = 302,
    MNL2HAL_RELEASE_DATA_CONN       = 303,
    MNL2HAL_REQUEST_NI_NOTIFY       = 304,
    MNL2HAL_REQUEST_SET_ID          = 305,
    MNL2HAL_REQUEST_REF_LOC         = 306,
    MNL2HAL_VZW_DEBUG_OUTPUT        = 307,
    MNL2HAL_UPDATE_GNSS_NAME        = 308,
    MNL2HAL_REQUEST_NLP             = 309,
    MNL2HAL_NFW_NOTIFY              = 310,
} mnl2hal_cmd;

#define MTK_HAL_GNSS_MAX_SVS            64
#define GPS_MAX_MEASUREMENT     32
#define MTK_HAL_GNSS_MAX_MEASUREMENT    64

typedef int gps_location_flags;
#define MTK_GPS_LOCATION_HAS_LAT_LONG   0x0001
#define MTK_GPS_LOCATION_HAS_ALT        0x0002
#define MTK_GPS_LOCATION_HAS_SPEED      0x0004
#define MTK_GPS_LOCATION_HAS_BEARING    0x0008
#define MTK_GPS_LOCATION_HAS_HORIZONTAL_ACCURACY    0x0010
#define MTK_GPS_LOCATION_HAS_VERTICAL_ACCURACY      0x0020
#define MTK_GPS_LOCATION_HAS_SPEED_ACCURACY         0x0040
#define MTK_GPS_LOCATION_HAS_BEARING_ACCURACY       0x0080

typedef int gps_capabilites;
#define GPS_CAP_NONE                    0x0010000
#define GPS_CAP_SCHEDULING              0x0000001
#define GPS_CAP_MSB                     0x0000002
#define GPS_CAP_MSA                     0x0000004
#define GPS_CAP_SINGLE_SHOT             0x0000008
#define GPS_CAP_ON_DEMAND_TIME          0x0000010
#define GPS_CAP_GEOFENCING              0x0000020
#define GPS_CAP_MEASUREMENTS            0x0000040
#define GPS_CAP_NAV_MESSAGES            0x0000080
#define GPS_CAP_LOW_POWER_MODE          0x0000100
#define GPS_CAP_SATELLITE_BLACKLIST     0x0000200
#define GPS_CAP_CORRECTION              0x0000400

typedef int request_setid;
#define REQUEST_SETID_IMSI     (1<<0L)
#define REQUEST_SETID_MSISDN   (1<<1L)

typedef int request_refloc;
#define REQUEST_REFLOC_CELLID  (1<<0L)
#define REQUEST_REFLOC_MAC     (1<<1L)   // not ready

typedef short gps_clock_flags;
#define GPS_CLK_HAS_LEAP_SECOND               (1<<0)
#define GPS_CLK_HAS_TIME_UNCERTAINTY          (1<<1)
#define GPS_CLK_HAS_FULL_BIAS                 (1<<2)
#define GPS_CLK_HAS_BIAS                      (1<<3)
#define GPS_CLK_HAS_BIAS_UNCERTAINTY          (1<<4)
#define GPS_CLK_HAS_DRIFT                     (1<<5)
#define GPS_CLK_HAS_DRIFT_UNCERTAINTY         (1<<6)

typedef char gps_clock_type;
#define GPS_CLOCK_TYPE_UNKNOWN                  0
#define GPS_CLOCK_TYPE_LOCAL_HW_TIME            1
#define GPS_CLOCK_TYPE_GPS_TIME                 2

typedef int gps_measurement_flags;
#define GPS_MEASUREMENT_HAS_SNR                               (1<<0)
#define GPS_MEASUREMENT_HAS_ELEVATION                         (1<<1)
#define GPS_MEASUREMENT_HAS_ELEVATION_UNCERTAINTY             (1<<2)
#define GPS_MEASUREMENT_HAS_AZIMUTH                           (1<<3)
#define GPS_MEASUREMENT_HAS_AZIMUTH_UNCERTAINTY               (1<<4)
#define GPS_MEASUREMENT_HAS_PSEUDORANGE                       (1<<5)
#define GPS_MEASUREMENT_HAS_PSEUDORANGE_UNCERTAINTY           (1<<6)
#define GPS_MEASUREMENT_HAS_CODE_PHASE                        (1<<7)
#define GPS_MEASUREMENT_HAS_CODE_PHASE_UNCERTAINTY            (1<<8)
#define GPS_MEASUREMENT_HAS_CARRIER_FREQUENCY                 (1<<9)
#define GPS_MEASUREMENT_HAS_CARRIER_CYCLES                    (1<<10)
#define GPS_MEASUREMENT_HAS_CARRIER_PHASE                     (1<<11)
#define GPS_MEASUREMENT_HAS_CARRIER_PHASE_UNCERTAINTY         (1<<12)
#define GPS_MEASUREMENT_HAS_BIT_NUMBER                        (1<<13)
#define GPS_MEASUREMENT_HAS_TIME_FROM_LAST_BIT                (1<<14)
#define GPS_MEASUREMENT_HAS_DOPPLER_SHIFT                     (1<<15)
#define GPS_MEASUREMENT_HAS_DOPPLER_SHIFT_UNCERTAINTY         (1<<16)
#define GPS_MEASUREMENT_HAS_USED_IN_FIX                       (1<<17)

typedef short gps_measurement_state;
#define GPS_MEASUREMENT_STATE_UNKNOWN                   0
#define GPS_MEASUREMENT_STATE_CODE_LOCK             (1<<0)
#define GPS_MEASUREMENT_STATE_BIT_SYNC              (1<<1)
#define GPS_MEASUREMENT_STATE_SUBFRAME_SYNC         (1<<2)
#define GPS_MEASUREMENT_STATE_TOW_DECODED           (1<<3)
#define GPS_MEASUREMENT_STATE_MSEC_AMBIGUOUS        (1<<4)

typedef short gps_accumulated_delta_range_state;
#define GPS_ADR_STATE_UNKNOWN                       0
#define GPS_ADR_STATE_VALID                     (1<<0)
#define GPS_ADR_STATE_RESET                     (1<<1)
#define GPS_ADR_STATE_CYCLE_SLIP                (1<<2)

typedef char gps_loss_of_lock;
#define GPS_LOSS_OF_LOCK_UNKNOWN                            0
#define GPS_LOSS_OF_LOCK_OK                                 1
#define GPS_LOSS_OF_LOCK_CYCLE_SLIP                         2

typedef char gps_multipath_indicator;
#define GPS_MULTIPATH_INDICATOR_UNKNOWN                 0
#define GPS_MULTIPATH_INDICATOR_DETECTED                1
#define GPS_MULTIPATH_INDICATOR_NOT_USED                2

typedef char gps_nav_msg_type;
#define GPS_NAV_MSG_TYPE_UNKNOWN         0
#define GPS_NAV_MSG_TYPE_L1CA            1
#define GPS_NAV_MSG_TYPE_L2CNAV          2
#define GPS_NAV_MSG_TYPE_L5CNAV          3
#define GPS_NAV_MSG_TYPE_CNAV2           4

typedef short nav_msg_status;
#define NAV_MSG_STATUS_UNKONW              0
#define NAV_MSG_STATUS_PARITY_PASSED   (1<<0)
#define NAV_MSG_STATUS_PARITY_REBUILT  (1<<1)

typedef int gps_status;
#define MTK_GPS_STATUS_SESSION_BEGIN        1
#define MTK_GPS_STATUS_SESSION_END          2
#define MTK_GPS_STATUS_SESSION_ENGINE_ON    3
#define MTK_GPS_STATUS_SESSION_ENGINE_OFF   4

typedef int agps_ni_type;
#define AGPS_NI_TYPE_VOICE              1
#define AGPS_NI_TYPE_UMTS_SUPL          2
#define AGPS_NI_TYPE_UMTS_CTRL_PLANE    3
#define AGPS_NI_TYPE_EMERGENCY_SUPL     4

typedef int agps_notify_type;
#define AGPS_NOTIFY_TYPE_NONE                       0
#define AGPS_NOTIFY_TYPE_NOTIFY_ONLY                1
#define AGPS_NOTIFY_TYPE_NOTIFY_ALLOW_NO_ANSWER     2
#define AGPS_NOTIFY_TYPE_NOTIFY_DENY_NO_ANSWER      3
#define AGPS_NOTIFY_TYPE_PRIVACY                    4

typedef int ni_encoding_type;
#define NI_ENCODING_TYPE_NONE   0
#define NI_ENCODING_TYPE_GSM7   1
#define NI_ENCODING_TYPE_UTF8   2
#define NI_ENCODING_TYPE_UCS2   3

typedef struct {
    gps_location_flags flags;
    double lat;
    double lng;
    double alt;
    float speed;
    float bearing;
    float h_accuracy;  //horizontal
    float v_accuracy;  //vertical
    float s_accuracy;  //spedd
    float b_accuracy;  //bearing
    int64_t timestamp;
} gps_location;

typedef struct {
    int16_t   svid;
    uint8_t   constellation;
    float     c_n0_dbhz;
    float     elevation;
    float     azimuth;
    uint8_t   flags;
    float     carrier_frequency;
} gnss_sv;

typedef struct {
    int  num_svs;
    gnss_sv  sv_list[MTK_HAL_GNSS_MAX_SVS];
} gnss_sv_info;

typedef struct {
    int prn;
    bool has_ephemeris_data;
    bool has_almanac_data;
    bool used_in_fix;
} NmeaCash;

typedef uint16_t MTK_HAL_NavigationMessageStatus;
#define MTK_NAV_MESSAGE_STATUS_UNKNOWN              0
#define MTK_NAV_MESSAGE_STATUS_PARITY_PASSED   (1<<0)
#define MTK_NAV_MESSAGE_STATUS_PARITY_REBUILT  (1<<1)

typedef int16_t MTK_HAL_GnssNavigationMessageType;
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_UNKNOWN       0
/** GPS L1 C/A message contained in the structure.  */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GPS_L1CA      0x0101
/** GPS L2-CNAV message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GPS_L2CNAV    0x0102
/** GPS L5-CNAV message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GPS_L5CNAV    0x0103
/** GPS CNAV-2 message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GPS_CNAV2     0x0104
/** Glonass L1 CA message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GLO_L1CA      0x0301
/** Beidou D1 message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_BDS_D1        0x0501
/** Beidou D2 message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_BDS_D2        0x0502
/** Galileo I/NAV message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GAL_I         0x0601
/** Galileo F/NAV message contained in the structure. */
#define MTK_GNSS_NAVIGATION_MESSAGE_TYPE_GAL_F         0x0602

typedef uint8_t MTK_HAL_GnssMultipathIndicator;
/** The indicator is not available or unknown. */
#define MTK_GNSS_MULTIPATH_INDICATOR_UNKNOWN                 0
/** The measurement is indicated to be affected by multipath. */
#define MTK_GNSS_MULTIPATH_INDICATOR_PRESENT                 1
/** The measurement is indicated to be not affected by multipath. */
#define MTK_GNSS_MULTIPATH_INDICATOR_NOT_PRESENT             2

typedef uint16_t MTK_HAL_GnssAccumulatedDeltaRangeState;
#define MTK_GNSS_ADR_STATE_UNKNOWN                       0
#define MTK_GNSS_ADR_STATE_VALID                     (1<<0)
#define MTK_GNSS_ADR_STATE_RESET                     (1<<1)
#define MTK_GNSS_ADR_STATE_CYCLE_SLIP                (1<<2)

typedef uint32_t MTK_HAL_GnssMeasurementState;
#define MTK_GNSS_MEASUREMENT_STATE_UNKNOWN                   0
#define MTK_GNSS_MEASUREMENT_STATE_CODE_LOCK             (1<<0)
#define MTK_GNSS_MEASUREMENT_STATE_BIT_SYNC              (1<<1)
#define MTK_GNSS_MEASUREMENT_STATE_SUBFRAME_SYNC         (1<<2)
#define MTK_GNSS_MEASUREMENT_STATE_TOW_DECODED           (1<<3)
#define MTK_GNSS_MEASUREMENT_STATE_MSEC_AMBIGUOUS        (1<<4)
#define MTK_GNSS_MEASUREMENT_STATE_SYMBOL_SYNC           (1<<5)
#define MTK_GNSS_MEASUREMENT_STATE_GLO_STRING_SYNC       (1<<6)
#define MTK_GNSS_MEASUREMENT_STATE_GLO_TOD_DECODED       (1<<7)
#define MTK_GNSS_MEASUREMENT_STATE_BDS_D2_BIT_SYNC       (1<<8)
#define MTK_GNSS_MEASUREMENT_STATE_BDS_D2_SUBFRAME_SYNC  (1<<9)
#define MTK_GNSS_MEASUREMENT_STATE_GAL_E1BC_CODE_LOCK    (1<<10)
#define MTK_GNSS_MEASUREMENT_STATE_GAL_E1C_2ND_CODE_LOCK (1<<11)
#define MTK_GNSS_MEASUREMENT_STATE_GAL_E1B_PAGE_SYNC     (1<<12)
#define MTK_GNSS_MEASUREMENT_STATE_SBAS_SYNC             (1<<13)

typedef uint8_t  MTK_HAL_GnssConstellationType;
#define MTK_GNSS_CONSTELLATION_UNKNOWN      0
#define MTK_GNSS_CONSTELLATION_GPS          1
#define MTK_GNSS_CONSTELLATION_SBAS         2
#define MTK_GNSS_CONSTELLATION_GLONASS      3
#define MTK_GNSS_CONSTELLATION_QZSS         4
#define MTK_GNSS_CONSTELLATION_BEIDOU       5
#define MTK_GNSS_CONSTELLATION_GALILEO      6
#define MTK_GNSS_CONSTELLATION_IRNSS        7

typedef uint32_t MTK_HAL_GnssMeasurementFlags;
/** A valid 'snr' is stored in the data structure. */
#define MTK_GNSS_MEASUREMENT_HAS_SNR                               (1<<0)
/** A valid 'carrier frequency' is stored in the data structure. */
#define MTK_GNSS_MEASUREMENT_HAS_CARRIER_FREQUENCY                 (1<<9)
/** A valid 'carrier cycles' is stored in the data structure. */
#define MTK_GNSS_MEASUREMENT_HAS_CARRIER_CYCLES                    (1<<10)
/** A valid 'carrier phase' is stored in the data structure. */
#define MTK_GNSS_MEASUREMENT_HAS_CARRIER_PHASE                     (1<<11)
/** A valid 'carrier phase uncertainty' is stored in the data structure. */
#define MTK_GNSS_MEASUREMENT_HAS_CARRIER_PHASE_UNCERTAINTY         (1<<12)

typedef uint16_t MTK_HAL_GnssClockFlags;
/** A valid 'leap second' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_LEAP_SECOND               (1<<0)
/** A valid 'time uncertainty' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_TIME_UNCERTAINTY          (1<<1)
/** A valid 'full bias' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_FULL_BIAS                 (1<<2)
/** A valid 'bias' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_BIAS                      (1<<3)
/** A valid 'bias uncertainty' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_BIAS_UNCERTAINTY          (1<<4)
/** A valid 'drift' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_DRIFT                     (1<<5)
/** A valid 'drift uncertainty' is stored in the data structure. */
#define MTK_GNSS_CLOCK_HAS_DRIFT_UNCERTAINTY         (1<<6)

typedef struct {
    /** A set of flags indicating the validity of the fields in this data structure. */
    gps_clock_flags flags;

    /**
     * Leap second data.
     * The sign of the value is defined by the following equation:
     *      utc_time_ns = time_ns + (full_bias_ns + bias_ns) - leap_second * 1,000,000,000
     *
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_LEAP_SECOND.
     */
    short leap_second;

    /**
     * Indicates the type of time reported by the 'time_ns' field.
     */
    gps_clock_type type;

    /**
     * The GPS receiver internal clock value. This can be either the local hardware clock value
     * (GPS_CLOCK_TYPE_LOCAL_HW_TIME), or the current GPS time derived inside GPS receiver
     * (GPS_CLOCK_TYPE_GPS_TIME). The field 'type' defines the time reported.
     *
     * For local hardware clock, this value is expected to be monotonically increasing during
     * the reporting session. The real GPS time can be derived by compensating the 'full bias'
     * (when it is available) from this value.
     *
     * For GPS time, this value is expected to be the best estimation of current GPS time that GPS
     * receiver can achieve. Set the 'time uncertainty' appropriately when GPS time is specified.
     *
     * Sub-nanosecond accuracy can be provided by means of the 'bias' field.
     * The value contains the 'time uncertainty' in it.
     */
    int64_t time_ns;

    /**
     * 1-Sigma uncertainty associated with the clock's time in nanoseconds.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * This value should be set if GPS_CLOCK_TYPE_GPS_TIME is set.
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_TIME_UNCERTAINTY.
     */
    double time_uncertainty_ns;

    /**
     * The difference between hardware clock ('time' field) inside GPS receiver and the true GPS
     * time since 0000Z, January 6, 1980, in nanoseconds.
     * This value is used if and only if GPS_CLOCK_TYPE_LOCAL_HW_TIME is set, and GPS receiver
     * has solved the clock for GPS time.
     * The caller is responsible for using the 'bias uncertainty' field for quality check.
     *
     * The sign of the value is defined by the following equation:
     *      true time (GPS time) = time_ns + (full_bias_ns + bias_ns)
     *
     * This value contains the 'bias uncertainty' in it.
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_FULL_BIAS.

     */
    int64_t full_bias_ns;

    /**
     * Sub-nanosecond bias.
     * The value contains the 'bias uncertainty' in it.
     *
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_BIAS.
     */
    double bias_ns;

    /**
     * 1-Sigma uncertainty associated with the clock's bias in nanoseconds.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_BIAS_UNCERTAINTY.
     */
    double bias_uncertainty_ns;

    /**
     * The clock's drift in nanoseconds (per second).
     * A positive value means that the frequency is higher than the nominal frequency.
     *
     * The value contains the 'drift uncertainty' in it.
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_DRIFT.
     *
     * If GpsMeasurement's 'flags' field contains GPS_MEASUREMENT_HAS_UNCORRECTED_PSEUDORANGE_RATE,
     * it is encouraged that this field is also provided.
     */
    double drift_nsps;

    /**
     * 1-Sigma uncertainty associated with the clock's drift in nanoseconds (per second).
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available 'flags' must contain GPS_CLOCK_HAS_DRIFT_UNCERTAINTY.
     */
    double drift_uncertainty_nsps;
} gps_clock;

/**
 * Represents an estimate of the GPS clock time.
 */
typedef struct {
    /**
     * A set of flags indicating the validity of the fields in this data
     * structure.
     */
    MTK_HAL_GnssClockFlags flags;

    /**
     * Leap second data.
     * The sign of the value is defined by the following equation:
     *      utc_time_ns = time_ns - (full_bias_ns + bias_ns) - leap_second *
     *      1,000,000,000
     *
     * If the data is available 'flags' must contain GNSS_CLOCK_HAS_LEAP_SECOND.
     */
    int16_t leap_second;

    /**
     * The GNSS receiver internal clock value. This is the local hardware clock
     * value.
     *
     * For local hardware clock, this value is expected to be monotonically
     * increasing while the hardware clock remains power on. (For the case of a
     * HW clock that is not continuously on, see the
     * hw_clock_discontinuity_count field). The receiver's estimate of GPS time
     * can be derived by substracting the sum of full_bias_ns and bias_ns (when
     * available) from this value.
     *
     * This GPS time is expected to be the best estimate of current GPS time
     * that GNSS receiver can achieve.
     *
     * Sub-nanosecond accuracy can be provided by means of the 'bias_ns' field.
     * The value contains the 'time uncertainty' in it.
     *
     * This field is mandatory.
     */
    int64_t time_ns;

    /**
     * 1-Sigma uncertainty associated with the clock's time in nanoseconds.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available, 'flags' must contain
     * GNSS_CLOCK_HAS_TIME_UNCERTAINTY. This value is effectively zero (it is
     * the reference local clock, by which all other times and time
     * uncertainties are measured.)  (And thus this field can be not provided,
     * per GNSS_CLOCK_HAS_TIME_UNCERTAINTY flag, or provided & set to 0.)
     */
    double time_uncertainty_ns;

    /**
     * The difference between hardware clock ('time' field) inside GPS receiver
     * and the true GPS time since 0000Z, January 6, 1980, in nanoseconds.
     *
     * The sign of the value is defined by the following equation:
     *      local estimate of GPS time = time_ns - (full_bias_ns + bias_ns)
     *
     * This value is mandatory if the receiver has estimated GPS time. If the
     * computed time is for a non-GPS constellation, the time offset of that
     * constellation to GPS has to be applied to fill this value. The error
     * estimate for the sum of this and the bias_ns is the bias_uncertainty_ns,
     * and the caller is responsible for using this uncertainty (it can be very
     * large before the GPS time has been solved for.) If the data is available
     * 'flags' must contain GNSS_CLOCK_HAS_FULL_BIAS.
     */
    int64_t full_bias_ns;

    /**
     * Sub-nanosecond bias.
     * The error estimate for the sum of this and the full_bias_ns is the
     * bias_uncertainty_ns
     *
     * If the data is available 'flags' must contain GNSS_CLOCK_HAS_BIAS. If GPS
     * has computed a position fix. This value is mandatory if the receiver has
     * estimated GPS time.
     */
    double bias_ns;

    /**
     * 1-Sigma uncertainty associated with the local estimate of GPS time (clock
     * bias) in nanoseconds. The uncertainty is represented as an absolute
     * (single sided) value.
     *
     * If the data is available 'flags' must contain
     * GNSS_CLOCK_HAS_BIAS_UNCERTAINTY. This value is mandatory if the receiver
     * has estimated GPS time.
     */
    double bias_uncertainty_ns;

    /**
     * The clock's drift in nanoseconds (per second).
     *
     * A positive value means that the frequency is higher than the nominal
     * frequency, and that the (full_bias_ns + bias_ns) is growing more positive
     * over time.
     *
     * The value contains the 'drift uncertainty' in it.
     * If the data is available 'flags' must contain GNSS_CLOCK_HAS_DRIFT.
     *
     * This value is mandatory if the receiver has estimated GNSS time
     */
    double drift_nsps;

    /**
     * 1-Sigma uncertainty associated with the clock's drift in nanoseconds (per second).
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available 'flags' must contain
     * GNSS_CLOCK_HAS_DRIFT_UNCERTAINTY. If GPS has computed a position fix this
     * field is mandatory and must be populated.
     */
    double drift_uncertainty_nsps;

    /**
     * When there are any discontinuities in the HW clock, this field is
     * mandatory.
     *
     * A "discontinuity" is meant to cover the case of a switch from one source
     * of clock to another.  A single free-running crystal oscillator (XO)
     * should generally not have any discontinuities, and this can be set and
     * left at 0.
     *
     * If, however, the time_ns value (HW clock) is derived from a composite of
     * sources, that is not as smooth as a typical XO, or is otherwise stopped &
     * restarted, then this value shall be incremented each time a discontinuity
     * occurs.  (E.g. this value may start at zero at device boot-up and
     * increment each time there is a change in clock continuity. In the
     * unlikely event that this value reaches full scale, rollover (not
     * clamping) is required, such that this value continues to change, during
     * subsequent discontinuity events.)
     *
     * While this number stays the same, between GnssClock reports, it can be
     * safely assumed that the time_ns value has been running continuously, e.g.
     * derived from a single, high quality clock (XO like, or better, that's
     * typically used during continuous GNSS signal sampling.)
     *
     * It is expected, esp. during periods where there are few GNSS signals
     * available, that the HW clock be discontinuity-free as long as possible,
     * as this avoids the need to use (waste) a GNSS measurement to fully
     * re-solve for the GPS clock bias and drift, when using the accompanying
     * measurements, from consecutive GnssData reports.
     */
    uint32_t hw_clock_discontinuity_count;
} gnss_clock;

typedef struct {
    /** A set of flags indicating the validity of the fields in this data structure. */
    gps_measurement_flags flags;

    /**
     * Pseudo-random number in the range of [1, 32]
     * This is a Mandatory value.
     */
    int8_t prn;

    /**
     * Time offset at which the measurement was taken in nanoseconds.
     * The reference receiver's time is specified by GpsData::clock::time_ns and should be
     * interpreted in the same way as indicated by GpsClock::type.
     *
     * The sign of time_offset_ns is given by the following equation:
     *      measurement time = GpsClock::time_ns + time_offset_ns
     *
     * It provides an individual time-stamp for the measurement, and allows sub-nanosecond accuracy.
     * This is a Mandatory value.
     */
    double time_offset_ns;

    /**
     * Per satellite sync state. It represents the current sync state for the associated satellite.
     * Based on the sync state, the 'received GPS tow' field should be interpreted accordingly.
     * This is a Mandatory value.
     */
    gps_measurement_state state;

    /**
     * Received GPS Time-of-Week at the measurement time, in nanoseconds.
     * The value is relative to the beginning of the current GPS week.
     *
     * Given the highest sync state that can be achieved, per each satellite, valid range for
     * this field can be:
     *     Searching       : [ 0       ]   : GPS_MEASUREMENT_STATE_UNKNOWN
     *     C/A code lock   : [ 0   1ms ]   : GPS_MEASUREMENT_STATE_CODE_LOCK is set
     *     Bit sync        : [ 0  20ms ]   : GPS_MEASUREMENT_STATE_BIT_SYNC is set
     *     Subframe sync   : [ 0    6s ]   : GPS_MEASUREMENT_STATE_SUBFRAME_SYNC is set
     *     TOW decoded     : [ 0 1week ]   : GPS_MEASUREMENT_STATE_TOW_DECODED is set
     *
     * However, if there is any ambiguity in integer millisecond,
     * GPS_MEASUREMENT_STATE_MSEC_AMBIGUOUS should be set accordingly, in the 'state' field.
     * This value must be populated if 'state' != GPS_MEASUREMENT_STATE_UNKNOWN.
     */
    int64_t received_gps_tow_ns;

    /**
     * 1-Sigma uncertainty of the Received GPS Time-of-Week in nanoseconds.
     * This value must be populated if 'state' != GPS_MEASUREMENT_STATE_UNKNOWN.
     */
    int64_t received_gps_tow_uncertainty_ns;

    /**
     * Carrier-to-noise density in dB-Hz, in the range [0, 63].
     * It contains the measured C/N0 value for the signal at the antenna input.
     * This is a Mandatory value.
     */
    double c_n0_dbhz;

    /**
     * Pseudorange rate at the timestamp in m/s.
     * The value also includes the effects of the receiver clock frequency and satellite clock
     * frequency errors.
     *
     * The value includes the 'pseudorange rate uncertainty' in it.
     * A positive value indicates that the pseudorange is getting larger.
     * This is a Mandatory value.
     */
    double pseudorange_rate_mps;

    /**
     * 1-Sigma uncertainty of the pseudurange rate in m/s.
     * The uncertainty is represented as an absolute (single sided) value.
     * This is a Mandatory value.
     */
    double pseudorange_rate_uncertainty_mps;

    /**
     * Accumulated delta range's state. It indicates whether ADR is reset or there is a cycle slip
     * (indicating loss of lock).
     * This is a Mandatory value.
     */
    gps_accumulated_delta_range_state accumulated_delta_range_state;

    /**
     * Accumulated delta range since the last channel reset in meters.
     * A positive value indicates that the SV is moving away from the receiver.
     *
     * The sign of the 'accumulated delta range' and its relation to the sign of 'carrier phase'
     * is given by the equation:
     *          accumulated delta range = -k * carrier phase    (where k is a constant)
     *
     * This value must be populated if 'accumulated delta range state' != GPS_ADR_STATE_UNKNOWN.
     * However, it is expected that the data is only accurate when:
     *      'accumulated delta range state' == GPS_ADR_STATE_VALID.
     */
    double accumulated_delta_range_m;

    /**
     * 1-Sigma uncertainty of the accumulated delta range in meters.
     * The data is available if 'accumulated delta range state' != GPS_ADR_STATE_UNKNOWN.
     */
    double accumulated_delta_range_uncertainty_m;

    /**
     * Best derived Pseudorange by the chip-set, in meters.
     * The value contains the 'pseudorange uncertainty' in it.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_PSEUDORANGE.
     */
    double pseudorange_m;

    /**
     * 1-Sigma uncertainty of the pseudorange in meters.
     * The value contains the 'pseudorange' and 'clock' uncertainty in it.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_PSEUDORANGE_UNCERTAINTY.
     */
    double pseudorange_uncertainty_m;

    /**
     * A fraction of the current C/A code cycle, in the range [0.0, 1023.0]
     * This value contains the time (in Chip units) since the last C/A code cycle (GPS Msec epoch).
     *
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     * The value contains the 'code-phase uncertainty' in it.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CODE_PHASE.
     */
    double code_phase_chips;

    /**
     * 1-Sigma uncertainty of the code-phase, in a fraction of chips.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CODE_PHASE_UNCERTAINTY.
     */
    double code_phase_uncertainty_chips;

    /**
     * Carrier frequency at which codes and messages are modulated, it can be L1 or L2.
     * If the field is not set, the carrier frequency is assumed to be L1.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CARRIER_FREQUENCY.
     */
    float carrier_frequency_hz;

    /**
     * The number of full carrier cycles between the satellite and the receiver.
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CARRIER_CYCLES.
     */
    int64_t carrier_cycles;

    /**
     * The RF phase detected by the receiver, in the range [0.0, 1.0].
     * This is usually the fractional part of the complete carrier phase measurement.
     *
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     * The value contains the 'carrier-phase uncertainty' in it.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CARRIER_PHASE.
     */
    double carrier_phase;

    /**
     * 1-Sigma uncertainty of the carrier-phase.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_CARRIER_PHASE_UNCERTAINTY.
     */
    double carrier_phase_uncertainty;

    /**
     * An enumeration that indicates the 'loss of lock' state of the event.
     */
    gps_loss_of_lock loss_of_lock;

    /**
     * The number of GPS bits transmitted since Sat-Sun midnight (GPS week).
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_BIT_NUMBER.
     */
    int32_t bit_number;

    /**
     * The elapsed time since the last received bit in milliseconds, in the range [0, 20]
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_TIME_FROM_LAST_BIT.
     */
    int16_t time_from_last_bit_ms;

    /**
     * Doppler shift in Hz.
     * A positive value indicates that the SV is moving toward the receiver.
     *
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     * The value contains the 'doppler shift uncertainty' in it.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_DOPPLER_SHIFT.
     */
    double doppler_shift_hz;

    /**
     * 1-Sigma uncertainty of the doppler shift in Hz.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_DOPPLER_SHIFT_UNCERTAINTY.
     */
    double doppler_shift_uncertainty_hz;

    /**
     * An enumeration that indicates the 'multipath' state of the event.
     */
    gps_multipath_indicator multipath_indicator;

    /**
     * Signal-to-noise ratio in dB.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_SNR.
     */
    double snr_db;

    /**
     * Elevation in degrees, the valid range is [-90, 90].
     * The value contains the 'elevation uncertainty' in it.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_ELEVATION.
     */
    double elevation_deg;

    /**
     * 1-Sigma uncertainty of the elevation in degrees, the valid range is [0, 90].
     * The uncertainty is represented as the absolute (single sided) value.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_ELEVATION_UNCERTAINTY.
     */
    double elevation_uncertainty_deg;

    /**
     * Azimuth in degrees, in the range [0, 360).
     * The value contains the 'azimuth uncertainty' in it.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_AZIMUTH.
     *  */
    double azimuth_deg;

    /**
     * 1-Sigma uncertainty of the azimuth in degrees, the valid range is [0, 180].
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_AZIMUTH_UNCERTAINTY.
     */
    double azimuth_uncertainty_deg;

    /**
     * Whether the GPS represented by the measurement was used for computing the most recent fix.
     * If the data is available, 'flags' must contain GPS_MEASUREMENT_HAS_USED_IN_FIX.
     */
    bool used_in_fix;
} gps_measurement;

/**
 * Represents a GNSS Measurement, it contains raw and computed information.
 *
 * Independence - All signal measurement information (e.g. sv_time,
 * pseudorange_rate, multipath_indicator) reported in this struct should be
 * based on GNSS signal measurements only. You may not synthesize measurements
 * by calculating or reporting expected measurements based on known or estimated
 * position, velocity, or time.
 */
typedef struct {
    /** A set of flags indicating the validity of the fields in this data structure. */
    MTK_HAL_GnssMeasurementFlags flags;

    /**
     * Satellite vehicle ID number, as defined in GnssSvInfo::svid
     * This is a mandatory value.
     */
    int16_t svid;

    /**
     * Defines the constellation of the given SV. Value should be one of those
     * GNSS_CONSTELLATION_* constants
     */
    MTK_HAL_GnssConstellationType constellation;

    /**
     * Time offset at which the measurement was taken in nanoseconds.
     * The reference receiver's time is specified by GpsData::clock::time_ns and should be
     * interpreted in the same way as indicated by GpsClock::type.
     *
     * The sign of time_offset_ns is given by the following equation:
     *      measurement time = GpsClock::time_ns + time_offset_ns
     *
     * It provides an individual time-stamp for the measurement, and allows sub-nanosecond accuracy.
     * This is a mandatory value.
     */
    double time_offset_ns;

    /**
     * Per satellite sync state. It represents the current sync state for the associated satellite.
     * Based on the sync state, the 'received GPS tow' field should be interpreted accordingly.
     *
     * This is a mandatory value.
     */
    MTK_HAL_GnssMeasurementState state;

    /**
     * The received GNSS Time-of-Week at the measurement time, in nanoseconds.
     * Ensure that this field is independent (see comment at top of
     * GnssMeasurement struct.)
     *
     * For GPS & QZSS, this is:
     *   Received GPS Time-of-Week at the measurement time, in nanoseconds.
     *   The value is relative to the beginning of the current GPS week.
     *
     *   Given the highest sync state that can be achieved, per each satellite, valid range
     *   for this field can be:
     *     Searching       : [ 0       ]   : GNSS_MEASUREMENT_STATE_UNKNOWN
     *     C/A code lock   : [ 0   1ms ]   : GNSS_MEASUREMENT_STATE_CODE_LOCK is set
     *     Bit sync        : [ 0  20ms ]   : GNSS_MEASUREMENT_STATE_BIT_SYNC is set
     *     Subframe sync   : [ 0    6s ]   : GNSS_MEASUREMENT_STATE_SUBFRAME_SYNC is set
     *     TOW decoded     : [ 0 1week ]   : GNSS_MEASUREMENT_STATE_TOW_DECODED is set
     *
     *   Note well: if there is any ambiguity in integer millisecond,
     *   GNSS_MEASUREMENT_STATE_MSEC_AMBIGUOUS should be set accordingly, in the 'state' field.
     *
     *   This value must be populated if 'state' != GNSS_MEASUREMENT_STATE_UNKNOWN.
     *
     * For Glonass, this is:
     *   Received Glonass time of day, at the measurement time in nanoseconds.
     *
     *   Given the highest sync state that can be achieved, per each satellite, valid range for
     *   this field can be:
     *     Searching       : [ 0       ]   : GNSS_MEASUREMENT_STATE_UNKNOWN
     *     C/A code lock   : [ 0   1ms ]   : GNSS_MEASUREMENT_STATE_CODE_LOCK is set
     *     Symbol sync     : [ 0  10ms ]   : GNSS_MEASUREMENT_STATE_SYMBOL_SYNC is set
     *     Bit sync        : [ 0  20ms ]   : GNSS_MEASUREMENT_STATE_BIT_SYNC is set
     *     String sync     : [ 0    2s ]   : GNSS_MEASUREMENT_STATE_GLO_STRING_SYNC is set
     *     Time of day     : [ 0  1day ]   : GNSS_MEASUREMENT_STATE_GLO_TOD_DECODED is set
     *
     * For Beidou, this is:
     *   Received Beidou time of week, at the measurement time in nanoseconds.
     *
     *   Given the highest sync state that can be achieved, per each satellite, valid range for
     *   this field can be:
     *     Searching    : [ 0       ] : GNSS_MEASUREMENT_STATE_UNKNOWN
     *     C/A code lock: [ 0   1ms ] : GNSS_MEASUREMENT_STATE_CODE_LOCK is set
     *     Bit sync (D2): [ 0   2ms ] : GNSS_MEASUREMENT_STATE_BDS_D2_BIT_SYNC is set
     *     Bit sync (D1): [ 0  20ms ] : GNSS_MEASUREMENT_STATE_BIT_SYNC is set
     *     Subframe (D2): [ 0  0.6s ] : GNSS_MEASUREMENT_STATE_BDS_D2_SUBFRAME_SYNC is set
     *     Subframe (D1): [ 0    6s ] : GNSS_MEASUREMENT_STATE_SUBFRAME_SYNC is set
     *     Time of week : [ 0 1week ] : GNSS_MEASUREMENT_STATE_TOW_DECODED is set
     *
     * For Galileo, this is:
     *   Received Galileo time of week, at the measurement time in nanoseconds.
     *
     *     E1BC code lock   : [ 0   4ms ]   : GNSS_MEASUREMENT_STATE_GAL_E1BC_CODE_LOCK is set
     *     E1C 2nd code lock: [ 0 100ms ]   :
     *     GNSS_MEASUREMENT_STATE_GAL_E1C_2ND_CODE_LOCK is set
     *
     *     E1B page    : [ 0    2s ] : GNSS_MEASUREMENT_STATE_GAL_E1B_PAGE_SYNC is set
     *     Time of week: [ 0 1week ] : GNSS_MEASUREMENT_STATE_TOW_DECODED is set
     *
     * For SBAS, this is:
     *   Received SBAS time, at the measurement time in nanoseconds.
     *
     *   Given the highest sync state that can be achieved, per each satellite,
     *   valid range for this field can be:
     *     Searching    : [ 0     ] : GNSS_MEASUREMENT_STATE_UNKNOWN
     *     C/A code lock: [ 0 1ms ] : GNSS_MEASUREMENT_STATE_CODE_LOCK is set
     *     Symbol sync  : [ 0 2ms ] : GNSS_MEASUREMENT_STATE_SYMBOL_SYNC is set
     *     Message      : [ 0  1s ] : GNSS_MEASUREMENT_STATE_SBAS_SYNC is set
    */
    int64_t received_sv_time_in_ns;

    /**
     * 1-Sigma uncertainty of the Received GPS Time-of-Week in nanoseconds.
     *
     * This value must be populated if 'state' != GPS_MEASUREMENT_STATE_UNKNOWN.
     */
    int64_t received_sv_time_uncertainty_in_ns;

    /**
     * Carrier-to-noise density in dB-Hz, typically in the range [0, 63].
     * It contains the measured C/N0 value for the signal at the antenna port.
     *
     * This is a mandatory value.
     */
    double c_n0_dbhz;

    /**
     * Pseudorange rate at the timestamp in m/s. The correction of a given
     * Pseudorange Rate value includes corrections for receiver and satellite
     * clock frequency errors. Ensure that this field is independent (see
     * comment at top of GnssMeasurement struct.)
     *
     * It is mandatory to provide the 'uncorrected' 'pseudorange rate', and provide GpsClock's
     * 'drift' field as well (When providing the uncorrected pseudorange rate, do not apply the
     * corrections described above.)
     *
     * The value includes the 'pseudorange rate uncertainty' in it.
     * A positive 'uncorrected' value indicates that the SV is moving away from the receiver.
     *
     * The sign of the 'uncorrected' 'pseudorange rate' and its relation to the sign of 'doppler
     * shift' is given by the equation:
     *      pseudorange rate = -k * doppler shift   (where k is a constant)
     *
     * This should be the most accurate pseudorange rate available, based on
     * fresh signal measurements from this channel.
     *
     * It is mandatory that this value be provided at typical carrier phase PRR
     * quality (few cm/sec per second of uncertainty, or better) - when signals
     * are sufficiently strong & stable, e.g. signals from a GPS simulator at >=
     * 35 dB-Hz.
     */
    double pseudorange_rate_mps;

    /**
     * 1-Sigma uncertainty of the pseudorange_rate_mps.
     * The uncertainty is represented as an absolute (single sided) value.
     *
     * This is a mandatory value.
     */
    double pseudorange_rate_uncertainty_mps;

    /**
     * Accumulated delta range's state. It indicates whether ADR is reset or there is a cycle slip
     * (indicating loss of lock).
     *
     * This is a mandatory value.
     */
    MTK_HAL_GnssAccumulatedDeltaRangeState accumulated_delta_range_state;

    /**
     * Accumulated delta range since the last channel reset in meters.
     * A positive value indicates that the SV is moving away from the receiver.
     *
     * The sign of the 'accumulated delta range' and its relation to the sign of 'carrier phase'
     * is given by the equation:
     *          accumulated delta range = -k * carrier phase    (where k is a constant)
     *
     * This value must be populated if 'accumulated delta range state' != GPS_ADR_STATE_UNKNOWN.
     * However, it is expected that the data is only accurate when:
     *      'accumulated delta range state' == GPS_ADR_STATE_VALID.
     */
    double accumulated_delta_range_m;

    /**
     * 1-Sigma uncertainty of the accumulated delta range in meters.
     * This value must be populated if 'accumulated delta range state' != GPS_ADR_STATE_UNKNOWN.
     */
    double accumulated_delta_range_uncertainty_m;

    /**
     * Carrier frequency at which codes and messages are modulated, it can be L1 or L2.
     * If the field is not set, the carrier frequency is assumed to be L1.
     *
     * If the data is available, 'flags' must contain
     * GNSS_MEASUREMENT_HAS_CARRIER_FREQUENCY.
     */
    float carrier_frequency_hz;

    /**
     * The number of full carrier cycles between the satellite and the receiver.
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     * Indications of possible cycle slips and resets in the accumulation of
     * this value can be inferred from the accumulated_delta_range_state flags.
     *
     * If the data is available, 'flags' must contain
     * GNSS_MEASUREMENT_HAS_CARRIER_CYCLES.
     */
    int64_t carrier_cycles;

    /**
     * The RF phase detected by the receiver, in the range [0.0, 1.0].
     * This is usually the fractional part of the complete carrier phase measurement.
     *
     * The reference frequency is given by the field 'carrier_frequency_hz'.
     * The value contains the 'carrier-phase uncertainty' in it.
     *
     * If the data is available, 'flags' must contain
     * GNSS_MEASUREMENT_HAS_CARRIER_PHASE.
     */
    double carrier_phase;

    /**
     * 1-Sigma uncertainty of the carrier-phase.
     * If the data is available, 'flags' must contain
     * GNSS_MEASUREMENT_HAS_CARRIER_PHASE_UNCERTAINTY.
     */
    double carrier_phase_uncertainty;

    /**
     * An enumeration that indicates the 'multipath' state of the event.
     *
     * The multipath Indicator is intended to report the presence of overlapping
     * signals that manifest as distorted correlation peaks.
     *
     * - if there is a distorted correlation peak shape, report that multipath
     *   is GNSS_MULTIPATH_INDICATOR_PRESENT.
     * - if there is not a distorted correlation peak shape, report
     *   GNSS_MULTIPATH_INDICATOR_NOT_PRESENT
     * - if signals are too weak to discern this information, report
     *   GNSS_MULTIPATH_INDICATOR_UNKNOWN
     *
     * Example: when doing the standardized overlapping Multipath Performance
     * test (3GPP TS 34.171) the Multipath indicator should report
     * GNSS_MULTIPATH_INDICATOR_PRESENT for those signals that are tracked, and
     * contain multipath, and GNSS_MULTIPATH_INDICATOR_NOT_PRESENT for those
     * signals that are tracked and do not contain multipath.
     */
    MTK_HAL_GnssMultipathIndicator multipath_indicator;

    /**
     * Signal-to-noise ratio at correlator output in dB.
     * If the data is available, 'flags' must contain GNSS_MEASUREMENT_HAS_SNR.
     * This is the power ratio of the "correlation peak height above the
     * observed noise floor" to "the noise RMS".
     */
    double snr_db;

    /**
       * Automatic gain control (AGC) level. AGC acts as a variable gain
       * amplifier adjusting the power of the incoming signal. The AGC level
       * may be used to indicate potential interference. When AGC is at a
       * nominal level, this value must be set as 0. Higher gain (and/or lower
       * input power) must be output as a positive number. Hence in cases of
       * strong jamming, in the band of this signal, this value must go more
       * negative.
       */
    double agc_level_db;

    /// v2.0
    char codeType[8];
} gnss_measurement;

typedef struct {
    /** Number of measurements. */
    size_t measurement_count;

    /** The array of measurements. */
    gps_measurement measurements[GPS_MAX_MEASUREMENT];

    /** The GPS clock time reading. */
    gps_clock clock;
} gps_data;

/**
 * Represents a reading of GNSS measurements. For devices where GnssSystemInfo's
 * year_of_hw is set to 2016+, it is mandatory that these be provided, on
 * request, when the GNSS receiver is searching/tracking signals.
 *
 * - Reporting of GPS constellation measurements is mandatory.
 * - Reporting of all tracked constellations are encouraged.
 */
typedef struct {
    /** Number of measurements. */
    size_t measurement_count;

    /** The array of measurements. */
    gnss_measurement measurements[MTK_HAL_GNSS_MAX_MEASUREMENT];

    /** The GPS clock time reading. */
    gnss_clock clock;
} gnss_data;

typedef struct {
    /**
     * Pseudo-random number in the range of [1, 32]
     * This is a Mandatory value.
     */
    int8_t prn;

    /**
     * The type of message contained in the structure.
     * This is a Mandatory value.
     */
    gps_nav_msg_type type;

    /**
     * The status of the received navigation message.
     * No need to send any navigation message that contains words with parity error and cannot be
     * corrected.
     */
    nav_msg_status status;

    /**
     * Message identifier.
     * It provides an index so the complete Navigation Message can be assembled. i.e. fo L1 C/A
     * subframe 4 and 5, this value corresponds to the 'frame id' of the navigation message.
     * Subframe 1, 2, 3 does not contain a 'frame id' and this value can be set to -1.
     */
    short message_id;

    /**
     * Sub-message identifier.
     * If required by the message 'type', this value contains a sub-index within the current
     * message (or frame) that is being transmitted.
     * i.e. for L1 C/A the submessage id corresponds to the sub-frame id of the navigation message.
     */
    short submessage_id;

    /**
     * The length of the data (in bytes) contained in the current message.
     * If this value is different from zero, 'data' must point to an array of the same size.
     * e.g. for L1 C/A the size of the sub-frame will be 40 bytes (10 words, 30 bits/word).
     * This is a Mandatory value.
     */
    size_t data_length;

    /**
     * The data of the reported GPS message.
     * The bytes (or words) specified using big endian format (MSB first).
     *
     * For L1 C/A, each subframe contains 10 30-bit GPS words. Each GPS word (30 bits) should be
     * fitted into the last 30 bits in a 4-byte word (skip B31 and B32), with MSB first.
     */
    char data[40];
} gps_nav_msg;

/** Represents a GPS navigation message (or a fragment of it). */
typedef struct {
    /** set to sizeof(GnssNavigationMessage) */
    size_t size;

    /**
     * Satellite vehicle ID number, as defined in GnssSvInfo::svid
     * This is a mandatory value.
     */
    int16_t svid;

    /**
     * The type of message contained in the structure.
     * This is a mandatory value.
     */
    MTK_HAL_GnssNavigationMessageType type;

    /**
     * The status of the received navigation message.
     * No need to send any navigation message that contains words with parity error and cannot be
     * corrected.
     */
    MTK_HAL_NavigationMessageStatus status;

    /**
     * Message identifier. It provides an index so the complete Navigation
     * Message can be assembled.
     *
     * - For GPS L1 C/A subframe 4 and 5, this value corresponds to the 'frame
     *   id' of the navigation message, in the range of 1-25 (Subframe 1, 2, 3
     *   does not contain a 'frame id' and this value can be set to -1.)
     *
     * - For Glonass L1 C/A, this refers to the frame ID, in the range of 1-5.
     *
     * - For BeiDou D1, this refers to the frame number in the range of 1-24
     *
     * - For Beidou D2, this refers to the frame number, in the range of 1-120
     *
     * - For Galileo F/NAV nominal frame structure, this refers to the subframe
     *   number, in the range of 1-12
     *
     * - For Galileo I/NAV nominal frame structure, this refers to the subframe
     *   number in the range of 1-24
     */
    int16_t message_id;

    /**
     * Sub-message identifier. If required by the message 'type', this value
     * contains a sub-index within the current message (or frame) that is being
     * transmitted.
     *
     * - For GPS L1 C/A, BeiDou D1 & BeiDou D2, the submessage id corresponds to
     *   the subframe number of the navigation message, in the range of 1-5.
     *
     * - For Glonass L1 C/A, this refers to the String number, in the range from
     *   1-15
     *
     * - For Galileo F/NAV, this refers to the page type in the range 1-6
     *
     * - For Galileo I/NAV, this refers to the word type in the range 1-10+
     */
    int16_t submessage_id;

    /**
     * The length of the data (in bytes) contained in the current message.
     * If this value is different from zero, 'data' must point to an array of the same size.
     * e.g. for L1 C/A the size of the sub-frame will be 40 bytes (10 words, 30 bits/word).
     *
     * This is a mandatory value.
     */
    size_t data_length;

    /**
     * The data of the reported GPS message. The bytes (or words) specified
     * using big endian format (MSB first).
     *
     * - For GPS L1 C/A, Beidou D1 & Beidou D2, each subframe contains 10 30-bit
     *   words. Each word (30 bits) should be fit into the last 30 bits in a
     *   4-byte word (skip B31 and B32), with MSB first, for a total of 40
     *   bytes, covering a time period of 6, 6, and 0.6 seconds, respectively.
     *
     * - For Glonass L1 C/A, each string contains 85 data bits, including the
     *   checksum.  These bits should be fit into 11 bytes, with MSB first (skip
     *   B86-B88), covering a time period of 2 seconds.
     *
     * - For Galileo F/NAV, each word consists of 238-bit (sync & tail symbols
     *   excluded). Each word should be fit into 30-bytes, with MSB first (skip
     *   B239, B240), covering a time period of 10 seconds.
     *
     * - For Galileo I/NAV, each page contains 2 page parts, even and odd, with
     *   a total of 2x114 = 228 bits, (sync & tail excluded) that should be fit
     *   into 29 bytes, with MSB first (skip B229-B232).
     */
    uint8_t* data;
} gnss_nav_msg;

#define MTK_MNLD_STRING_MAXLEN (256)
/**
 * Represents a non-framework location information request/response notification.
 */
typedef struct {
    /**
     * Package name of the Android proxy application representing the non-framework
     * entity that requested location. Set to empty string if unknown.
     */
    char proxyAppPackageName[MTK_MNLD_STRING_MAXLEN];

    /** Protocol stack that initiated the non-framework location request. */
    uint8_t protocolStack;

    /**
     * Name of the protocol stack if protocolStack field is set to OTHER_PROTOCOL_STACK.
     * Otherwise, set to empty string.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    char otherProtocolStackName[MTK_MNLD_STRING_MAXLEN];

    /** Source initiating/receiving the location information. */
    uint8_t requestor;

    /**
     * Identity of the endpoint receiving the location information. For example, carrier
     * name, OEM name, SUPL SLP/E-SLP FQDN, chipset vendor name, etc.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    char requestorId[MTK_MNLD_STRING_MAXLEN];

    /** Indicates whether location information was provided for this request. */
    uint8_t responseType;

    /** Is the device in user initiated emergency session. */
    bool inEmergencyMode;

    /** Is cached location provided */
    bool isCachedLocation;
} nfw_notification;

#if 0
void dump_gps_location(gps_location in);
void dump_gnss_sv(gnss_sv in);
void dump_gnss_sv_info(gnss_sv_info in);
void dump_gps_data(gps_data in);
void dump_gps_measurement(gps_measurement in);
void dump_gps_clock(gps_clock in);
void dump_gps_nav_msg(gps_nav_msg in);
void dump_gnss_data(gnss_data in);
void dump_gnss_measurement(gnss_measurement in);
void dump_gnss_clock(gnss_clock in);
void dump_gnss_nav_msg(gnss_nav_msg in);
#endif


#ifdef __cplusplus
}
#endif

#endif

