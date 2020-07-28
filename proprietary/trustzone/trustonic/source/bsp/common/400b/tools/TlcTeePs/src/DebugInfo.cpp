/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>
#include <stdio.h>     // fopen
#include <fnmatch.h>
#include <dirent.h>
#include <signal.h>
#include <libgen.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "buildTag.h"
#include "mcLoadFormat.h"
#include "log.h"
#include "mcVersionHelper.h"

#include "Utils.h"
#include "DebugInfo.h"

#define TOP_TIME_STAMP 1
#define NWD_PROC_NAME  "mcDriverDaemon"

static DebugInfo* gDebugInfo;
static void alarmHandler(int signum) {
#ifdef NDEBUG
    (void)signum;
#else
    LOG_D("Signal received : %d in %s", signum, __func__);
#endif
    gDebugInfo->top();
}

static void terminateHandler(int signum) {
#ifdef NDEBUG
    (void)signum;
#else
    LOG_D("Signal received : %d in %s", signum, __func__);
#endif
    gDebugInfo->stopTop();
}

DebugInfo::DebugInfo():
        nwd_proc_name_(NWD_PROC_NAME),
        debug_info_(NULL),
        is_alloc_(false),
        debug_info_header_(NULL),
        debug_info_id_(NULL),
        debug_info_kernel_(NULL),
        debug_info_rtm_(NULL),
        size_(0),
        debug_session_PID_(0),
        actions_(NULL),
        do_top_(false),
        last_update_failed_(false),
        stop_(false),
        cond_(PTHREAD_COND_INITIALIZER) {

    // Set up global Pointer
    gDebugInfo = this;

    // FIX compilation warning
    last_update_date_[0] = '\0';

    // Set up alarm
    struct sigaction actionAlarm;
    ::memset(&actionAlarm, 0, sizeof(actionAlarm));
    actionAlarm.sa_handler = alarmHandler;
    ::sigemptyset(&actionAlarm.sa_mask);
    actionAlarm.sa_flags = SA_RESTART;  // Restart interrupted system calls
    ::sigaction(SIGALRM, &actionAlarm, NULL);

    // Set up ctrl C
    struct sigaction actionTerminate;
    ::memset(&actionTerminate, 0, sizeof(actionTerminate));
    actionTerminate.sa_handler = terminateHandler;
    ::sigemptyset(&actionAlarm.sa_mask);
    actionTerminate.sa_flags = SA_RESTART;  // Restart interrupted system calls
    ::sigaction(SIGINT,  &actionTerminate, NULL);
    ::sigaction(SIGTERM, &actionTerminate, NULL);
    ::pthread_mutex_init(&stop_lock_, NULL);
}

DebugInfo::~DebugInfo(){
    if (is_alloc_) {
        LOG_D("Free program memory");
        ::free(debug_info_);
    }
}

int DebugInfo::exec(bool top_behavior, std::vector<char>* act){
    // update even if only degugfs display is activated to get the TEE Product Id
    if (update()) {
        IP_LOG_E("Can't update debug info");
        return -1;
    }
    actions_ = act;
    do_top_ = top_behavior;
    if (!do_top_) {
        showInfo();
    } else {
        top();
        // Break with ctrl_C
        topWait();
    }
    return 0;
}

static int isPID(const struct dirent* entry) {
    return !fnmatch("[1-9]*", entry->d_name, 0);
}

int DebugInfo::getDebugSessionPID(std::string proc_name){
    struct dirent** dirents;
    int dirents_length = ::scandir("/proc", &dirents, isPID, alphasort);
    if (dirents_length < 0) {
        LOG_ERRNO("scandir");
        return 1;
    }

    // dirents contains all the active PID files
    int ret = -1;
    for (int i = 0; i < dirents_length; i++) {
        char status_path[64];
        int size = ::snprintf(status_path, sizeof(status_path), "/proc/%s/status", dirents[i]->d_name);
        if (size < 0 || size >= static_cast<int>(sizeof(status_path))) {
            IP_LOG_E("Can't set status file path");
            return -1;
        }

        FILE *status = ::fopen(status_path,"r");
        if(status != NULL){
            // status file exists for the current PID "string_pid"

            // /!\ apparently can't use ifstream because no size exists for procfs files
            // The program name is located in the first line
            char line[512]; // Should be enough
            char *fgets_ret = ::fgets(line, 512, status);

            if (::fclose(status)) {
                LOG_ERRNO("close");
                ret = -1;
                break;
            }

            if (fgets_ret != NULL) {
                if (std::string(line).find(proc_name) != std::string::npos) {
                    LOG_D("%s PID found : %s", proc_name.c_str(), dirents[i]->d_name);
                    ret = ::atoi(dirents[i]->d_name);
                    break;
                }
            }
        }
        // Continue to the next PID
    }

    // Free memory
    for (int i = 0; i < dirents_length; i++) {
        ::free(dirents[i]);
    }
    ::free(dirents);

    return ret;
}

