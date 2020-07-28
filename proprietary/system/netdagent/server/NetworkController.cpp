/*
 * Copyright (C) 2014 The Android Open Source Project
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

// THREAD-SAFETY
// -------------
// The methods in this file are called from multiple threads (from CommandListener, FwmarkServer
// and DnsProxyListener). So, all accesses to shared state are guarded by a lock.
//
// In some cases, a single non-const method acquires and releases the lock several times, like so:
//     if (isValidNetwork(...)) {  // isValidNetwork() acquires and releases the lock.
//        setDefaultNetwork(...);  // setDefaultNetwork() also acquires and releases the lock.
//
// It might seem that this allows races where the state changes between the two statements, but in
// fact there are no races because:
//     1. This pattern only occurs in non-const methods (i.e., those that mutate state).
//     2. Only CommandListener calls these non-const methods. The others call only const methods.
//     3. CommandListener only processes one command at a time. I.e., it's serialized.
// Thus, no other mutation can occur in between the two statements above.

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/fib_rules.h>
#include <sys/stat.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>

#include "NetlinkCommands.h"
#include "NetdagentUtils.h"
#include "IptablesInterface.h"
#include "NetworkController.h"
#define LOG_TAG "NetdagentNetwork"
#include "log/log.h"

namespace android {
namespace netdagent {

const char* const NetworkController::IIF_NONE = NULL;
const char* const NetworkController::OIF_NONE = NULL;
const bool NetworkController::ACTION_ADD = true;
const bool NetworkController::ACTION_DEL = false;

const uint32_t NetworkController::ROUTE_TABLE_LOCAL_NETWORK  = 97;
const uint32_t NetworkController::ROUTE_TABLE_LEGACY_NETWORK = 98;
const uint32_t NetworkController::ROUTE_TABLE_LEGACY_SYSTEM  = 99;

const char* const NetworkController::ROUTE_TABLE_NAME_LOCAL_NETWORK  = "local_network";
const char* const NetworkController::ROUTE_TABLE_NAME_LEGACY_NETWORK = "legacy_network";
const char* const NetworkController::ROUTE_TABLE_NAME_LEGACY_SYSTEM  = "legacy_system";

const char* const NetworkController::ROUTE_TABLE_NAME_LOCAL = "local";
const char* const NetworkController::ROUTE_TABLE_NAME_MAIN  = "main";

const int NetworkController::ROUTE_TABLE_OFFSET_FROM_INDEX = 1000;

const char* NetworkController::LOCAL_FILTER_FORWARD = "oem_fwd";  //AOSP chain
const char* NetworkController::LOCAL_MANGLE_PREROUTING = "oem_mangle_pre";  //mtk new chain
const char* NetworkController::LOCAL_FILTER_OUT = "oem_out";  //AOSP chain
const char* NetworkController::LOCAL_FILTER_INPUT = "oem_input";  //mtk new chain

// Avoids "non-constant-expression cannot be narrowed from type 'unsigned int' to 'unsigned short'"
// warnings when using RTA_LENGTH(x) inside static initializers (even when x is already uint16_t).
constexpr uint16_t U16_RTA_LENGTH(uint16_t x) {
    return RTA_LENGTH(x);
}

rtattr RTATTR_TABLE     = { U16_RTA_LENGTH(sizeof(uint32_t)),           RTA_TABLE };
rtattr RTATTR_OIF       = { U16_RTA_LENGTH(sizeof(uint32_t)),           RTA_OIF };

uint8_t PADDING_BUFFER[RTA_ALIGNTO] = {0, 0, 0, 0};

const char *actionName(uint16_t action) {
    static const char *ops[4] = {"adding", "deleting", "getting", "???"};
    return ops[action % 4];
}

const char *familyName(uint8_t family) {
    switch (family) {
        case AF_INET: return "IPv4";
        case AF_INET6: return "IPv6";
        default: return "???";
    }
}

static int parsePrefix(const char *prefix, uint8_t *family, void *address, int size, uint8_t *prefixlen) {
    if (!prefix || !family || !address || !prefixlen) {
        return -EFAULT;
    }

    // Find the '/' separating address from prefix length.
    const char *slash = strchr(prefix, '/');
    const char *prefixlenString = slash + 1;
    if (!slash || !*prefixlenString)
        return -EINVAL;

    // Convert the prefix length to a uint8_t.
    char *endptr;
    unsigned templen;
    templen = strtoul(prefixlenString, &endptr, 10);
    if (*endptr || templen > 255) {
        return -EINVAL;
    }
    *prefixlen = templen;

    // Copy the address part of the prefix to a local buffer. We have to copy
    // because inet_pton and getaddrinfo operate on null-terminated address
    // strings, but prefix is const and has '/' after the address.
    std::string addressString(prefix, slash - prefix);

    // Parse the address.
    addrinfo *res;
    addrinfo hints = {
        .ai_flags = AI_NUMERICHOST,
    };
    int ret = getaddrinfo(addressString.c_str(), NULL, &hints, &res);
    if (ret || !res) {
        return -EINVAL;  // getaddrinfo return values are not errno values.
    }

    // Convert the address string to raw address bytes.
    void *rawAddress;
    int rawLength;
    switch (res[0].ai_family) {
        case AF_INET: {
            if (*prefixlen > 32) {
                return -EINVAL;
            }
            sockaddr_in *sin = (sockaddr_in *) res[0].ai_addr;
            rawAddress = &sin->sin_addr;
            rawLength = 4;
            break;
        }
        case AF_INET6: {
            if (*prefixlen > 128) {
                return -EINVAL;
            }
            sockaddr_in6 *sin6 = (sockaddr_in6 *) res[0].ai_addr;
            rawAddress = &sin6->sin6_addr;
            rawLength = 16;
            break;
        }
        default: {
            freeaddrinfo(res);
            return -EAFNOSUPPORT;
        }
    }

    if (rawLength > size) {
        freeaddrinfo(res);
        return -ENOSPC;
    }

    *family = res[0].ai_family;
    memcpy(address, rawAddress, rawLength);
    freeaddrinfo(res);

    return rawLength;
}


std::map<std::string, uint32_t> interfaceToTable;
uint32_t NetworkController::getRouteTableForInterface(const char* interface) {
    uint32_t index = if_nametoindex(interface);
    if (index) {
        index += ROUTE_TABLE_OFFSET_FROM_INDEX;
        interfaceToTable[interface] = index;
        return index;
    }
    // If the interface goes away if_nametoindex() will return 0 but we still need to know
    // the index so we can remove the rules and routes.
    auto iter = interfaceToTable.find(interface);
    if (iter == interfaceToTable.end()) {
        ALOGE("cannot find interface %s", interface);
        return RT_TABLE_UNSPEC;
    }
    return iter->second;
}

// Adds or deletes an IPv4 or IPv6 route.
// Returns 0 on success or negative errno on failure.
int NetworkController::modifyIpRoute(uint16_t action, uint32_t table, const char* interface,
                                     const char* destination, const char* nexthop) {
    // At least the destination must be non-null.
    if (!destination) {
        ALOGE("null destination");
        return -EFAULT;
    }

    // Parse the prefix.
    uint8_t rawAddress[sizeof(in6_addr)];
    uint8_t family;
    uint8_t prefixLength;
    int rawLength = parsePrefix(destination, &family, rawAddress, sizeof(rawAddress),
                                &prefixLength);
    if (rawLength < 0) {
        ALOGE("parsePrefix failed for destination %s (%s)", destination, strerror(-rawLength));
        return rawLength;
    }

    if (static_cast<size_t>(rawLength) > sizeof(rawAddress)) {
        ALOGE("impossible! address too long (%d vs %zu)", rawLength, sizeof(rawAddress));
        return -ENOBUFS;  // Cannot happen; parsePrefix only supports IPv4 and IPv6.
    }

    uint8_t type = RTN_UNICAST;
    uint32_t ifindex;
    uint8_t rawNexthop[sizeof(in6_addr)];

    if (nexthop && !strcmp(nexthop, "unreachable")) {
        type = RTN_UNREACHABLE;
        // 'interface' is likely non-NULL, as the caller (modifyRoute()) likely used it to lookup
        // the table number. But it's an error to specify an interface ("dev ...") or a nexthop for
        // unreachable routes, so nuke them. (IPv6 allows them to be specified; IPv4 doesn't.)
        interface = OIF_NONE;
        nexthop = NULL;
    } else if (nexthop && !strcmp(nexthop, "throw")) {
        type = RTN_THROW;
        interface = OIF_NONE;
        nexthop = NULL;
    } else {
        // If an interface was specified, find the ifindex.
        if (interface != OIF_NONE) {
            ifindex = if_nametoindex(interface);
            if (!ifindex) {
                ALOGE("cannot find interface %s", interface);
                return -ENODEV;
            }
        }

        // If a nexthop was specified, parse it as the same family as the prefix.
        if (nexthop && inet_pton(family, nexthop, rawNexthop) <= 0) {
            ALOGE("inet_pton failed for nexthop %s", nexthop);
            return -EINVAL;
        }
    }

    // Assemble a rtmsg and put it in an array of iovec structures.
    rtmsg route = {
        .rtm_protocol = RTPROT_STATIC,
        .rtm_type = type,
        .rtm_family = family,
        .rtm_dst_len = prefixLength,
        .rtm_scope = static_cast<uint8_t>(nexthop ? RT_SCOPE_UNIVERSE : RT_SCOPE_LINK),
    };

    rtattr rtaDst     = { U16_RTA_LENGTH(rawLength), RTA_DST };
    rtattr rtaGateway = { U16_RTA_LENGTH(rawLength), RTA_GATEWAY };

    iovec iov[] = {
        { NULL,          0 },
        { &route,        sizeof(route) },
        { &RTATTR_TABLE, sizeof(RTATTR_TABLE) },
        { &table,        sizeof(table) },
        { &rtaDst,       sizeof(rtaDst) },
        { rawAddress,    static_cast<size_t>(rawLength) },
        { &RTATTR_OIF,   interface != OIF_NONE ? sizeof(RTATTR_OIF) : 0 },
        { &ifindex,      interface != OIF_NONE ? sizeof(ifindex) : 0 },
        { &rtaGateway,   nexthop ? sizeof(rtaGateway) : 0 },
        { rawNexthop,    nexthop ? static_cast<size_t>(rawLength) : 0 },
    };

    uint16_t flags = (action == RTM_NEWROUTE) ? NETLINK_CREATE_REQUEST_FLAGS :
                                                NETLINK_REQUEST_FLAGS;
    int ret = sendNetlinkRequest(action, flags, iov, ARRAY_SIZE(iov), nullptr);
    if (ret) {
        ALOGE("Error %s route %s -> %s %s to table %u: %s",
              actionName(action), destination, nexthop, interface, table, strerror(-ret));
    }
    return ret;
}



// Adds or removes an IPv4 or IPv6 route to the specified table and, if it's a directly-connected
// route, to the main table as well.
// Returns 0 on success or negative errno on failure.
int NetworkController::modifyRoute(uint16_t action, const char* interface, const char* destination,
                                   const char* nexthop, TableType tableType) {
    uint32_t table;
    switch (tableType) {
        case INTERFACE: {
            table = getRouteTableForInterface(interface);
            if (table == RT_TABLE_UNSPEC) {
                return -ESRCH;
            }
            break;
        }
        case LOCAL_NETWORK: {
            table = ROUTE_TABLE_LOCAL_NETWORK;
            break;
        }
        case LEGACY_NETWORK: {
            table = ROUTE_TABLE_LEGACY_NETWORK;
            break;
        }
        case LEGACY_SYSTEM: {
            table = ROUTE_TABLE_LEGACY_SYSTEM;
            break;
        }
    }

    int ret = modifyIpRoute(action, table, interface, destination, nexthop);
    // Trying to add a route that already exists shouldn't cause an error.
    if (ret && !(action == RTM_NEWROUTE && ret == -EEXIST)) {
        return ret;
    }

    return 0;
}

int NetworkController::addRoute(const char* interface, const char* destination, const char* nexthop,
                              TableType tableType) {
    return modifyRoute(RTM_NEWROUTE, interface, destination, nexthop, tableType);
}

int NetworkController::removeRoute(const char* interface, const char* destination,
                                 const char* nexthop, TableType tableType) {
    return modifyRoute(RTM_DELROUTE, interface, destination, nexthop, tableType);
}


static uint32_t getRulePriority(const nlmsghdr *nlh) {
    return getRtmU32Attribute(nlh, FRA_PRIORITY);
}

static uint32_t getRouteTable(const nlmsghdr *nlh) {
    return getRtmU32Attribute(nlh, RTA_TABLE);
}

int NetworkController::flushRules() {
    NetlinkDumpFilter shouldDelete = [] (nlmsghdr *nlh) {
        // Don't touch rules at priority 0 because by default they are used for local input.
        return getRulePriority(nlh) != 0;
    };
    return rtNetlinkFlush(RTM_GETRULE, RTM_DELRULE, "rules", shouldDelete);
}

int NetworkController::flushRoutes(uint32_t table) {
    NetlinkDumpFilter shouldDelete = [table] (nlmsghdr *nlh) {
        return getRouteTable(nlh) == table;
    };

    return rtNetlinkFlush(RTM_GETROUTE, RTM_DELROUTE, "routes", shouldDelete);
}

// Returns 0 on success or negative errno on failure.
int NetworkController::flushRoutes(const char* interface) {
    uint32_t table = getRouteTableForInterface(interface);
    if (table == RT_TABLE_UNSPEC) {
        return -ESRCH;
    }

    int ret = flushRoutes(table);

    // If we failed to flush routes, the caller may elect to keep this interface around, so keep
    // track of its name.
    if (ret == 0) {
        interfaceToTable.erase(interface);
    }

    return ret;
}

//mtk for ipsec MTU issue
int NetworkController::forwardIpsec(char *inIface, char *outIface, char *nxthop, char *tableId, int family, int enable)
{
    int res = 0;
    const char *FORWARD_MARK = "0x10000";
    if (!isIfaceName(inIface) || !isIfaceName(outIface)) {
        return -1;
    }
    ALOGI("Setting IP forward enable = %d from %s to %s\n", enable, inIface, outIface);
    if(enable) {
        //enable forwarding
        res |= execNdcCmd("ipfwd", "enable", "ipsec", NULL);
        //add rorward mark
        res |= execIptables(V4V6, "-t", "mangle", "-I", LOCAL_MANGLE_PREROUTING, "-i", inIface, "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
        //add forward exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_FORWARD, "-i", inIface, "-o", outIface, "-j", "ACCEPT", NULL);
        //add powersave or dozable output exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_OUT, "-o", outIface, "-m", "mark", "--mark", FORWARD_MARK, "-j", "ACCEPT", NULL);
        //add powersave or dozable input exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_INPUT, "-i", outIface, "-j", "ACCEPT", NULL);
        //add forward route
        res |= execIpCmd(family, "route", "add", nxthop, "dev", outIface, "table", tableId, NULL);
    } else {
        //disable forwarding
        res |= execNdcCmd("ipfwd", "disable", "ipsec", NULL);
        //del forward mark
        res |= execIptables(V4V6, "-t", "mangle", "-D", LOCAL_MANGLE_PREROUTING, "-i", inIface, "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
        //del forward exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_FORWARD, "-i", inIface, "-o", outIface, "-j", "ACCEPT", NULL);
        //del powersave or dozable output exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_OUT, "-o", outIface, "-m", "mark", "--mark", FORWARD_MARK, "-j", "ACCEPT", NULL);
        //del powersave or dozable input exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_INPUT, "-i", outIface, "-j", "ACCEPT", NULL);
        //del forward route
        res |= execIpCmd(family, "route", "del", nxthop, "dev", outIface, "table", tableId, NULL);
    }
    return res;
}

}  // namespace netdagent
}  // namespace android
