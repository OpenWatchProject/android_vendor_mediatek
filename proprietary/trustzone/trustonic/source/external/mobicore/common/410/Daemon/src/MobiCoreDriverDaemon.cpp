/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#include <string>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <libgen.h>
#include <getopt.h>
#include <sys/types.h>

// Version
#include "mc_user.h"

#include "dynamic_log.h"
#include "buildTag.h"

#include "PrivateRegistry.h"    // setSearchPaths
#include "IService.h"
#include "SecureWorld.h"
#include "EndorsementInstaller.h"
#include "FSD2.h"
#include "RegistryServer.h"
#include "DebugSession.h"
#include "TuiStarter.h"

#define DRIVER_TCI_LEN 4096

static std::vector<IService*> services;

/**
 * Print daemon command line options
 */
static void printUsage(const char* path) {
    printf("TEE Driver Daemon %u.%u. \"%s\"\n",
           MCDRVMODULEAPI_VERSION_MAJOR, MCDRVMODULEAPI_VERSION_MINOR,
           MOBICORE_COMPONENT_BUILD_TAG);
    printf("usage: %s [OPTIONS]\n", path);
    printf("Start the TEE Daemon\n\n");
    printf("  -h\t\t\t\tshow this help and exit\n");
    printf("  -v\t\t\t\tprint version and exit\n");
    printf("  -b\t\t\t\tfork to background\n");
    printf("  -d\t\t\t\tspecify the path to the decryption key\n");
    printf("  -l\t\t\t\tdo not start FSD/TUI services\n");
    printf("  -p REGISTRY\t\t\tspecify path to registry (may be repeated)\n");
    printf("  -r DRIVER\t\t\tsecure driver to load at start-up (may be repeated)\n");
    printf("  -r DRIVER\t\t\tIt could be a /path/to/secure/driver.tlbin or a UUID 000102030405060708090A0B0C0D0E0F\n");
    printf("  --Px PARTITION[x] PATH\tspecify the directory where the secure partition is stored, 0 <= x <= 15\n");
}

static void signal_handler(int signum) {
    LOG_I("Signal %d received", signum);
    for (auto it = services.begin(); it != services.end(); it++) {
        auto service = *it;
        service->receiveSignal(signum);
    }
}

/**
 * Main entry of the TEE Driver Daemon.
 */