int DebugInfo::update() {

    debug_session_PID_ = getDebugSessionPID(nwd_proc_name_);
    if(debug_session_PID_ < 1){
        IP_LOG_E("Can't get DebugSession PID (Is %s running ?)", nwd_proc_name_.c_str());

        return -1;
    }

    // wait for file notification (initialization)
    auto inotifyInstance = ::inotify_init();
    if (inotifyInstance <= 0) {
        LOG_ERRNO("notify instance");
    }
    int watchDescriptor;
    if (::access(DEBUG_SESSION_DUMP_FILE, F_OK)) {
        // File does not exit, wait for a creation in the directory
        //char *dirc = strdup(DEBUG_SESSION_DUMP_FILE);//TODO: handle dirc==NULL
        watchDescriptor = ::inotify_add_watch(inotifyInstance,
                                ::dirname(const_cast<char *>(DEBUG_SESSION_DUMP_FILE)),
                                IN_CREATE);
        //free(dirc);
    } else {
        // File exits, wait for a close
        watchDescriptor = ::inotify_add_watch(inotifyInstance, DEBUG_SESSION_DUMP_FILE, IN_CLOSE);
    }
    if (watchDescriptor <= 0) {
        LOG_ERRNO("watch descriptor");
    }
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(inotifyInstance, &rfds);
    timeval timeout;
    timeout.tv_sec  = TOP_TIME_STAMP;
    timeout.tv_usec = 0;

    // Trigger the info page dumping
    if(::kill(debug_session_PID_, SIGUSR2)) {
        IP_LOG_E("Can't send signal to PID(%d)", debug_session_PID_);
        return -1;
    }

    // wait timeout
    LOG_D("Waiting for TEE to update the info page");
    int ret = ::select(FD_SETSIZE, &rfds, NULL, NULL, &timeout);
    if (ret < 0) {
        LOG_ERRNO("select");
        return -1;
    } else if (ret == 0) {
        LOG_D("Timeout during the debugsession update");
        return -1;
    } // else { continue }
    ::inotify_rm_watch(inotifyInstance, watchDescriptor);
    ::close(inotifyInstance);

    // Copy the new debug info in the local buffer
    // Open DebugSession file
    FILE *fd = ::fopen(DEBUG_SESSION_DUMP_FILE, "r");
    if (fd == NULL) {
        LOG_ERRNO("open");
        return -1;
    }

    // The file size is the size of the info page buffer
    ::fseek(fd, 0L, SEEK_END);
    size_t file_size = ::ftell(fd);
    ::fseek(fd, 0L, SEEK_SET);    // Get back to begining

    if (file_size != size_) {
        size_ = file_size;
        if (is_alloc_) {
            // In case size changes between two calls of update
            ::free(debug_info_);
        } // else { first allocation }
        debug_info_ = static_cast<uint8_t*>(::malloc(size_));
        is_alloc_ = true;
        if (debug_info_ == NULL) {
            IP_LOG_E("Can't allocate memory");
            goto fd_DEBUGSESSION;
        }
    }
    LOG_D("Debug info buffer size = %zu", size_);

    {
        // Read DebugSession file
        size_t read = ::fread(debug_info_, 1, size_, fd);
        if(read != size_) {
            LOG_ERRNO("reading info page file");
            goto fd_DEBUGSESSION;
        }
    }
    // Close DebugSession file
    if(::fclose(fd)){
        LOG_ERRNO("close");
        return -1;
    }

    // The debugsession has successfully been stored in buffer
    // Update the pointers
    debug_info_header_ = (debugsession_header_t *)debug_info_;
    debug_info_id_ = (tee_identity_t *)(debug_info_ + debug_info_header_->headers[0].offset);
    debug_info_kernel_ = (tee_kernel_t *)(debug_info_ + debug_info_header_->headers[1].offset);
    debug_info_rtm_ = (tee_rtm_t *)(debug_info_ + debug_info_header_->headers[2].offset);

    // Check debug session version
    if (debug_info_header_->version != MC_MAKE_VERSION(DEBUGSESSION_VERSION_MAJOR, DEBUGSESSION_VERSION_MINOR)) {
        LOG_ERRNO("wrong version");
        return -1;
    }

    getCurrentDateTime(last_update_date_);
    return 0;

fd_DEBUGSESSION:
    if(::fclose(fd)){
        LOG_ERRNO("close");
    }
    return -1;
}

