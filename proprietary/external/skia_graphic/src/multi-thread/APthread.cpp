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

#include "APthread.h"
#include "PthreadData.h"

extern "C" {
    #include "stdio.h"
    #include "unistd.h" 
}

//#define __ANDROID_DEBUG__

#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_mt"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif

static void *pthreadCommonEntry(void *args) {
	PthreadData *pthreadData = static_cast<PthreadData *>(args);
	while (true) {
		LOGD("%s: pthread[%lu] wait for start signal...\n", __FUNCTION__, gettid());
		// Wait for start signal
		pthreadData->fStarted.wait();
		if (pthreadData->fRunnable != nullptr) {
			LOGD("%s: pthread[%lu] got start signal and run task...\n", __FUNCTION__, gettid());
			pthreadData->fRunnable->run();
		} else {
			LOGD("%s: pthread[%lu] invalid runnable, exit pthread\n", __FUNCTION__, gettid());
			break;
		}
        /*NOTICE: must reset before set next runnable*/
		pthreadData->fStarted.reset();
		LOGD("%s: pthread[%lu] task finished and trigger main.\n", __FUNCTION__, gettid());
		pthreadData->fBlockMain.trigger();
	}
	LOGD("%s: pthread[%lu] exit.\n", __FUNCTION__, gettid());
	pthreadData->resetPthreadData();
	return nullptr;
}

APthread::APthread() {
	PthreadData *pthreadData = new PthreadData();
	if (!pthreadData)
		LOGD("%s error: new PthreadData failed.\n", __FUNCTION__);
	fData = static_cast<void *>(pthreadData);
	fAPthreadStatus = init_status;
	LOGD("APthread status %s -> after %s.\n", curStatus(fAPthreadStatus), __FUNCTION__);
}

void APthread::createPthread() {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
	if (fAPthreadStatus != init_status) {
		LOGD("APthread status %s -> Cannot %s.\n", curStatus(fAPthreadStatus), __FUNCTION__);
		return;
	}
	int ret = pthread_create(&pthreadData->fThreadId,
							&pthreadData->fAttr,
							pthreadCommonEntry,
							pthreadData);
	pthreadData->fValidPThread = (ret == 0);
	fAPthreadStatus = idle_status;
	LOGD("thread[%x] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
}

void APthread::setRunnable(Runnable *r) {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
	if (fAPthreadStatus != idle_status) {
        /*TODO: should not return directly.*/
		LOGD("thread[%lx] status %s -> Cannot %s.\n",
            pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
        return;
	}

	pthreadData->setRunnable(r);
	fAPthreadStatus = loaded_status;
	LOGD("thread[%x] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
}

bool APthread::start() {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
#if 0
	/*must be a valid pthread before start*/
	if (!pthreadData->fValidPThread) {
		LOGD("%s error: invalid pthread.\n", __FUNCTION__);
		return false;
	}
#endif
    /*caller must make sure apthread is in loaded status.*/
	if (fAPthreadStatus != loaded_status) {
        /*TODO: should not return directly.*/
		LOGD("thread[%x] status %s -> Cannot %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
		return false;
	}

    /*if current status is loaded status, this means apthread has completed last job.
     *But if pthreadData->fStarted is still Triggered, this means apthread didn't reset in time.
     *It's ok to reset.*/
	if (pthreadData->fStarted.isTriggered()) {
		LOGD("%s error: double trigger or resume error.\n", __FUNCTION__);
        pthreadData->fStarted.reset();
	}

	pthreadData->fStarted.trigger();
	fAPthreadStatus = start_status;
	LOGD("thread[%x] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
	return true;
}

void APthread::waitUtilRunnableDone() {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
	if (fAPthreadStatus != start_status) {
        /*TODO: should not return directly.*/
		LOGD("thread[%lx] status %s -> Cannot %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
		return;
	}
	/*block main thread to run until runnable is completed*/
	pthreadData->fBlockMain.wait();
	pthreadData->fBlockMain.reset();
	/*double reset avoid running next job directly without reset. need?*/
	//pthreadData->fStarted.reset();
    fAPthreadStatus = idle_status;
	LOGD("thread[%x] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
}


void APthread::stop() {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
	if (fAPthreadStatus == start_status) {
		waitUtilRunnableDone();
		setRunnable(nullptr);
		start();
		join();
		fAPthreadStatus = stop_status;
		return;
	}
	if (fAPthreadStatus == idle_status || fAPthreadStatus == loaded_status) {
		setRunnable(nullptr);
		start();
		join();
		fAPthreadStatus = stop_status;
	}
	fAPthreadStatus = stop_status;
	LOGD("thread[%x] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
}

void APthread::join() {
	PthreadData *pthreadData = static_cast<PthreadData *>(fData);
	if (!pthreadData->fValidPThread)
		return;
	LOGD("%s: join pthread[%lu]...\n", __FUNCTION__, pthreadData->fThreadId);
	pthread_join(pthreadData->fThreadId, nullptr);
}

APthread::~APthread() {
	if (fData) {
		PthreadData *pthreadData = static_cast<PthreadData *>(fData);
		stop();
		fAPthreadStatus = release_status;
		LOGD("thread[%lx] status %s -> after %s.\n", pthreadData->fThreadId, curStatus(fAPthreadStatus), __FUNCTION__);
		delete pthreadData;
	}
}