int main(int argc, char* args[]) {
    SecureWorld secure_world;
    // Read the Command line options
    const char* base_name = basename(args[0]);
    extern char* optarg;
    extern int optopt, opterr;

    std::vector<std::string> drivers;
    std::vector<std::string> registry_paths;
    std::vector<std::string> partition_paths(16);
    std::string decryption_key;

    // Defaults:
    // *  don't fork
    bool is_daemon = false;
    // * load all services
    bool light_mode = false;

    opterr = 0;
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help",         no_argument,        0, 'h'},
        {"backgroung",   no_argument,        0, 'b'},
        {"decryptKeySO", required_argument,  0, 'd'},
        {"light_mode",   no_argument,        0, 'l'},
        {"driver",       required_argument,  0, 'r'},
        {"registryPath", required_argument,  0, 'p'},
        {"version",      no_argument,        0, 'v'},
        {"P0",           required_argument,  0, 0},
        {"P1",           required_argument,  0, 0},
        {"P2",           required_argument,  0, 0},
        {"P3",           required_argument,  0, 0},
        {"P4",           required_argument,  0, 0},
        {"P5",           required_argument,  0, 0},
        {"P6",           required_argument,  0, 0},
        {"P7",           required_argument,  0, 0},
        {"P8",           required_argument,  0, 0},
        {"P9",           required_argument,  0, 0},
        {"P10",          required_argument,  0, 0},
        {"P11",          required_argument,  0, 0},
        {"P12",          required_argument,  0, 0},
        {"P13",          required_argument,  0, 0},
        {"P14",          required_argument,  0, 0},
        {"P15",          required_argument,  0, 0},
        {0,              0,                  0, 0}
    };
    while ((opt = ::getopt_long(argc, args, "bd:hlp:r:v", long_options, &option_index)) != -1) {
        bool failed = false;
        int partitionId;
        switch (opt) {
            case 'h': /* Help */
                printUsage(base_name);
                return 0;
            case 'b': /* Fork to background */
                is_daemon = true;
                break;
            case 'd': /* Load decryption key SO at start-up */
                decryption_key = optarg;
                LOG_D("decryption_key %s will be loaded", optarg);
                break;
            case 'l': /* No FSD/TUI */
                light_mode = true;
                break;
            case 'p': /* Search paths for registry */
                registry_paths.push_back(optarg);
                LOG_D("registry search path %s added", optarg);
                break;
            case 'r': /* Load secure driver at start-up */
                drivers.push_back(optarg);
                LOG_D("driver %s enqueued for loading", optarg);
                break;
            case 'v': /* Print version */
                printf("%s v%d.%d\n", base_name, MCDRVMODULEAPI_VERSION_MAJOR,
                       MCDRVMODULEAPI_VERSION_MINOR);
                return 0;
            case 0 :
                // Only support Px[y] long option
                if (long_options[option_index].name[0] != 'P') {
                    failed = true;
                    break;
                }
                partitionId = atoi(long_options[option_index].name + 1);
                if ((partitionId < 0) || (partitionId >= 16)) {
                    failed = true;
                    break;
                }
                partition_paths[partitionId] = optarg;
                LOG_D("Specific path for partition %d: %s", partitionId, optarg);
                break;
            case '?':
                if (optopt == 0) {
                    // long option
                    fprintf(stderr, "%s: invalid option \n", base_name);
                } else if ((optopt == 'p') || (optopt == 'r')) {
                    fprintf(stderr, "%s: missing argument to '-%c'\n", base_name, optopt);
                } else {
                    fprintf(stderr, "%s: invalid option '-%c'\n", base_name, optopt);
                }

                failed = true;
                break;
        }

        if (failed) {
            fprintf(stderr, "Try '%s -h' for more information.\n", base_name);
            return 2;
        }
    }

    // Default registry paths if none specified (for Android)
    if (registry_paths.empty()) {
        registry_paths.push_back("/data/vendor/mcRegistry");
        registry_paths.push_back("/vendor/app/mcRegistry");
        registry_paths.push_back("/system/app/mcRegistry");
    }
    setSearchPaths(registry_paths);
    LOG_I("Registry search paths:");
    for (auto path = registry_paths.begin(); path != registry_paths.end(); path++) {
        LOG_I("  %s", path->c_str());
    }

    /* Set main thread's signal mask to block SIGUSR1.
     * All other threads will inherit mask and have it blocked too */
    sigset_t sigs_to_block;
    ::sigemptyset(&sigs_to_block);
    ::sigaddset(&sigs_to_block, SIGUSR1);
    ::pthread_sigmask(SIG_BLOCK, &sigs_to_block, NULL);

    // Process termination action
    struct sigaction sigaction;
    ::memset(&sigaction, 0, sizeof(sigaction));
    ::sigemptyset(&sigaction.sa_mask);
    sigaction.sa_handler = signal_handler;
    if (::sigaction(SIGINT, &sigaction, NULL)) {
        LOG_ERRNO("sigaction");
        return EXIT_FAILURE;
    }
    if (::sigaction(SIGTERM, &sigaction, NULL)) {
        LOG_ERRNO("sigaction");
        return EXIT_FAILURE;
    }
    if (::sigaction(SIGUSR2, &sigaction, NULL)) {
        LOG_ERRNO("sigaction");
        return EXIT_FAILURE;
    }
    ::signal(SIGPIPE, SIG_IGN);

    // Open Secure World access
    LOG_I("Initialise Secure World");
    if (secure_world.setup() < 0) {
        LOG_E("Can't open secure world");
        return EXIT_FAILURE;
    }

    // Socket server for registry access
    RegistryServer registry_server(&secure_world);

    // send decryption key to the TEE if found
    LOG_I("Load decryption key");
    if (secure_world.loadDecryptionKey(decryption_key)) {
        if (errno != ENOENT) {
            LOG_E("decryption key %s couldn't be loaded: %s",
                    decryption_key.c_str(), strerror(errno));
        } else {
            LOG_W("key decryption %s missing, ignore", decryption_key.c_str());
        }
    }

    // We should fork the daemon to background
    if (is_daemon) {
        /* ignore "terminal has been closed" signal */
        signal(SIGHUP, SIG_IGN);

        if (daemon(0, 0) < 0) {
            fprintf(stderr, "Fork failed, exiting...\n");
            return EXIT_FAILURE;
        }
        LOG_I("Daemon forked");

        /* ignore tty signals */
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
    }

    LOG_I("%s", MOBICORE_COMPONENT_BUILD_TAG);

    // File system for TEE
    FileSystem file_system(partition_paths);
    // Thread that waits for and installs endorsement token
    EndorsementInstaller endorsement_installer(registry_server, registry_paths[0]);
    // Debug
    DebugSession debug_session(&secure_world);

    // Add all services in open order
    services.push_back(&secure_world);
    if (!light_mode) {
        services.push_back(&file_system);
        services.push_back(&endorsement_installer);
    }
    services.push_back(&registry_server);
    services.push_back(&debug_session);

    // The thread needs to be started after the daemon's forking
    // otherwise it causes some problems accessing ioctl's.
    // Also, it needs to be running before any session can be open, so before
    // the calls to open() below.
    LOG_I("Start services");
    auto it_service = services.begin();
    while (it_service != services.end()) {
        auto service = *it_service;
        LOG_I("* start %s", service->displayName());
        if (service->open()) {
            LOG_ERRNO(service->displayName());
            break;
        }
        it_service++;
    }

    if (it_service == services.end()) {
        // Load SW device driver(s), if requested. The drivers need the services
        // (SecureWorld, FileSystem) to be ready.
        auto it_driver = drivers.begin();
        if (it_driver != drivers.end()) {
            LOG_I("Load secure drivers");
            while (it_driver != drivers.end()) {
                auto driver = *it_driver;
                LOG_I("* loading %s", driver.c_str());
                if (secure_world.loadDriver(driver)) {
                    if (errno != ENOENT) {
                        LOG_E("Driver %s couldn't be loaded: %s",
                              driver.c_str(), strerror(errno));
                        break;
                    }

                    // Some customers require that the same BSP have different
                    // installed drivers on different platforms, so we ignore a
                    // missing driver
                    LOG_W("Driver %s missing, ignore", driver.c_str());
                }
                it_driver++;
            }
        }

        if (it_driver == drivers.end()) {
            // Trigger a thread in the Trusted UI Linux driver which loads the
            // Trusted UI secure driver.
            // This needs the drivers to be loaded first, as they may provide
            // anti-rollback protection.
            if (!light_mode) {
                TrustedUI::kick();
            }

            // Keep running if there is no error and until told otherwise
            LOG_I("Daemon running");
            secure_world.wait();
            LOG_I("Daemon stopping");
        }
    }

    // Make sure we die eventually
    alarm(5);

    // Close all services that were open
    LOG_I("Stop services");
    std::reverse_iterator< std::vector<IService*>::iterator > reverse_it_service(it_service);
    while (reverse_it_service != services.rend()) {
        auto service = *reverse_it_service;
        service->receiveSignal(0);
        LOG_I("* stop %s", service->displayName());
        service->close();
        reverse_it_service ++;
    }

    LOG_I("Daemon exiting");
    return EXIT_SUCCESS;
}
