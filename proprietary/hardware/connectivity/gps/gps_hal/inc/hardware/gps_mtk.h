/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_INCLUDE_HARDWARE_GPS_MTK_H
#define ANDROID_INCLUDE_HARDWARE_GPS_MTK_H

#include <hardware/gps_internal.h>

__BEGIN_DECLS

// MTK extended GpsAidingData values.
#define GPS_DELETE_HOT_STILL 0x2000
#define GPS_DELETE_EPO      0x4000

// ====================vzw debug screen API =================
/**
 * Name for the VZW debug interface.
 */
#define VZW_DEBUG_INTERFACE      "vzw-debug"

#define VZW_DEBUG_STRING_MAXLEN      200

#define HAL_NFW_USER_NUM_MAX        (10)
#define HAL_NFW_USER_NAME_LEN       (64)


/** Represents data of VzwDebugData. */
typedef struct {
    /** set to sizeof(VzwDebugData) */
    size_t size;

    char  vzw_msg_data[VZW_DEBUG_STRING_MAXLEN];
} VzwDebugData;


typedef void (* vzw_debug_callback)(VzwDebugData* vzw_message);

/** Callback structure for the Vzw debug interface. */
typedef struct {
    vzw_debug_callback vzw_debug_cb;
} VzwDebugCallbacks;


/** Extended interface for VZW DEBUG support. */
typedef struct {
    /** set to sizeof(VzwDebugInterface) */
    size_t          size;

    /** Registers the callbacks for Vzw debug message. */
    int  (*init)( VzwDebugCallbacks* callbacks );

    /** Set Vzw debug screen enable/disable **/
    void (*set_vzw_debug_screen)(bool enabled);
} VzwDebugInterface;

////////////////////// GNSS HIDL v1.0 ////////////////////////////

enum { // ElapsedRealtimeFlags : uint16_t {
    /** A valid timestampNs is stored in the data structure. */
    HAS_TIMESTAMP_NS        = 1 << 0,
    /** A valid timeUncertaintyNs is stored in the data structure. */
    HAS_TIME_UNCERTAINTY_NS = 1 << 1,
};
typedef uint16_t ElapsedRealtimeFlags;

typedef struct {
    /**
     * A set of flags indicating the validity of each field in this data structure.
     *
     * Fields may have invalid information in them, if not marked as valid by the
     * corresponding bit in flags.
     */
    uint16_t flags;

    /**
     * Estimate of the elapsed time since boot value for the corresponding event in nanoseconds.
     */
    uint64_t timestampNs;

    /**
     * Estimate of the relative precision of the alignment of this SystemClock
     * timestamp, with the reported measurements in nanoseconds (68% confidence).
     */
    uint64_t timeUncertaintyNs;
}ElapsedRealtime ;


/** Represents a location. */
typedef struct {
    GpsLocation legacyLocation;
    /**
    * Represents expected horizontal position accuracy, radial, in meters
    * (68% confidence).
    */
    float           horizontalAccuracyMeters;

    /**
    * Represents expected vertical position accuracy in meters
    * (68% confidence).
    */
    float           verticalAccuracyMeters;

    /**
    * Represents expected speed accuracy in meter per seconds
    * (68% confidence).
    */
    float           speedAccuracyMetersPerSecond;

    /**
    * Represents expected bearing accuracy in degrees
    * (68% confidence).
    */
    float           bearingAccuracyDegrees;

    /// v2.0
    ElapsedRealtime     elapsedRealtime;

} GpsLocation_ext;


typedef struct {
    GnssSvInfo legacySvInfo;

    /// v1.0 ///
    float carrier_frequency;

} GnssSvInfo_ext;

/**
 * Represents SV status.
 */
typedef struct {
    /** set to sizeof(GnssSvStatus) */
    size_t size;

    /** Number of GPS SVs currently visible, refers to the SVs stored in sv_list */
    int num_svs;
    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GPS, which is reported using sv_list
     */
    GnssSvInfo_ext gnss_sv_list[GNSS_MAX_SVS];

} GnssSvStatus_ext;

/**
 * Callback with location information. Can only be called from a thread created
 * by create_thread_cb.
 */
typedef void (* gps_location_ext_callback)(GpsLocation_ext* location);

/**
 * Callback with SV status information.
 * Can only be called from a thread created by create_thread_cb.
 */
typedef void (* gnss_sv_status_ext_callback)(GnssSvStatus_ext* sv_info);

/**
 * The callback associated with the geofence.
 * Parameters:
 *      geofence_id - The id associated with the add_geofence_area.
 *      location    - The current GPS location.
 *      transition  - Can be one of GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED,
 *                    GPS_GEOFENCE_UNCERTAIN.
 *      timestamp   - Timestamp when the transition was detected.
 *
 * The callback should only be called when the caller is interested in that
 * particular transition. For instance, if the caller is interested only in
 * ENTERED transition, then the callback should NOT be called with the EXITED
 * transition.
 *
 * IMPORTANT: If a transition is triggered resulting in this callback, the GPS
 * subsystem will wake up the application processor, if its in suspend state.
 */
typedef void (*gps_geofence_transition_ext_callback) (int32_t geofence_id,
        GpsLocation_ext* location, int32_t transition, GpsUtcTime timestamp);

