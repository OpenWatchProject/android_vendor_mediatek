/**
 * @file pal.h
 * @brief File containing PAL interface to DM client.
 *
 * Platform abstraction layer definitions. OEM should implement
 * all required functions in order to use DM client with specific
 * platform.
 */


#ifndef PAL_H_
#define PAL_H_

#include "pal_common.h"
#include "pal_connmo.h"
#include "pal_devdetail.h"
#include "pal_devinfo.h"
#include "pal_dcmo.h"

__BEGIN_DECLS

/**
  * Max buffer size for values from pal
  */
#define MAX_BUFFER_SIZE 255
/**
  * Max buffer size for values from diagmon pal
  */
#define MAX_BUFFER_SIZE_2000 2000
/**
 *  @breif last_update_date_time  MM:DD:YYYY HH:MM:SS
 */
#define PAL_FILENAME_LAST_UPD_DATETIME "last_update_date_time"
/**
 *  @breif last_update_firmware_version, for example 1.2.3-456
 *      PAL_DEFAULT_FIRMWARE_VERSION by default
 */
#define PAL_FILENAME_LAST_UPD_FIRMWARE "last_update_firmware_version"
/**
 *  @breif last_update_firmware_packet_name
 *      <Manufacturer>_<Model>_<BaseFwVersion>_<TargetFwVersion>
 */
#define PAL_FILENAME_FIRMWARE_PACKET_NAME "last_update_firmware_packet_name"
/**
 *  @brief date time format in file for storing firmware version
 */
#define PAL_DATEIME_FILE_FORMAT "%m:%d:%Y %H:%M:%S"
/**
 *  @brief default firmware version
 */
#define PAL_DEFAULT_FIRMWARE_VERSION "1.0.0"
/**
 *  @brief default firmware package name
 *  <Manufacturer>_<Model>_<BaseFwVersion>_<TargetFwVersion>
 */
#define PAL_DEFAULT_FIRMWARE_PACKET_NAME \
    "Manufacturer_Model_0.0.0_" PAL_DEFAULT_FIRMWARE_VERSION

/**
 * This method initializes PAL
 * @return execution status: 0 on success or error code
 * on failure.
 * @note Sets OmadmServiceAPI::mInitialized to true
 * @note Thist method can block forever if PAL service
 * not found.
 */
int pal_init();
#define PAL_INIT "pal_init"

/**
  * Omadm string delimiter
  */
#define OMADM_DELIMITER "<omadm>"

/**
 * Errors codes returned by PAL's function
 */
typedef enum result_states {

    RESULT_SUCCESS = 0,            /// PAL function complete successfully
    RESULT_ERROR = 1,              /// PAL function return unspecify error
    RESULT_BUFFER_NOT_DEFINED = 2, /// return if buffer not defined
    RESULT_EMPTY_BUFFER = 3,       /// return if buffer is empty
    /// Size of buffer is not enough to store output data
    RESULT_BUFFER_OVERFLOW = 4,
    RESULT_PARSE_ERROR = 5,        /// return if not parsed
    RESULT_MEMORY_ERROR = 6,       /// PAL function return memory error
    RESULT_ERROR_INVALID_ARGS = 7, /// some arguments are not valid
    RESULT_ERROR_UNDEFINED = 8,    /// returned value is undefined
    RESULT_ERROR_PAL_NO_INIT = 9,  /// PAL is not initialized
    /// Operation cannot be executed in the current state
    RESULT_ERROR_INVALID_STATE = 10,
    /// Some services or providers are not available
    RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE = 11,
    /// Client doesn't have permissions for requested operation
    RESULT_ERROR_PERMISSIONS = 12,
    RESULT_ERROR_TIMEOUT = 13,     /// Timer timeout occured during operation
    RESULT_ERROR_TRANSACTION = 14,     /// This is IPC Binder specific error
} result_states_t;



/**
 * This method register omadm callback
 * @param[in] omadmCallbackId type of omadm callback
 * @param[in] omadmcallback omadm callback function pointer
 * @return execution status 0 on success or error code
 * on failure.
 */
int pal_register_omadm_callback(omadmCallback_id omadmCallbackId, omadmCallback omadmcallback);

/**
 * This method unregister omadm callback
 * @param[in] omadmCallbackId type of omadm callback
 * @return execution status 0 on success or error code
 * on failure.
 */
int pal_unregister_omadm_callback(omadmCallback_id omadmCallbackId);

