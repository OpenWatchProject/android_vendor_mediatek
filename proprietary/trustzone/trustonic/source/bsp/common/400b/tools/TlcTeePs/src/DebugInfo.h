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

#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <pthread.h>
#include "mcdebugsession.h"
#include "Utils.h"

class DebugInfo {
    std::string         nwd_proc_name_;
    uint8_t             *debug_info_;
    bool                is_alloc_;
    debugsession_header_t *debug_info_header_;
    tee_identity_t      *debug_info_id_;
    tee_kernel_t        *debug_info_kernel_;
    tee_rtm_t           *debug_info_rtm_;
    size_t              size_;
    int                 debug_session_PID_;
    std::vector<char>*  actions_;
    bool                do_top_;
    bool                last_update_failed_;
    char                last_update_date_[DATE_BUFFER_SIZE];

    bool                stop_;
    pthread_cond_t      cond_;
    pthread_mutex_t     stop_lock_;

    int getDebugSessionPID(std::string proc_name);
    int update();

    bool isTopStopped();
    void topWait();

    // Show functions spreaded in different files for each functional units.
    // In DebugInfo.cpp
    void showInfo();
    void showProgramHeader();
    void showProductId();
    // In DebugInfo_rtm.cpp
    void showRTMInfo();
    void showSessionInfo(uint32_t index);
    // In DebugInfo_kernel.cpp
    void showKernelInfo(bool ordered);
    void showThreadInfo(uint32_t index);
    void showBasicKernelInfo();
    void showGroupedThreadsInfo();
    // In DebugInfo_fc.cpp
    void showFCInfo();
    // In DebugInfo_fs.cpp
    void showLastMcpCommand();
    void showSessions();
    void showStructs();

public:
    DebugInfo();
    ~DebugInfo();
    int exec(bool top_behavior, std::vector<char>* act);
    void top();
    void stopTop();
};

#endif /* DEBUG_INFO_H_ */