/**
 * The callback associated with the availability of the GPS system for geofencing
 * monitoring. If the GPS system determines that it cannot monitor geofences
 * because of lack of reliability or unavailability of the GPS signals, it will
 * call this callback with GPS_GEOFENCE_UNAVAILABLE parameter.
 *
 * Parameters:
 *  status - GPS_GEOFENCE_UNAVAILABLE or GPS_GEOFENCE_AVAILABLE.
 *  last_location - Last known location.
 */
typedef void (*gps_geofence_status_ext_callback) (int32_t status,
        GpsLocation_ext* last_location);

typedef struct {
    gps_geofence_transition_ext_callback geofence_transition_callback;
    gps_geofence_status_ext_callback geofence_status_callback;
    gps_geofence_add_callback geofence_add_callback;
    gps_geofence_remove_callback geofence_remove_callback;
    gps_geofence_pause_callback geofence_pause_callback;
    gps_geofence_resume_callback geofence_resume_callback;
    gps_create_thread create_thread_cb;
} GpsGeofenceCallbacks_ext;

/** Extended interface for GPS_Geofencing support */
typedef struct {
   /** set to sizeof(GpsGeofencingInterface) */
   size_t          size;

   /**
    * Opens the geofence interface and provides the callback routines
    * to the implementation of this interface.
    */
   void  (*init)( GpsGeofenceCallbacks_ext* callbacks );

   /**
    * Add a geofence area. This api currently supports circular geofences.
    * Parameters:
    *    geofence_id - The id for the geofence. If a geofence with this id
    *       already exists, an error value (GPS_GEOFENCE_ERROR_ID_EXISTS)
    *       should be returned.
    *    latitude, longtitude, radius_meters - The lat, long and radius
    *       (in meters) for the geofence
    *    last_transition - The current state of the geofence. For example, if
    *       the system already knows that the user is inside the geofence,
    *       this will be set to GPS_GEOFENCE_ENTERED. In most cases, it
    *       will be GPS_GEOFENCE_UNCERTAIN.
    *    monitor_transition - Which transitions to monitor. Bitwise OR of
    *       GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED and
    *       GPS_GEOFENCE_UNCERTAIN.
    *    notification_responsiveness_ms - Defines the best-effort description
    *       of how soon should the callback be called when the transition
    *       associated with the Geofence is triggered. For instance, if set
    *       to 1000 millseconds with GPS_GEOFENCE_ENTERED, the callback
    *       should be called 1000 milliseconds within entering the geofence.
    *       This parameter is defined in milliseconds.
    *       NOTE: This is not to be confused with the rate that the GPS is
    *       polled at. It is acceptable to dynamically vary the rate of
    *       sampling the GPS for power-saving reasons; thus the rate of
    *       sampling may be faster or slower than this.
    *    unknown_timer_ms - The time limit after which the UNCERTAIN transition
    *       should be triggered. This parameter is defined in milliseconds.
    *       See above for a detailed explanation.
    */
   void (*add_geofence_area) (int32_t geofence_id, double latitude, double longitude,
       double radius_meters, int last_transition, int monitor_transitions,
       int notification_responsiveness_ms, int unknown_timer_ms);

   /**
    * Pause monitoring a particular geofence.
    * Parameters:
    *   geofence_id - The id for the geofence.
    */
   void (*pause_geofence) (int32_t geofence_id);

   /**
    * Resume monitoring a particular geofence.
    * Parameters:
    *   geofence_id - The id for the geofence.
    *   monitor_transitions - Which transitions to monitor. Bitwise OR of
    *       GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED and
    *       GPS_GEOFENCE_UNCERTAIN.
    *       This supersedes the value associated provided in the
    *       add_geofence_area call.
    */
   void (*resume_geofence) (int32_t geofence_id, int monitor_transitions);

   /**
    * Remove a geofence area. After the function returns, no notifications
    * should be sent.
    * Parameter:
    *   geofence_id - The id for the geofence.
    */
   void (*remove_geofence_area) (int32_t geofence_id);
} GpsGeofencingInterface_ext;

enum {
    STATE_UNKNOWN                = 0,
    STATE_CODE_LOCK              = 1 << 0,
    STATE_BIT_SYNC               = 1 << 1,
    STATE_SUBFRAME_SYNC          = 1 << 2,
    STATE_TOW_DECODED            = 1 << 3,
    STATE_MSEC_AMBIGUOUS         = 1 << 4,
    STATE_SYMBOL_SYNC            = 1 << 5,
    STATE_GLO_STRING_SYNC        = 1 << 6,
    STATE_GLO_TOD_DECODED        = 1 << 7,
    STATE_BDS_D2_BIT_SYNC        = 1 << 8,
    STATE_BDS_D2_SUBFRAME_SYNC   = 1 << 9,
    STATE_GAL_E1BC_CODE_LOCK     = 1 << 10,
    STATE_GAL_E1C_2ND_CODE_LOCK  = 1 << 11,
    STATE_GAL_E1B_PAGE_SYNC      = 1 << 12,
    STATE_SBAS_SYNC              = 1 << 13,
    STATE_TOW_KNOWN              = 1 << 14,
    STATE_GLO_TOD_KNOWN          = 1 << 15,
    STATE_2ND_CODE_LOCK          = 1 << 16,
};
//typedef uint32_t GnssMeasurementState;

