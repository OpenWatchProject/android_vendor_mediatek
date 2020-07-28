/*
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created on: 2017-11-30
 * Author: Christ Sun
 */

#include "ThreadPool.h"
extern "C" {
#include "stdio.h"
}

#define __ANDROID_DEBUG__
#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_draw_async"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif


MtkSkia::ThreadPool *MtkSkia::ThreadPool::sInstance = new ThreadPool();
MtkSkia::ThreadPool::AutoRelease MtkSkia::ThreadPool::sAutoRelease;


void MtkSkia::ThreadPool::handleTask() {

	while (!done) {
		std::function<void()> task;
		if (work_queue.try_pop(task)) {
			task();
		} else {
			std::this_thread::yield();
		}
	}
}

MtkSkia::ThreadPool::ThreadPool()
	: fCurThreadNum(0), fInitThreadNum(0), done(false), joiner(threads) {
	LOGD("consturct thread pool\n");
}

void MtkSkia::ThreadPool::startPool(int initThreadNum) {
    if (fInitThreadNum != 0) {
        LOGD("pool has been started with initThreadNum(%d)\n", initThreadNum);
        return;
    }
    
    LOGD("startPool with initThreadNum(%d)\n", initThreadNum);
	try {
        fInitThreadNum = initThreadNum;
        fCurThreadNum = fInitThreadNum;
    	for (int i = 0; i < fInitThreadNum; i++) {
    		threads.push_back(std::thread(&ThreadPool::handleTask, this));
    	}

	} catch(...) {
	    LOGD("startPool error.\n");
		done = true;
		throw;
	}
}


MtkSkia::ThreadPool::~ThreadPool() {
	done = true;
	LOGD("desturct thread pool\n");
}

MtkSkia::ThreadPool::AutoRelease::AutoRelease() {LOGD("construct AutoRelease\n");}

MtkSkia::ThreadPool::AutoRelease::~AutoRelease() {
	LOGD("AutoRelease\n");
	if (sInstance)
		delete sInstance;
}
