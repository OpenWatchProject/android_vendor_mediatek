/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

using android::base::ReadFdToString;
using android::base::Trim;
using android::base::WriteStringToFd;

#define BASE_SLEEP_TIME 100000
#define MAX_SLEEP_TIME 60000000

static int state_fd = -1;
static int wake_lock_fd = -1;
static int wake_unlock_fd = -1;
static int wakeup_count_fd = -1;
static pthread_t suspend_thread;
static constexpr char sleep_state[] = "mem";
static int sleep_time = BASE_SLEEP_TIME;
static constexpr char sys_power_state[] = "/sys/power/state";
static constexpr char sys_power_wake_lock[] = "/sys/power/wake_lock";
static constexpr char sys_power_wake_unlock[] = "/sys/power/wake_unlock";
static constexpr char sys_power_wakeup_count[] = "/sys/power/wakeup_count";
static bool autosuspend_is_init = false;
static bool autosuspend_enabled = false;

#ifdef __cplusplus
extern "C" {
#endif

static void update_sleep_time(bool success) {
    if (success) {
        sleep_time = BASE_SLEEP_TIME;
        return;
    }
    // double sleep time after each failure up to one minute
    sleep_time = MIN(sleep_time * 2, MAX_SLEEP_TIME);
}

static void* suspend_thread_func(void* arg __attribute__((unused))) {
    bool success = true;

    while (true) {
        update_sleep_time(success);
        usleep(sleep_time);
        success = false;
        LOG(INFO) << "read wakeup_count";
        lseek(wakeup_count_fd, 0, SEEK_SET);
        std::string wakeup_count;
        if (!ReadFdToString(wakeup_count_fd, &wakeup_count)) {
            LOG(ERROR) << "error reading from " << sys_power_wakeup_count;
            continue;
        }

        wakeup_count = Trim(wakeup_count);
        if (wakeup_count.empty()) {
            LOG(ERROR) << "empty wakeup count";
            continue;
        }

        LOG(INFO) << "write " << wakeup_count << " to wakeup_count";
        if (WriteStringToFd(wakeup_count, wakeup_count_fd)) {
            LOG(VERBOSE) << "write " << sleep_state << " to " << sys_power_state;
            success = WriteStringToFd(sleep_state, state_fd);
        } else {
            LOG(ERROR) << "error writing to " << sys_power_wakeup_count;
        }
    }
    return NULL;
}

static int init_wakelock_fd(void) {
    if (wake_lock_fd >= 0 && wake_unlock_fd >= 0) {
        return 0;
    }

    int fd = TEMP_FAILURE_RETRY(open(sys_power_wake_lock, O_CLOEXEC | O_RDWR));
    if (fd < 0) {
        PLOG(ERROR) << "error opening " << sys_power_wake_lock;
        return -1;
    }

    wake_lock_fd = fd;

    LOG(INFO) << "init_wake_lock_fd success";

    fd = TEMP_FAILURE_RETRY(open(sys_power_wake_unlock, O_CLOEXEC | O_RDWR));
    if (fd < 0) {
        PLOG(ERROR) << "error opening " << sys_power_wake_unlock;
        return -1;
    }

    wake_unlock_fd = fd;

    LOG(INFO) << "init_wake_unlock_fd success";
    return 0;
}

static int acquire_wake_lock(const char* id){
    int ret = init_wakelock_fd();
    if (ret < 0) {
        return ret;
    }

    return WriteStringToFd(id, wake_lock_fd) ? 0 : -1;
}

static int release_wake_lock(const char* id){
    int ret = init_wakelock_fd();
    if (ret < 0) {
        return ret;
    }

    return WriteStringToFd(id, wake_unlock_fd) ? 0 : -1;
}

static int init_state_fd(void) {
    if (state_fd >= 0) {
        return 0;
    }

    int fd = TEMP_FAILURE_RETRY(open(sys_power_state, O_CLOEXEC | O_RDWR));
    if (fd < 0) {
        PLOG(ERROR) << "error opening " << sys_power_state;
        return -1;
    }

    state_fd = fd;

    LOG(INFO) << "init_state_fd success";
    return 0;
}

static int autosuspend_init(void) {
	if (autosuspend_is_init) {
		LOG(ERROR) << "Autosuspend already started.";
		return 0;
	}

	int ret = init_state_fd();
	if (ret < 0) {
		return -1;
	}

	wakeup_count_fd = TEMP_FAILURE_RETRY(open(sys_power_wakeup_count, O_CLOEXEC | O_RDWR));
	if (wakeup_count_fd < 0) {
		PLOG(ERROR) << "error opening " << sys_power_wakeup_count;
		goto err_open_wakeup_count;
	}

	ret = pthread_create(&suspend_thread, NULL, suspend_thread_func, NULL);
	if (ret) {
		PLOG(ERROR) << "error creating thread: " << strerror(ret);
		goto err_pthread_create;
	}

	LOG(INFO) << "autosuspend_init success";
	autosuspend_is_init = true;
	return 0;

err_pthread_create:
	close(wakeup_count_fd);
err_open_wakeup_count:
	return -1;
}

int enableAutoSuspend(void) {
    if (autosuspend_enabled) {
        return 0;
    }

	LOG(INFO) << "autosuspend enable";
	int ret = autosuspend_init();
	if (ret < 0) {
		LOG(ERROR) << "autosuspend_init failed";
		return ret;
	}

	if (!autosuspend_enabled) {
		ret = release_wake_lock("FtmSuspend.SuspendLockout");
		if (ret < 0) {
			LOG(ERROR) << "error release wakelock";
		}
	}

	LOG(INFO) << "autosuspend enable done";

	autosuspend_enabled = true;

	return 0;
}

int disableAutoSuspend(void) {
    if (!autosuspend_enabled) {
        return 0;
    }

    LOG(INFO) << "autosuspend disable";
	int ret = acquire_wake_lock("FtmSuspend.SuspendLockout");

    if (ret < 0) {
        LOG(ERROR) << "error acquire wakelock";
		return ret;
    }

    LOG(INFO) << "autosuspend disable done";

	autosuspend_enabled = false;

	return 0;
}

int force_suspend(int timeout_ms) {
    LOG(INFO) << "force_suspend called with timeout: " << timeout_ms;

    int ret = init_state_fd();
    if (ret < 0) {
        return ret;
    }

    return WriteStringToFd(sleep_state, state_fd) ? 0 : -1;
}

#ifdef __cplusplus
};
#endif