/// extension to GnssConstellationType;
enum {
    /** Indian Regional Navigation Satellite System. */
    GNSS_CONSTELLATION_IRNSS   = 7,
    GNSS_CONSTELLATION_SIZE    = 8,
};
//typedef uint8_t GnssConstellationType;

typedef struct {
    GnssMeasurement legacyMeasurement;

    /**
     * Automatic gain control (AGC) level. AGC acts as a variable gain
     * amplifier adjusting the power of the incoming signal. The AGC level
     * may be used to indicate potential interference. When AGC is at a
     * nominal level, this value must be set as 0. Higher gain (and/or lower
     * input power) must be output as a positive number. Hence in cases of
     * strong jamming, in the band of this signal, this value must go more
     * negative.
     *
     * Note: Different hardware designs (e.g. antenna, pre-amplification, or
     * other RF HW components) may also affect the typical output of of this
     * value on any given hardware design in an open sky test - the
     * important aspect of this output is that changes in this value are
     * indicative of changes on input signal power in the frequency band for
     * this measurement.
     */
    /// v1.1
    double agc_level_db;

    /// v2.0
    char codeType[8];
    // uint32_t state; direct use original field
    //GnssConstellationType constellation; direct use original field

} GnssMeasurement_ext;

/**
 * Represents a reading of GNSS measurements. For devices where GnssSystemInfo's
 * year_of_hw is set to 2016+, it is mandatory that these be provided, on
 * request, when the GNSS receiver is searching/tracking signals.
 *
 * - Reporting of GPS constellation measurements is mandatory.
 * - Reporting of all tracked constellations are encouraged.
 */
typedef struct {
    /** set to sizeof(GnssData) */
    size_t size;

    /** Number of measurements. */
    size_t measurement_count;

    /** The array of measurements. */
    GnssMeasurement_ext measurements[GNSS_MAX_MEASUREMENT];

    /** The GPS clock time reading. */
    GnssClock clock;

    /// v2.0
    ElapsedRealtime elapsedRealtime;
} GnssData_ext;

/**
 * The callback for to report measurements from the HAL.
 *
 * Parameters:
 *    data - A data structure containing the measurements.
 */
typedef void (*gnss_measurement_ext_callback) (GnssData_ext* data);

typedef struct {
    /** set to sizeof(GpsMeasurementCallbacks) */
    size_t size;
    gps_measurement_callback measurement_callback;
    gnss_measurement_ext_callback gnss_measurement_callback;
} GpsMeasurementCallbacks_ext;


/////// Gnss debug ////

/** Milliseconds since January 1, 1970 */
typedef int64_t GnssUtcTime;

typedef enum {
    /** Ephemeris is known for this satellite. */
    EPHEMERIS,
    /**
     * Ephemeris is not known, but Almanac (approximate location) is known.
     */
    ALMANAC_ONLY,
    /**
     * Both ephemeris & almanac are not known (e.g. during a cold start
     * blind search.)
     */
    NOT_AVAILABLE
} SatelliteEphemerisType;

typedef enum {
    /**
     * The ephemeris (or almanac only) information was demodulated from the
     * signal received on the device
     */
    DEMODULATED,
    /**
     * The ephemeris (or almanac only) information was received from a SUPL
     * server.
     */
    SUPL_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * server.
     */
    OTHER_SERVER_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * method, e.g. injected via a local debug tool, from build defaults
     * (e.g. almanac), or is from a satellite
     * with SatelliteEphemerisType::NOT_AVAILABLE.
     */
    OTHER
} SatelliteEphemerisSource;

typedef enum {
    /** The ephemeris is known good. */
    GOOD,
    /** The ephemeris is known bad. */
    BAD,
    /** The ephemeris is unknown to be good or bad. */
    UNKNOWN
} SatelliteEphemerisHealth;

/**
 * Provides the current best known position from any
 * source (GNSS or injected assistance).
 */
typedef struct {
    /**
     * Validity of the data in this struct. False only if no
     * latitude/longitude information is known.
     */
    bool valid;
    /** Latitude expressed in degrees */
    double latitudeDegrees;
    /** Longitude expressed in degrees */
    double longitudeDegrees;
    /** Altitude above ellipsoid expressed in meters */
    float altitudeMeters;
    /** Represents horizontal speed in meters per second. */
    float speedMetersPerSec;
    /** Represents heading in degrees. */
    float bearingDegrees;
    /**
     * Estimated horizontal accuracy of position expressed in meters,
     * radial, 68% confidence.
     */
    double horizontalAccuracyMeters;
    /**
     * Estimated vertical accuracy of position expressed in meters, with
     * 68% confidence.
     */
    double verticalAccuracyMeters;
    /**
     * Estimated speed accuracy in meters per second with 68% confidence.
     */
    double speedAccuracyMetersPerSecond;
    /**
     * estimated bearing accuracy degrees with 68% confidence.
     */
    double bearingAccuracyDegrees;
    /**
     * Time duration before this report that this position information was
     * valid.  This can, for example, be a previous injected location with
     * an age potentially thousands of seconds old, or
     * extrapolated to the current time (with appropriately increased
     * accuracy estimates), with a (near) zero age.
     */
    float ageSeconds;
} PositionDebug;

