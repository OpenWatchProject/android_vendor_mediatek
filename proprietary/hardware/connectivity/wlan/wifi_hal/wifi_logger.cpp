#include <stdint.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <linux/rtnetlink.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#include <linux/errqueue.h>

#include <linux/pkt_sched.h>
#include <netlink/object-api.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink-private/object-api.h>
#include <netlink-private/types.h>

#include "nl80211_copy.h"
#include "sync.h"

#define LOG_TAG  "WifiHAL"

#include <log/log.h>

#include "wifi_hal.h"
#include "common.h"
#include "cpp_bindings.h"

enum DEBUG_SUB_COMMAND {
    LOGGER_START_LOGGING = ANDROID_NL80211_SUBCMD_DEBUG_RANGE_START,
    LOGGER_GET_VER,
};

enum LOGGER_ATTRIBUTE {
    LOGGER_ATTRIBUTE_DRIVER_VER,
    LOGGER_ATTRIBUTE_FW_VER,
};

enum GetCmdType {
    GET_FW_VER,
    GET_DRV_VER,
};

///////////////////////////////////////////////////////////////////////////////
class DebugCommand : public WifiCommand
{
    char *mBuff;
    int *mBuffSize;
    GetCmdType mType;

public:

    // constructor for get version
    DebugCommand(wifi_interface_handle iface, char *buffer, int *buffer_size,
            GetCmdType cmdType)
        : WifiCommand("DebugCommand", iface, 0), mBuff(buffer), mBuffSize(buffer_size), mType
        (cmdType)
    {
        memset(mBuff, 0, *mBuffSize);
    }

    int createRequest(WifiRequest &request) {
        int result;

        nlattr *data = NULL;
        switch (mType) {
            case GET_FW_VER:
            case GET_DRV_VER:
                result = request.create(GOOGLE_OUI, LOGGER_GET_VER);
                if (result != WIFI_SUCCESS) {
                    ALOGE("Failed to create get drv version request; result = %d", result);
                    return result;
                }

                data = request.attr_start(NL80211_ATTR_VENDOR_DATA);

                // Driver expecting only attribute type, passing mbuff as data with
                // length 0 to avoid undefined state
                result = request.put(mType == GET_FW_VER ?
                    LOGGER_ATTRIBUTE_FW_VER : LOGGER_ATTRIBUTE_DRIVER_VER, mBuff, 0);
                if (result != WIFI_SUCCESS) {
                    ALOGE("Failed to put get drv version request; result = %d", result);
                    return result;
                }
                request.attr_end(data);
                break;
            default:
                ALOGE("Unknown Debug command");
                result = WIFI_ERROR_UNKNOWN;
        }
        return result;
    }

    int start() {
        // ALOGD("Start debug command");
        WifiRequest request(familyId(), ifaceId());
        int result = createRequest(request);
        if (result != WIFI_SUCCESS) {
            ALOGE("Failed to create debug request; result = %d", result);
            return result;
        }

        result = requestResponse(request);
        if (result != WIFI_SUCCESS) {
            ALOGE("Failed to register debug response; result = %d", result);
        }
        return result;
    }

    virtual int handleResponse(WifiEvent& reply) {
        ALOGD("In DebugCommand::handleResponse");

        if (reply.get_cmd() != NL80211_CMD_VENDOR) {
            ALOGD("Ignoring reply with cmd = %d", reply.get_cmd());
            return NL_SKIP;
        }

        void *data = NULL;
        int len = 0;
        switch (mType) {
            case GET_DRV_VER:
            case GET_FW_VER:
                data = reply.get_vendor_data();
                len = reply.get_vendor_data_len();

                ALOGD("len = %d, expected len = %d", len, *mBuffSize);
                memcpy(mBuff, data, min(len, *mBuffSize));
                if (*mBuffSize < len)
                    return NL_SKIP;
                *mBuffSize = len;
                break;
            default:
                ALOGW("Unknown Debug command");
        }
        return NL_OK;
    }

    virtual int handleEvent(WifiEvent& event) {
        /* NO events! */
        return NL_SKIP;
    }
};

wifi_error get_version(wifi_interface_handle iface, char *buffer, int *buffer_size, GetCmdType type)
{
    if (buffer && (*buffer_size > 0)) {
        DebugCommand *cmd = new DebugCommand(iface, buffer, buffer_size, type);
        NULL_CHECK_RETURN(cmd, "memory allocation failure", WIFI_ERROR_OUT_OF_MEMORY);
        wifi_error result = (wifi_error)cmd->start();
        cmd->releaseRef();
        return result;
    } else {
        ALOGE("%s version buffer NULL", type == GET_FW_VER ? "FW" : "Driver");
        return  WIFI_ERROR_INVALID_ARGS;
    }
}

/* API to collect a firmware version string */
wifi_error wifi_get_firmware_version(wifi_interface_handle iface, char *buffer,
        int buffer_size)
{
    return get_version(iface, buffer, &buffer_size, GET_FW_VER);
}

/* API to collect a driver version string */
wifi_error wifi_get_driver_version(wifi_interface_handle iface, char *buffer, int buffer_size)
{
    return get_version(iface, buffer, &buffer_size, GET_DRV_VER);
}
