/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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
#include <sys/types.h>
#include <getopt.h>

// Version
#include "mc_user.h"

#include <log.h>
#include "buildTag.h"

#include "PrivateRegistry.h"    // setSearchPaths
#include "IService.h"
#include "SecureWorld.h"
#include "FSD2Wrapper.h"
#include "UpgradeStorage.h"
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
    printf("  -h\t\tshow this help and exit\n");
    printf("  -v\t\tprint version and exit\n");
    printf("  -b\t\tfork to background\n");
    printf("  -p REGISTRY\tspecify path to registry (may be repeated)\n");
    printf("  -r DRIVER\tsecure driver to load at start-up (may be repeated)\n");
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

    // Defaults:
    // *  don't fork
    bool is_daemon = false;
    // * require authentication

    opterr = 0;
    int opt;
    static struct option long_options[] = {
	    {"help",         no_argument,        0, 'h'},
	    {"backgroung",   no_argument,        0, 'b'},
	    {"driver",       required_argument,  0, 'r'},
	    {"registryPath", required_argument,  0, 'p'},
	    {"version",      no_argument,        0, 'v'},
	    {"P0",           required_argument,  0, 0},
	    {"P1",           required_argument,  0, 0},
    };

    while ((opt = ::getopt_long(argc, args, "bhp:r:v", long_options, NULL)) != -1) {
        bool failed = false;

        switch (opt) {
            case 'h': /* Help */
                printUsage(base_name);
                return 0;
            case 'b': /* Fork to background */
                is_daemon = true;
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
	    case 0:
	        LOG_I("Not support specify the directory of secure partition yet\n");
                break;
            case '?':
                if ((optopt == 'p') || (optopt == 'r')) {
                    fprintf(stderr, "%s: missing argument to `-%c'\n", base_name, optopt);
                } else {
                    fprintf(stderr, "%s: invalid option -- '%c'\n", base_name, optopt);
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
    }
    setSearchPaths(registry_paths);
    LOG_D("Registry search paths:");
    for (auto path = registry_paths.begin(); path != registry_paths.end(); path++) {
        LOG_D("  %s", path->c_str());
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

    // Load SW device driver(s), if requested
    if (!drivers.empty()) {
        LOG_I("Load secure drivers");
    }
    for (auto driver = drivers.begin(); driver != drivers.end(); driver++) {
        // We don't care will loadDriver failed or not...
        secure_world.loadDriver(driver->c_str());
    }

    // Look for tokens and send it to TEE if any
    RegistryServer registry_server(&secure_world);
    registry_server.installEndorsementToken();

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

    // File system
    UpgradeStorage upgrade_storage(secure_world);
    FileSystemWrapper file_system_wrapper(registry_server, registry_paths,
                                          upgrade_storage);
    // Debug
    DebugSession debug_session;
    // Trusted UI
    TuiStarter tui_starter;

    // Add all services in open order
    services.push_back(&secure_world);
    services.push_back(&file_system_wrapper);
    services.push_back(&registry_server);
    services.push_back(&debug_session);
    services.push_back(&tui_starter);

    // The thread needs to be started after the daemon's forking
    // otherwise it causes some problems accessing ioctl's.
    // Also, it needs to be running before any session can be open, so before
    // the calls to open() below.
    LOG_I("Start services");
    auto it = services.begin();
    while (it != services.end()) {
        auto service = *it;
        LOG_I("* start %s", service->displayName());
        if (service->open()) {
            LOG_ERRNO(service->displayName());
            break;
        }
        it++;
    }

    // Keep running until told otherwise
    if (it == services.end()) {
        LOG_I("Daemon running");
        secure_world.wait();
        LOG_I("Daemon stopping");
    }

    // Close all services that were open
    LOG_I("Stop services");
    do {
        it--;
        auto service = *it;
        service->receiveSignal(0);
        LOG_I("* stop %s", service->displayName());
        service->close();
    } while (it != services.begin());

    LOG_I("Daemon exiting");
    return EXIT_SUCCESS;
}