/**
 * Provides the current best known UTC time estimate.
 * If no fresh information is available, e.g. after a delete all,
 * then whatever the effective defaults are on the device must be
 * provided (e.g. Jan. 1, 2017, with an uncertainty of 5 years) expressed
 * in the specified units.
 */
typedef struct {
    /** UTC time estimate. */
    GnssUtcTime timeEstimate;
    /** 68% error estimate in time. */
    float timeUncertaintyNs;
    /**
     * 68% error estimate in local clock drift,
     * in nanoseconds per second (also known as parts per billion - ppb.)
     */
    float frequencyUncertaintyNsPerSec;
} TimeDebug;

/**
 * Provides a single satellite info that has decoded navigation data.
 */
typedef struct {
    /** Satellite vehicle ID number */
    int16_t svid;
    /** Defines the constellation type of the given SV. */
    GnssConstellationType constellation;

    /**
     * Defines the standard broadcast ephemeris or almanac availability for
     * the satellite.  To report status of predicted orbit and clock
     * information, see the serverPrediction fields below.
     */
    SatelliteEphemerisType ephemerisType;
    /** Defines the ephemeris source of the satellite. */
    SatelliteEphemerisSource ephemerisSource;
    /**
     * Defines whether the satellite is known healthy
     * (safe for use in location calculation.)
     */
    SatelliteEphemerisHealth ephemerisHealth;
    /**
     * Time duration from this report (current time), minus the
     * effective time of the ephemeris source (e.g. TOE, TOA.)
     * Set to 0 when ephemerisType is NOT_AVAILABLE.
     */
    float ephemerisAgeSeconds;

    /**
     * True if a server has provided a predicted orbit and clock model for
     * this satellite.
     */
    bool serverPredictionIsAvailable;
    /**
     * Time duration from this report (current time) minus the time of the
     * start of the server predicted information.  For example, a 1 day
     * old prediction would be reported as 86400 seconds here.
     */
    float serverPredictionAgeSeconds;
} SatelliteData;

/**
 * Provides a set of debug information that is filled by the GNSS chipset
 * when the method getDebugData() is invoked.
 */
typedef struct {
    /** Current best known position. */
    PositionDebug position;
    /** Current best know time estimate */
    TimeDebug time;
    /**
     * Provides a list of the available satellite data, for all
     * satellites and constellations the device can track,
     * including GnssConstellationType UNKNOWN.
     */
    SatelliteData satelliteDataArray[GNSS_MAX_SVS];
} DebugData;


/** Extended interface for DEBUG support. */
typedef struct {
    /** set to sizeof(GpsDebugInterface) */
    size_t          size;

    /**
     * This function should return any information that the native
     * implementation wishes to include in a bugreport.
     */
    // size_t (*get_internal_state)(char* buffer, size_t bufferSize);
    /// v1.0 ///
    bool (*get_internal_state)(DebugData* debugData);
} GpsDebugInterface_ext;


////////////////////// GNSS HIDL v1.1 ////////////////////////////

/**
 * Callback for reporting driver name information.
 */
typedef void (* gnss_set_name_callback)(const char* name, int length);

/**
 * Callback for requesting framework NLP or Fused location injection.
 */
typedef void (* gnss_request_location_callback)(bool independentFromGnss, bool isUserEmergency);

/** New GPS callback structure. */
typedef struct {
    /** set to sizeof(GpsCallbacks) */
    size_t      size;
    gps_location_ext_callback location_cb;
    gps_status_callback status_cb;
    gps_sv_status_callback sv_status_cb;
    gps_nmea_callback nmea_cb;
    gps_set_capabilities set_capabilities_cb;
    gps_acquire_wakelock acquire_wakelock_cb;
    gps_release_wakelock release_wakelock_cb;
    gps_create_thread create_thread_cb;
    gps_request_utc_time request_utc_time_cb;

    gnss_set_system_info set_system_info_cb;
    gnss_sv_status_ext_callback gnss_sv_status_cb;

    /////v1.1////
    gnss_set_name_callback set_name_cb;
    ///// v2.0 ///
    gnss_request_location_callback request_location_cb;
} GpsCallbacks_ext;


