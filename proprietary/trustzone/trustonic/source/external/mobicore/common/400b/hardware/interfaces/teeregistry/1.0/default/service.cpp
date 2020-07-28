/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.trustonic.teeregistry@1.0-service"
//#define LOG_NDEBUG 0

#include <string>

#include <unistd.h>
#include <getopt.h>

#include <hidl/HidlTransportSupport.h>
#include <utils/Log.h>

#include "TeeRegistry.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

using vendor::trustonic::teeregistry::V1_0::ITeeRegistry;
using vendor::trustonic::teeregistry::V1_0::implementation::TeeRegistry;

int main(int argc, char** argv) {
    // Get option for registry location
    std::string registry_path = "/data/vendor/mcRegistry";
    while (true) {
        static struct option opts[] = {
            {"registry_path", required_argument, nullptr, 'p'},
        };

        int option_index = 0;
        int c = ::getopt_long(argc, argv, "", opts, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                registry_path = optarg;
                break;
            case 'v':
                printf("Trustonic Tee registry server\n");
                return 0;
            default:
                // getopt already prints for us.
                return 1;
        }
    }

    configureRpcThreadpool(10, true);

    ::android::sp<ITeeRegistry> teeregistry = new TeeRegistry(registry_path);
    ::android::status_t status = teeregistry->registerAsService();
    LOG_ALWAYS_FATAL_IF(status != ::android::OK, "Could not register ITeeRegistry");

    // other interface registration comes here
    joinRpcThreadpool();
    return 0;
}
