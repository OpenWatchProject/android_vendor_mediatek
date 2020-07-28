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

#include "PthreadData.h"

PthreadData::PthreadData()
: fThreadId()
, fValidPThread(false)
, fRunnable(nullptr)
{
	pthread_attr_init(&fAttr);
	pthread_attr_setdetachstate(&fAttr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&fMutex, nullptr);
}

void PthreadData::setRunnable(Runnable *r) {
	pthread_mutex_lock(&fMutex);
	fRunnable = r;
	pthread_mutex_unlock(&fMutex);
}

void PthreadData::resetPthreadData() {
    pthread_mutex_lock(&fMutex);
    fRunnable = nullptr;
	fValidPThread = false;
	fStarted.reset();
	fBlockMain.reset();
    pthread_mutex_unlock(&fMutex);
}

PthreadData::~PthreadData() {
	pthread_attr_destroy(&fAttr);
	pthread_mutex_destroy(&fMutex);
}