/** Represents the standard GPS interface. */
typedef struct {
    /** set to sizeof(GpsInterface) */
    size_t          size;
    /**
     * Opens the interface and provides the callback routines
     * to the implementation of this interface.
     */
    /// v1.0 ///
//    int   (*init)( GpsCallbacks* callbacks );
    /// v1.1 ///
    int   (*init)( GpsCallbacks_ext* callbacks );

    /** Starts navigating. */
    int   (*start)( void );

    /** Stops navigating. */
    int   (*stop)( void );

    /** Closes the interface. */
    void  (*cleanup)( void );

    /** Injects the current time. */
    int   (*inject_time)(GpsUtcTime time, int64_t timeReference,
                         int uncertainty);

    /**
     * Injects current location from another location provider (typically cell
     * ID). Latitude and longitude are measured in degrees expected accuracy is
     * measured in meters
     */
    int  (*inject_location)(double latitude, double longitude, float accuracy);

    /**
     * Specifies that the next call to start will not use the
     * information defined in the flags. GPS_DELETE_ALL is passed for
     * a cold start.
     */
    void  (*delete_aiding_data)(GpsAidingData flags);

    /**
     * min_interval represents the time between fixes in milliseconds.
     * preferred_accuracy represents the requested fix accuracy in meters.
     * preferred_time represents the requested time to first fix in milliseconds.
     *
     * 'mode' parameter should be one of GPS_POSITION_MODE_MS_BASED
     * or GPS_POSITION_MODE_STANDALONE.
     * It is allowed by the platform (and it is recommended) to fallback to
     * GPS_POSITION_MODE_MS_BASED if GPS_POSITION_MODE_MS_ASSISTED is passed in, and
     * GPS_POSITION_MODE_MS_BASED is supported.
     */
     /// v1.0 ///
//    int   (*set_position_mode)(GpsPositionMode mode, GpsPositionRecurrence recurrence,
//            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);
    /// v1.1 ///
    int   (*set_position_mode)(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time,
            bool lowPowerMode);


    /** Get a pointer to extension information. */
    const void* (*get_extension)(const char* name);

    /// v1.1 ///
    int  (*inject_fused_location)(double latitude, double longitude, float accuracy);

} GpsInterface_ext;


/**
 * Extended interface for GPS Measurements support.
 */
typedef struct {
    /** Set to sizeof(GpsMeasurementInterface_ext) */
    size_t size;

    /**
     * Initializes the interface and registers the callback routines with the HAL.
     * After a successful call to 'init' the HAL must begin to provide updates at its own phase.
     *
     * Status:
     *    GPS_MEASUREMENT_OPERATION_SUCCESS
     *    GPS_MEASUREMENT_ERROR_ALREADY_INIT - if a callback has already been registered without a
     *              corresponding call to 'close'
     *    GPS_MEASUREMENT_ERROR_GENERIC - if any other error occurred, it is expected that the HAL
     *              will not generate any updates upon returning this error code.
     */
    /// v1.0 ///
//    int (*init) (GpsMeasurementCallbacks* callbacks);
    /// v1.1 ///
    int (*init) (GpsMeasurementCallbacks_ext* callbacks, bool enableFullTracking);

    /**
     * Stops updates from the HAL, and unregisters the callback routines.
     * After a call to stop, the previously registered callbacks must be considered invalid by the
     * HAL.
     * If stop is invoked without a previous 'init', this function should perform no work.
     */
    void (*close) ();

} GpsMeasurementInterface_ext;


/**
 * Interface for passing GNSS configuration contents from platform to HAL.
 */
typedef struct {
    /** Set to sizeof(GnssConfigurationInterface) */
    size_t size;

    /**
     * Deliver GNSS configuration contents to HAL.
     * Parameters:
     *     config_data - a pointer to a char array which holds what usually is expected from
                         file(/etc/gps.conf), i.e., a sequence of UTF8 strings separated by '\n'.
     *     length - total number of UTF8 characters in configuraiton data.
     *
     * IMPORTANT:
     *      GPS HAL should expect this function can be called multiple times. And it may be
     *      called even when GpsLocationProvider is already constructed and enabled. GPS HAL
     *      should maintain the existing requests for various callback regardless the change
     *      in configuration data.
     */
    void (*configuration_update) (const char* config_data, int32_t length);

   //// v1.1 ////
   void (*set_black_list) (long long* blacklist, int32_t size);

   //// v2.0 ////
   void (*set_es_extension_sec) (uint32_t emergencyExtensionSeconds);
} GnssConfigurationInterface_ext;

////////////////////// GNSS HIDL v2.0 ////////////////////////////

#define STRING_MAXLEN      256

/**
 * Name for the GNSS measurement correction interface.
 */
#define GNSS_MEASUREMENT_CORRECTION_INTERFACE      "gnss_measurement_correction"

/**
 * Name for the GNSS visibility control interface.
 */
#define GNSS_VISIBILITY_CONTROL_INTERFACE      "gnss_visiblity_control"

//// GNSS capabilities extension
enum {
    /** GNSS supports low power mode */
    GPS_CAPABILITY_LOW_POWER_MODE                  = 1 << 8,
    /** GNSS supports blacklisting satellites */
    GPS_CAPABILITY_SATELLITE_BLACKLIST             = 1 << 9,
    /** GNSS supports measurement corrections */
    MEASUREMENT_CORRECTIONS                        = 1 << 10
};
typedef uint32_t GnssCapabilities;

enum {
    // SUPL         = 1, // already defined
    // C2K          = 2, // already defined
    AGPS_TYPE_SUPL_EIMS    = 3,
    AGPS_TYPE_SUPL_IMS     = 4,
};
typedef uint8_t AGnssType;

enum {
    /** GNSS requests data connection for AGNSS. */
    REQUEST_AGNSS_DATA_CONN  = 1,
    /** GNSS releases the AGNSS data connection. */
    RELEASE_AGNSS_DATA_CONN  = 2,
    /** AGNSS data connection initiated */
    AGNSS_DATA_CONNECTED     = 3,
    /** AGNSS data connection completed */
    AGNSS_DATA_CONN_DONE     = 4,
    /** AGNSS data connection failed */
    AGNSS_DATA_CONN_FAILED   = 5
};
typedef uint8_t AGnssStatusValue;

