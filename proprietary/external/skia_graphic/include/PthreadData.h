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

#ifndef _PTHREAD_WRAPPER_PTHREADDATA_H_
#define _PTHREAD_WRAPPER_PTHREADDATA_H_
extern "C" {
#include "pthread.h"
}
#include "APthread.h"
#include "PthreadEvent.h"

/*save thread attribute and data*/
class PthreadData : public NonCopyable {
public:
	PthreadData();
	void setRunnable(Runnable *r);
	void resetPthreadData();
	~PthreadData();
	/*hold 2 event to judge thread status*/
	PthreadEvent fStarted;
	PthreadEvent fBlockMain;
	pthread_t fThreadId;
	bool fValidPThread;
	pthread_attr_t fAttr;
	Runnable *fRunnable;
private:
	pthread_mutex_t fMutex;
};
#endif
