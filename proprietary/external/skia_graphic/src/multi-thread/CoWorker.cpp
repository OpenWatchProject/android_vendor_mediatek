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
 
#include "CoWorker.h"
extern "C" {
    #include "stdio.h"
    #include "unistd.h" 
}
using namespace std;

#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_mt"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif

#define WORKER_NUM 4

/*NOTE: the init order must not be changed!
 *init by constructing order
 *release by desconstructing order*/
/*be initialized before main*/
CoWorker *CoWorker::sInstance = new CoWorker();
/*auto destruct static obj before exit()*/
CoWorker::AutoGC CoWorker::sAGC;

CoWorker::CoWorker()
	: fFreeThreadQ()
	, fBusyThreadQ()
	, fWaitThreadQ()
	, fThreadNum(WORKER_NUM) {
	pthread_mutex_init(&fMutex, nullptr);
}

//we have to create threads after boot complete in case that zygote blocking.
bool CoWorker::requestWorker(unsigned int num) {
	LOGD("---thread[%lu] request %u workers, coworker can have %u workers\n",
        gettid(), num, fThreadNum);
    if (fThreadNum != num) {
        LOGD("thread[%lu] wrong segment num(%u), cannot use coworker(%u)!\n",
            num, fThreadNum, gettid());
        return false;
    }

    if (fThreadNum != WORKER_NUM) {
        LOGD("---thread[%lu] coworker init thread num(%d) error.\n", fThreadNum, gettid());
        return false;
    }

    /*try lock failed means threads have been created.*/
    if (pthread_mutex_trylock(&fMutex)) {
        LOGD("---workers have existed, thread[%lu] requeset successfully\n", gettid());
        return true;
    } else {
        // trylock success means we need to check if threads exist.
        if (!fFreeThreadQ.empty() || !fBusyThreadQ.empty() || !fWaitThreadQ.empty()) {
            LOGD("---thread[%lu] workers have existed. requeset successfully\n", gettid());
            pthread_mutex_unlock(&fMutex);
            return true;
        }
        pthread_mutex_unlock(&fMutex);
    }

	pthread_mutex_lock(&fMutex);
	for (unsigned int i = 0; i < fThreadNum; i++) {
		APthread *worker = new APthread();
		worker->createPthread();
		fFreeThreadQ.push(worker);
	}
    LOGD("---thread[%lu] requeset successfully\n", gettid());
	pthread_mutex_unlock(&fMutex);
	return true;
}

/*only be called in dispatchJobs. It's thread safe.*/
bool CoWorker::canDispatch() {
	bool flag = false;
	if (fThreadNum != 0 &&
		fThreadNum == fFreeThreadQ.size() &&
		fBusyThreadQ.empty()) {
		flag = true;
	}
	return flag;
}

bool CoWorker::dispatchJobs(queue<Runnable *> *runnableQ) {
	bool flag = false;
	LOGD("---thread[%lu] %s\n",gettid() , __PRETTY_FUNCTION__);
    /*lock successfully when pthread_mutex_trylock return 0.*/
	if (!pthread_mutex_trylock(&fMutex)) {
		if (!canDispatch()) {
			pthread_mutex_unlock(&fMutex);
			return flag;
		}
		for (unsigned int i = 0; i < fThreadNum; i++) {
			APthread *worker = fFreeThreadQ.front();
			worker->setRunnable(runnableQ->front());
			fFreeThreadQ.pop();
			runnableQ->pop();
			fBusyThreadQ.push(worker);
		}
        startAndWaitDone();
		flag = true;
		pthread_mutex_unlock(&fMutex);
	}
	return flag;
}

void CoWorker::startAndWaitDone() {
	LOGD("---thread[%lu] %s\n",gettid() , __PRETTY_FUNCTION__);

	for (unsigned int i = 0; i < fThreadNum; i++) {
		APthread *worker = fBusyThreadQ.front();
		worker->start();
		fBusyThreadQ.pop();
		fWaitThreadQ.push(worker);
	}
	for (unsigned int i = 0; i < fThreadNum; i++) {
		APthread *worker = fWaitThreadQ.front();
		worker->waitUtilRunnableDone();
		fWaitThreadQ.pop();
		fFreeThreadQ.push(worker);
	}
}

unsigned int CoWorker::getWorkerNum() {
    return fThreadNum;
}

/*when destruct CoWorker, all threads should be in free status.*/
CoWorker::~CoWorker() {
	LOGD("---thread[%u] %s: fThreadNum = %d, fFreeThreadQ.size() = %d!---\n",
			gettid(), __PRETTY_FUNCTION__, fThreadNum, fFreeThreadQ.size());
    /*when release coworker, there is 2 status in coworker:
    *1. threads created and do nothing. all threads are in free queue.
    *2. threads are dispatched jobs, and one or many of them are in busy or wait queue.
    *   but destructor cannot get lock. it gets lock until dispatchJobs() works done.
    *so it's ok to just lock here.*/
	pthread_mutex_lock(&fMutex);
#if 0
	if (fBusyThreadQ.size() != 0 || fFreeThreadQ.size() != fThreadNum) {
		LOGD("thread[%lu] %s error: Cannot destruct working CoWorker!\n", gettid(), __PRETTY_FUNCTION__);
		pthread_mutex_unlock(&fMutex);
		return;
	}
	while (!fFreeThreadQ.empty()) {
		APthread *worker = fFreeThreadQ.front();
		fFreeThreadQ.pop();
		delete worker;
	}
#endif
    LOGD("---thread[%u] %s: destruct CoWorker done!---\n", gettid(), __PRETTY_FUNCTION__);
    pthread_mutex_unlock(&fMutex);
}

CoWorker::AutoGC::~AutoGC() {
	LOGD("---%s---\n", __PRETTY_FUNCTION__);
	if (sInstance)
		delete sInstance;
}