/**
 * Extended interface for AGPS support, it is augmented to enable to pass
 * extra APN data.
 */
typedef struct {
    /** set to sizeof(AGpsInterface) */
    size_t size;

    /**
     * Opens the AGPS interface and provides the callback routines to the
     * implementation of this interface.
     */
    void (*init)(AGpsCallbacks* callbacks);
    /**
     * Deprecated.
     * If the HAL supports AGpsInterface_v2 this API will not be used, see
     * data_conn_open_with_apn_ip_type for more information.
     */
    int (*data_conn_open)(const char* apn);
    /**
     * Notifies that the AGPS data connection has been closed.
     */
    int (*data_conn_closed)();
    /**
     * Notifies that a data connection is not available for AGPS.
     */
    int (*data_conn_failed)();
    /**
     * Sets the hostname and port for the AGPS server.
     */
    int (*set_server)(AGpsType type, const char* hostname, int port);

    /**
     * Notifies that a data connection is available and sets the name of the
     * APN, and its IP type, to be used for SUPL connections.
     */
    int (*data_conn_open_with_apn_ip_type)(
            uint64_t networkHandle,
            const char* apn,
            ApnIpType apnIpType);
} AGpsInterface_ext;

/** Flags to indicate capabilities of the network */
enum { //NetworkCapability : uint16_t {
    /** Network is not metered. */
    NOT_METERED       = 1 << 0,
    /** Network is not roaming. */
    NOT_ROAMING       = 1 << 1
};
typedef uint16_t NetworkCapability;


/** Extended interface for AGPS_RIL support. */
typedef struct {
    /** set to sizeof(AGpsRilInterface_ext) */
    size_t          size;
    /**
     * Opens the AGPS interface and provides the callback routines
     * to the implementation of this interface.
     */
    void  (*init)( AGpsRilCallbacks* callbacks );

    /**
     * Sets the reference location.
     */
    void (*set_ref_location) (const AGpsRefLocation *agps_reflocation, size_t sz_struct);
    /**
     * Sets the set ID.
     */
    void (*set_set_id) (AGpsSetIDType type, const char* setid);

    /**
     * Send network initiated message.
     */
    void (*ni_message) (uint8_t *msg, size_t len);

    /**
     * Notify GPS of network status changes.
     * These parameters match values in the android.net.NetworkInfo class.
     */
    //// v2.0 ///
    void (*update_network_state_ext) (uint64_t networkHandle, bool isConnected,
            uint16_t capabilities, const char* apn);

    /**
     * Notify GPS of network status changes.
     * These parameters match values in the android.net.NetworkInfo class.
     */
    void (*update_network_availability) (int avaiable, const char* apn);
} AGpsRilInterface_ext;


/** Bit mask to indicate which values are valid in a SingleSatCorrection object. */
enum { //GnssSingleSatCorrectionFlags : uint16_t {
    /** GnssSingleSatCorrectionFlags has valid satellite-is-line-of-sight-probability field. */
    HAS_SAT_IS_LOS_PROBABILITY               = 0x0001,
    /** GnssSingleSatCorrectionFlags has valid Excess Path Length field. */
    HAS_EXCESS_PATH_LENGTH                   = 0x0002,
    /** GnssSingleSatCorrectionFlags has valid Excess Path Length Uncertainty field. */
    HAS_EXCESS_PATH_LENGTH_UNC               = 0x0004,
    /** GnssSingleSatCorrectionFlags has valid Reflecting Plane field. */
    HAS_REFLECTING_PLANE                     = 0x0008
};
typedef uint16_t GnssSingleSatCorrectionFlags;

/**
 * A struct containing the characteristics of the reflecting plane that the satellite signal has
 * bounced from.
 *
 * The value is only valid if HAS_REFLECTING_PLANE flag is set. An invalid reflecting plane
 * means either reflection planes serving is not supported or the satellite signal has gone
 * through multiple reflections.
 */
typedef struct {
    /** Represents latitude of the reflecting plane in degrees. */
    double latitudeDegrees;

    /** Represents longitude of the reflecting plane in degrees. */
    double longitudeDegrees;

    /**
     * Represents altitude of the reflecting point in the plane in meters above the WGS 84 reference
     * ellipsoid.
     */
    double altitudeMeters;

    /** Represents azimuth clockwise from north of the reflecting plane in degrees. */
    double azimuthDegrees;
} ReflectingPlane ;


