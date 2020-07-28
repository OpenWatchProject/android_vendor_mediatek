
#include "mnl_agps_interface.h"

const char* get_mnl_agps_type_str(mnl_agps_type input) {
    switch (input) {
    case MNL_AGPS_TYPE_MNL_REBOOT:
        return "MNL_REBOOT";
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_DONE:
        return "AGPS_OPEN_GPS_DONE";
    case MNL_AGPS_TYPE_AGPS_CLOSE_GPS_DONE:
        return "AGPS_CLOSE_GPS_DONE";
    case MNL_AGPS_TYPE_AGPS_RESET_GPS_DONE:
        return "AGPS_RESET_GPS_DONE";
    case MNL_AGPS_TYPE_GPS_INIT:
        return "GPS_INIT";
    case MNL_AGPS_TYPE_GPS_CLEANUP:
        return "GPS_CLEANUP";
    case MNL_AGPS_TYPE_SET_SERVER:
        return "SET_SERVER";
    case MNL_AGPS_TYPE_DELETE_AIDING_DATA:
        return "DELETE_AIDING_DATA";
    case MNL_AGPS_TYPE_GPS_OPEN:
        return "GPS_OPEN";
    case MNL_AGPS_TYPE_GPS_CLOSE:
        return "GPS_CLOSE";
    case MNL_AGPS_TYPE_DATA_CONN_OPEN:
        return "DATA_CONN_OPEN";
    case MNL_AGPS_TYPE_DATA_CONN_FAILED:
        return "DATA_CONN_FAILED";
    case MNL_AGPS_TYPE_DATA_CONN_CLOSED:
        return "DATA_CONN_CLOSED";
    case MNL_AGPS_TYPE_NI_MESSAGE:
        return "NI_MESSAGE";
    case MNL_AGPS_TYPE_NI_RESPOND:
        return "NI_RESPOND";
    case MNL_AGPS_TYPE_SET_REF_LOC:
        return "SET_REF_LOC";
    case MNL_AGPS_TYPE_SET_SET_ID:
        return "SET_SET_ID";
    case MNL_AGPS_TYPE_UPDATE_NETWORK_STATE:
        return "UPDATE_NETWORK_STATE";
    case MNL_AGPS_TYPE_UPDATE_NETWORK_AVAILABILITY:
        return "UPDATE_NETWORK_AVAILABILITY";
    case MNL_AGPS_TYPE_DATA_CONN_OPEN_IP_TYPE:
        return "DATA_CONN_OPEN_IP_TYPE";
    case MNL_AGPS_TYPE_INSTALL_CERTIFICATES:
        return "INSTALL_CERTIFICATES";
    case MNL_AGPS_TYPE_REVOKE_CERTIFICATES:
        return "REVOKE_CERTIFICATES";
    case MNL_AGPS_TYPE_UPDATE_GNSS_ACCESS_CONTROL:
        return "UPDATE_GNSS_ACCESS_CONTROL";
    case MNL_AGPS_TYPE_MNL2AGPS_PMTK:
        return "MNL2AGPS_PMTK";
    case MNL_AGPS_TYPE_RAW_DBG:
        return "RAW_DBG";
    case MNL_AGPS_TYPE_REAIDING:
        return "REAIDING_REQ";
    case MNL_AGPS_TYPE_AGPS_REBOOT:
        return "AGPS_REBOOT";
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_REQ:
        return "AGPS_OPEN_GPS_REQ";
    case MNL_AGPS_TYPE_AGPS_CLOSE_GPS_REQ:
        return "AGPS_CLOSE_GPS_REQ";
    case MNL_AGPS_TYPE_AGPS_RESET_GPS_REQ:
        return "AGPS_RESET_GPS_REQ";
    case MNL_AGPS_TYPE_AGPS_OPEN_GPS_REJECTED:
        return "AGPS_OPEN_GPS_REJECTED";
    case MNL_AGPS_TYPE_AGPS_SESSION_DONE:
        return "AGPS_SESSION_DONE";
    case MNL_AGPS_TYPE_NI_NOTIFY:
        return "NI_NOTIFY";
    case MNL_AGPS_TYPE_NI_NOTIFY_2:
        return "NI_NOTIFY2";
    case MNL_AGPS_TYPE_DATA_CONN_REQ:
        return "DATA_CONN_REQ";
    case MNL_AGPS_TYPE_DATA_CONN_REQ2:
        return "DATA_CONN_REQ2";
    case MNL_AGPS_TYPE_DATA_CONN_RELEASE:
        return "DATA_CONN_RELEASE";
    case MNL_AGPS_TYPE_SET_ID_REQ:
        return "SET_ID_REQ";
    case MNL_AGPS_TYPE_REF_LOC_REQ:
        return "REF_LOC_REQ";
    case MNL_AGPS_TYPE_AGPS2MNL_PMTK:
        return "AGPS2MNL_PMTK";
    case MNL_AGPS_TYPE_GPEVT:
        return "GPEVT";
    case MNL_AGPS_TYPE_AGPS_LOC:
        return "AGPS_LOC";
    default:
        break;
    }
    return "UNKNOWN";
}

