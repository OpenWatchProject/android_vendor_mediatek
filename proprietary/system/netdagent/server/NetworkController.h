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

#ifndef NETD_SERVER_NETWORK_CONTROLLER_H
#define NETD_SERVER_NETWORK_CONTROLLER_H

#include <list>
#include <map>
#include <set>
#include <vector>
#include <sys/types.h>

namespace android {
namespace netdagent {

/*
 * Keeps track of default, per-pid, and per-uid-range network selection, as
 * well as the mark associated with each network. Networks are identified
 * by netid. In all set* commands netid == 0 means "unspecified" and is
 * equivalent to clearing the mapping.
 */
class NetworkController {

private:

    // How the routing table number is determined for route modification requests.
    enum TableType {
        INTERFACE,       // Compute the table number based on the interface index.
        LOCAL_NETWORK,   // A fixed table used for routes to directly-connected clients/peers.
        LEGACY_NETWORK,  // Use a fixed table that's used to override the default network.
        LEGACY_SYSTEM,   // A fixed table, only modifiable by system apps; overrides VPNs too.
    };

    uint32_t getRouteTableForInterface(const char* interface);
    int modifyIpRoute(uint16_t action, uint32_t table, const char* interface,
                                     const char* destination, const char* nexthop);
    int modifyRoute(uint16_t action, const char* interface, const char* destination,
                                         const char* nexthop, TableType tableType);

    static const char* const IIF_NONE;
    static const char* const OIF_NONE;
    static const bool ACTION_ADD;
    static const bool ACTION_DEL;

    static const uint32_t ROUTE_TABLE_LOCAL_NETWORK;
    static const uint32_t ROUTE_TABLE_LEGACY_NETWORK;
    static const uint32_t ROUTE_TABLE_LEGACY_SYSTEM;

    static const char* const ROUTE_TABLE_NAME_LOCAL_NETWORK;
    static const char* const ROUTE_TABLE_NAME_LEGACY_NETWORK;
    static const char* const ROUTE_TABLE_NAME_LEGACY_SYSTEM;

    static const char* const ROUTE_TABLE_NAME_LOCAL;
    static const char* const ROUTE_TABLE_NAME_MAIN;

    static const int ROUTE_TABLE_OFFSET_FROM_INDEX;

public:

    int addRoute(const char* interface, const char* destination, const char* nexthop,
                              TableType tableType);
    int removeRoute(const char* interface, const char* destination,
                                 const char* nexthop, TableType tableType);
    int flushRules();
    int flushRoutes(uint32_t table);
    int flushRoutes(const char* interface);
    int forwardIpsec(char *inIface, char *outIface, char *nxthop, char *tableId,
                           int family, int enable);

    static const char* LOCAL_FILTER_FORWARD;  //AOSP chain
    static const char* LOCAL_MANGLE_PREROUTING;  //mtk new chain
    static const char* LOCAL_FILTER_INPUT;  //mtk new chain
    static const char* LOCAL_FILTER_OUT;  //AOSP chain

};

}  // namespace netdagent
}  // namespace android

#endif  // NETD_SERVER_NETWORK_CONTROLLER_H