typedef struct {

    /** Contains GnssSingleSatCorrectionFlags bits. */
    uint16_t singleSatCorrectionFlags;

    /**
     * Defines the constellation of the given satellite.
     */
    GnssConstellationType constellation;

    /**
     * Satellite vehicle ID number, as defined in GnssSvInfo::svid
     */
    uint16_t svid;

    /**
     * Carrier frequency of the signal to be corrected, for example it can be the
     * GPS center frequency for L1 = 1,575,420,000 Hz, varying GLO channels, etc.
     *
     * For a receiver with capabilities to track multiple frequencies for the same satellite,
     * multiple corrections for the same satellite may be provided.
     */
    float carrierFrequencyHz;

    /**
     * The probability that the satellite is estimated to be in Line-of-Sight condition at the given
     * location.
     */
    float probSatIsLos;

    /**
     * Excess path length to be subtracted from pseudorange before using it in calculating location.
     *
     * Note this value is NOT to be used to adjust the GnssMeasurementCallback outputs.
     */
    float excessPathLengthMeters;

    /** Error estimate (1-sigma) for the Excess path length estimate */
    float excessPathLengthUncertaintyMeters;

    /**
     * Defines the reflecting plane characteristics such as location and azimuth
     *
     * The value is only valid if HAS_REFLECTING_PLANE flag is set. An invalid reflecting plane
     * means either reflection planes serving is not supported or the satellite signal has gone
     * through multiple reflections.
     */
     ReflectingPlane reflectingPlane;
}SingleSatCorrection;

/**
 * A struct containing a set of measurement corrections for all used GNSS satellites at the location
 * specified by latitudeDegrees, longitudeDegrees, altitudeMeters and at the time of week specified
 * toaGpsNanosecondsOfWeek
 */
typedef struct {
    /** Represents latitude in degrees at which the corrections are computed.. */
    double latitudeDegrees;

    /** Represents longitude in degrees at which the corrections are computed.. */
    double longitudeDegrees;

    /**
     * Represents altitude in meters above the WGS 84 reference ellipsoid at which the corrections
     * are computed.
     */
    double altitudeMeters;

    /**
     * Represents the horizontal uncertainty (68% confidence) in meters on the device position at
     * which the corrections are provided.
     *
     * This value is useful for example to judge how accurate the provided corrections are.
     */
    double horizontalPositionUncertaintyMeters;

    /**
     * Represents the vertical uncertainty (68% confidence) in meters on the device position at
     * which the corrections are provided.
     *
     * This value is useful for example to judge how accurate the provided corrections are.
     */
    double verticalPositionUncertaintyMeters;

    /** Time Of Applicability, GPS time of week in nanoseconds. */
    uint64_t toaGpsNanosecondsOfWeek;

    /** Number of singleSatCorrection */
    size_t num_satCorrection;

    /**
     * A set of SingleSatCorrection each containing measurement corrections for a satellite in view
     */
    SingleSatCorrection satCorrections[GNSS_MAX_SVS];
} MeasurementCorrections;


enum { //MeasurementCorrectionCapabilities : uint32_t {
    /** GNSS supports line-of-sight satellite identification measurement corrections */
    LOS_SATS                        = 1 << 0,
    /** GNSS supports per satellite excess-path-length measurement corrections */
    EXCESS_PATH_LENGTH              = 1 << 1,
    /** GNSS supports reflecting planes measurement corrections */
    REFLECTING_PLANE                = 1 << 2
};
typedef uint32_t MeasurementCorrectionCapabilities;


/**
 * Callback to inform framework the measurement correction specific capabilities of the GNSS
 * HAL implementation.
 *
 * The GNSS HAL must call this method immediately after the framework opens the measurement
 * corrections interface.
 *
 * @param capabilities Supported measurement corrections capabilities. It is mandatory to
 *        support either LOS_STATS or EXCESS_PATH_LENGTH capability.
 *
 */
typedef void (*meac_set_capabilities_cb) (uint32_t capabilities);

typedef struct {
    /** set to sizeof(GpsMeasurementCorrectionCallbacks) */
    size_t size;
    meac_set_capabilities_cb set_capabilities_cb;
} MeasurementCorrectionCallbacks_ext;



typedef struct {

    /** Set to sizeof(GpsMeasurementCorrectionInterface) */
    size_t size;

    bool (*meac_set_callback) (MeasurementCorrectionCallbacks_ext* callbacks);

    bool (*meac_set_corrections) (MeasurementCorrections* corrections);

}MeasurementCorrectionInterface;


/**
 * Protocol stack that is requesting the non-framework location information.
 */
enum {
    /** Cellular control plane requests */
    CTRL_PLANE                      = 0,
    /** All types of SUPL requests */
    SUPL                            = 1,

    /** All types of requests from IMS */
    IMS                             = 10,
    /** All types of requests from SIM */
    SIM                             = 11,

    /** Requests from other protocol stacks */
    OTHER_PROTOCOL_STACK            = 100
};
typedef uint8_t NfwProtocolStack;

/*
 * Entity that is requesting/receiving the location information.
 */
enum { //NfwRequestor : uint8_t {
    /** Wireless service provider */
    CARRIER                         = 0,
    /** Device manufacturer */
    OEM                             = 10,
    /** Modem chipset vendor */
    MODEM_CHIPSET_VENDOR            = 11,
    /** GNSS chipset vendor */
    GNSS_CHIPSET_VENDOR             = 12,
    /** Other chipset vendor */
    OTHER_CHIPSET_VENDOR            = 13,
    /** Automobile client */
    AUTOMOBILE_CLIENT               = 20,
    /** Other sources */
    OTHER_REQUESTOR                 = 100
 };
typedef uint8_t NfwRequestor;

/**
 * GNSS response type for non-framework location requests.
 */
enum {//NfwResponseType : uint8_t {
    /** Request rejected because framework has not given permission for this use case */
    REJECTED                        = 0,