////////////////////////////////////////////////////
//             EVENT HANDLER SECTION              //
////////////////////////////////////////////////////

/**
 * Declare event callback function type
 */
typedef int (*process_omadm_event_t)(omadm_request_type_t, void *);

/**
 * Set callback function for WAP push or <93>Check for update<94> events
 *
 * @param[in] process_omadm_event Callback function
 * @return Rresult code
 */
int pal_notify_on_omadm_event(process_omadm_event_t process_omadm_event);

#define PAL_NOTIFY_ON_OMADM_EVENT "pal_notify_on_omadm_event"

//////////////////////////////////////////////////////////////////////////////
//                               NETWORK SECTION                            //
//////////////////////////////////////////////////////////////////////////////
/// HTTP response status codes:
#define HTTP_SERVICE_UNAVAILABLE 503 ///< The server is currently unavailable

#define OMADM_CURL_LOW_SPEED_LIMIT 1000 ///< 1K

#define OMADM_CURL_LOW_SPEED_TIME 20 ///< sec

#define OMADM_CURL_MAXREDIRS 50L

/// It is internal struct definition that we use
/// to allow internal handling of this data
typedef struct
{
    unsigned int data_size; ///< size of data
    char *data_buffer;      ///< pointer to the data buffer
} pal_data_buffer_t;

/// HTTP authentication part
typedef enum
{
    AUTH_UNKNOWN = 0,
    AUTH_BASIC   = 1,
    AUTH_DIGEST  = 2,
    AUTH_HMAC    = 3,
    AUTH_X509    = 4,
} pal_auth_type_t;

/// Parameters for CURL initialisation
typedef struct
{
    int timeout;            ///< the maximum time in seconds that you allow
                            ///< transfer operation to take.
    char *hdr_content_data; ///< Content-Type field is to describe the data type
                            ///< contained in the body
    char *hdr_auth_data;    ///< the nonce value
    char *username;         ///< the user name to be used in protocol authentication
    char *password;         ///< the user password to use in authentication
    char *ca_fname;         ///< path to server's CA certificate file
    char *client_cert;      ///< certificate for client authentication
    char *client_key;       ///< client private key
    pal_auth_type_t  auth_type; ///< type of authentication
} pal_http_options_t;

/**
 *  Global PAL Network module initialize
 *  @return 0 if success
 */
int pal_network_initialize();

/**
 *  Sends a package to server and receives another package in return
 *  @param[in]  server_url  server URI
 *  @param[in]  options parameters for CURL initialisation
 *  @param[in]  data  data which will be sent to the server
 *  @param[out] reply  data received from the server
 *  @return HTTP response status code
 */
long pal_send_http_post_request(char *server_url, void *options, void *data, void *reply);

/**
 *  Request or release a special admin network and binds current process
 *  to this network.
 *  @param[in] enable Request or release admin network.
 *  @return network status
 *  ADMINNW_UNKNOWN, ADMINNW_AVAILABLE, ADMINNW_UNAVAILABLE, ADMIN_NET_LOST
 */
int pal_request_admin_network(bool enable);

/**
 *  Global PAL Network module deinitialize
 *  @return 0 if success
 */
int pal_network_deinitialize();

////////////////////////////////////////////////////////////////////////////////
///    network manager section
////////////////////////////////////////////////////////////////////////////////
/**
 * Declare event callback function type
 */
typedef int (*network_conditions_changed_t)(network_conditions_t);

/// Definition of pal_notify_on_net_manager_event for dynamic linking
#define PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES "pal_notify_on_network_conditions_changed"
/// Definition of pal_notify_stop_on_net_manager_event for dynamic linking
#define PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES "pal_notify_stop_on_network_conditions_changed"
/// Definition of pal_swich_admin_apn for dynamic linking
#define PAL_REQUEST_ADMIN_APN "pal_request_admin_network"

/**
 * Set callback function for process cellular network events
 *
 * @param[in] netm_handler_process_event Callback function
 * @return non-negative id of registered callback if success, otherwise error
 * code.
 */
int pal_notify_on_network_conditions_changed(network_conditions_changed_t
        netm_handler_process_event);

/**
 * Reset callback function to stop processing of cellular network events
 * @param callback_id returned by pal_notify_on_network_conditions_changed call
 * @return result code 0 if success
 */
int pal_notify_stop_on_network_conditions_changed(int callback_id);

__END_DECLS

#endif // PAL_H_