void DebugInfo::top() {
    clear_terminal();

    // Display the N-1 sample while the Nth is updated
    showInfo();

    // Set the alarm first for the next top execution
    if (!isTopStopped()) {
        ::alarm(TOP_TIME_STAMP);
    }

    // Update during the delay time
    if (update()){
        if (size_ == 0) {
            // Can't update on the first attempt: exit
            IP_LOG_E("Can't update info page");
            ::exit(1);
        } else {
            // Can't update anymore: keep last data displayed
            LOG_W("Can't update the debugsession anymore");
            last_update_failed_ = true;
        }
    } else {
        last_update_failed_ = false;
    }
}

void DebugInfo::stopTop() {
    ::pthread_mutex_lock(&stop_lock_);
    stop_ = true;
    ::pthread_cond_broadcast(&cond_);
    ::pthread_mutex_unlock(&stop_lock_);
}

bool DebugInfo::isTopStopped() {
    bool ret;
    ::pthread_mutex_lock(&stop_lock_);
    ret = stop_;
    ::pthread_mutex_unlock(&stop_lock_);
    return ret;
}

void DebugInfo::topWait() {
    LOG_D("Top (wait)");
    ::pthread_mutex_lock(&stop_lock_);
    while (!stop_) {
        pthread_cond_wait(&cond_, &stop_lock_);
    }
    ::pthread_mutex_unlock(&stop_lock_);
    LOG_D("Top (over)");
}

void DebugInfo::showInfo(){

    showProgramHeader();
    showProductId();

    if (actions_->size() == 0) {
        // Show default information => RTM
        showRTMInfo();
    } else {
        // Execute actions according to the ones stored
        for (auto c = actions_->begin(); c != actions_->end(); c++) {
            switch (*c) {
                case 'a':
                case 'A':
                    showRTMInfo();
                    showKernelInfo(true);
                    showFCInfo();
                    showLastMcpCommand();
                    showStructs();
                    showSessions();
                    break;
                case 'k':
                case 'K':
                    showKernelInfo(false);
                    break;
                case 'f':
                case 'F':
                    showFCInfo();
                    break;
                case 'm':
                case 'M':
                    showLastMcpCommand();
                    break;
                case 'r':
                case 'R':
                    showRTMInfo();
                    break;
                case 's':
                case 'S':
                    showStructs();
                    showSessions();
                    break;
                case 'o':
                case 'O':
                    showKernelInfo(true);
                    break;
                case 'g':
                case 'G':
                    showGroupedThreadsInfo();
                    break;
            }
            // TODO
            // for now only display the first argument (output string format not adapted)
            // for long displays. To be removed as soon as the ouput format (a "linux top")
            // do_top field can be removed too
            // like one has been implemented, Of course don't lauch "tee-ps -ta" ...
            if (do_top_) {
                printf("break");
                break;
            }
        }
    }
}

void DebugInfo::showProgramHeader() {
    char* date = static_cast<char*>(::malloc(DATE_BUFFER_SIZE));
    if (date != NULL) {
        getCurrentDateTime(date);
    }
    printf(HIGHLIGHT("Local time : %s     mcDriverDaemon PID : %d     Size of debug info : %zu") "\n", date, debug_session_PID_, size_);
    if (last_update_failed_) {
        printf(HIGHLIGHT_ERROR("ERROR : can't update debug data, last successful update (%s)") "\n", last_update_date_);
    }
    printf("\n");

    ::free(date);
}

void DebugInfo::showProductId() {
    printf(BOLD("TEE Product Id: ") "%s", debug_info_id_->productid);
}
