/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#include <stdlib.h>
#include <string.h>     // memset
#include <stdio.h>      // fopen
#include <libgen.h>     // basename
#include <vector>
#include <ctype.h>
#include <algorithm>

#include "buildTag.h"
#include "log.h"
#include "DebugInfo.h"

static int printUsage(char *path) {
    fprintf(stderr, "\nusage: %s [-hakfmorst]\n", ::basename(path));
    fprintf(stderr, "Copyright (c) Trustonic Limited 2015.\n");
    fprintf(stderr, MOBICORE_COMPONENT_BUILD_TAG "\n");
    fprintf(stderr, "-a/A\t\tALL (product ID, RTM information, kernel information and FastCall buffer)\n");
    fprintf(stderr, "-f/F\t\tFASTCALL (FastCall buffer)\n");
    fprintf(stderr, "-k/K\t\tKERNEL (kernel information)\n");
    fprintf(stderr, "-m/M\t\tdggFs last mcp commmands\n");
    fprintf(stderr, "-o/O\t\tKERNEL ORDERED (kernel information with threads ordered by last modification)\n");
    fprintf(stderr, "-g/G\t\tKERNEL GROUPED (kernel information with threads grouped by session)\n");
    fprintf(stderr, "-r/R\t\tRTM (RTM information)\n");
    fprintf(stderr, "-s/S\t\tdbgFs structs and sessions\n");


    fprintf(stderr, "-t/T\t\tTOP (executes a 'Linux top' like action on the corresponding command, only the first argument is taken in account)\n");
    fprintf(stderr, "\t\t\t\t -> eg : ./tee-ps -tr  (RTM info dynamically)\n");
    fprintf(stderr, "\t\t\t\t -> eg : ./tee-ps [-tfr | -frt] (only fastcall info dynamically)\n");

    fprintf(stderr, "-no option\tProduct ID and RTM information\n");
    return 2;
}

int main(int argc, char *args[]) {
    // Read the Command line options
    int c, errFlag = 0;
    bool do_top = false;
    // Get the actions
    std::vector<char> actions;

    while ((c = ::getopt(argc, args, "haAkKfFmMoOgGrRtsST")) != -1) {
        switch (::tolower(c)) {
            case 'h': // Help
                errFlag++;
                break;
            case 't':
                do_top = true;
                break;
            case 'a': // Actions
            case 'f':
            case 'k':
            case 'm':
            case 's':
            case 'o':
            case 'r':
            case 'g':
                // Do not duplicate actions
                if (std::find(actions.begin(), actions.end(), c) == actions.end()) {
                    actions.push_back(static_cast<char>(c));
                }
                break;
            case '?' :
                errFlag++;
        }
    }
    if (errFlag)
        return printUsage(args[0]);
    // Here the command is valid and the actions are all stored

    // Setting up the debug info
    DebugInfo debug_info;
    if (!debug_info.exec(do_top, &actions)) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