    /** Request accepted but could not provide location because of a failure */
    ACCEPTED_NO_LOCATION_PROVIDED   = 1,

    /** Request accepted and location provided */
    ACCEPTED_LOCATION_PROVIDED      = 2,
};
typedef uint8_t NfwResponseType;

/**
 * Represents a non-framework location information request/response notification.
 */
typedef struct {
    /**
     * Package name of the Android proxy application representing the non-framework
     * entity that requested location. Set to empty string if unknown.
     */
    char proxyAppPackageName[STRING_MAXLEN];

    /** Protocol stack that initiated the non-framework location request. */
    NfwProtocolStack protocolStack;

    /**
     * Name of the protocol stack if protocolStack field is set to OTHER_PROTOCOL_STACK.
     * Otherwise, set to empty string.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    char otherProtocolStackName[STRING_MAXLEN];

    /** Source initiating/receiving the location information. */
    NfwRequestor requestor;

    /**
     * Identity of the endpoint receiving the location information. For example, carrier
     * name, OEM name, SUPL SLP/E-SLP FQDN, chipset vendor name, etc.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    char requestorId[STRING_MAXLEN];

    /** Indicates whether location information was provided for this request. */
    NfwResponseType responseType;

    /** Is the device in user initiated emergency session. */
    bool inEmergencyMode;

    /** Is cached location provided */
    bool isCachedLocation;
} NfwNotification;

/**
 * Callback to report a non-framework delivered location.
 *
 * The GNSS HAL implementation must call this method to notify the framework whenever
 * a non-framework location request is made to the GNSS HAL.
 *
 * Non-framework entities like low power sensor hubs that request location from GNSS and
 * only pass location information through Android framework controls are exempt from this
 * power-spending reporting. However, low power sensor hubs or other chipsets which may send
 * the location information to anywhere other than Android framework (which provides user
 * visibility and control), must report location information use through this API whenever
 * location information (or events driven by that location such as "home" location detection)
 * leaves the domain of that low power chipset.
 *
 * To avoid overly spamming the framework, high speed location reporting of the exact same
 * type may be throttled to report location at a lower rate than the actual report rate, as
 * long as the location is reported with a latency of no more than the larger of 5 seconds,
 * or the next the Android processor awake time. For example, if an Automotive client is
 * getting location information from the GNSS location system at 20Hz, this method may be
 * called at 1Hz. As another example, if a low power processor is getting location from the
 * GNSS chipset, and the Android processor is asleep, the notification to the Android HAL may
 * be delayed until the next wake of the Android processor.
 *
 * @param notification Non-framework delivered location request/response description.
 */
typedef void (*vc_nfw_notify_cb) (NfwNotification notification);

/**
 * Tells if the device is currently in an emergency session.
 *
 * Emergency session is defined as the device being actively in a user initiated emergency
 * call or in post emergency call extension time period.
 *
 * If the GNSS HAL implementation cannot determine if the device is in emergency session
 * mode, it must call this method to confirm that the device is in emergency session before
 * serving network initiated emergency SUPL and Control Plane location requests.
 *
 * @return success True if the framework determines that the device is in emergency session.
 */
typedef bool (*vc_is_in_emergency_session) ();

typedef struct {
    /** set to sizeof(GpsMeasurementCorrectionCallbacks) */
    size_t size;
    vc_nfw_notify_cb nfw_notify_cb;
    vc_is_in_emergency_session is_in_emergency_session;
} GnssVisibilityControlCallback_ext;



typedef struct {

    /** Set to sizeof(GnssVisibilityControlInterface) */
    size_t size;

    /**
     * Registers the callback for HAL implementation to use.
     *
     * @param callback Handle to IGnssVisibilityControlCallback interface.
     */
    bool (*vc_set_callback) (GnssVisibilityControlCallback_ext* callbacks);

    /**
     * Enables/disables non-framework entity location access permission in the GNSS HAL.
     *
     * The framework will call this method to update GNSS HAL implementation every time the
     * framework user, through the given proxy application(s) and/or device location settings,
     * explicitly grants/revokes the location access permission for non-framework, non-user
     * initiated emergency use cases.
     *
     * Whenever the user location information is delivered to non-framework entities, the HAL
     * implementation must call the method IGnssVisibilityControlCallback.nfwNotifyCb() to notify
     * the framework for user visibility.
     *
     * @param proxyApps Full list of package names of proxy Android applications representing
     * the non-framework location access entities (on/off the device) for which the framework
     * user has granted non-framework location access permission. The GNSS HAL implementation
     * must provide location information only to non-framework entities represented by these
     * proxy applications.
     *
     * The package name of the proxy Android application follows the standard Java language
     * package naming format. For example, com.example.myapp.
     *
     * @return success True if the operation was successful.
     */
    bool (*vc_enable_nfw_location_access) (const char* proxyApps, int32_t length);

}GnssVisibilityControlInterface;



struct gps_device_t_ext {
    struct hw_device_t common;

    /**
     * Set the provided lights to the provided values.
     *
     * Returns: 0 on succes, error code on failure.
     */
    const GpsInterface_ext* (*get_gps_interface)(struct gps_device_t_ext* dev);
};


__END_DECLS

#endif /* ANDROID_INCLUDE_HARDWARE_GPS_MTK_H */

