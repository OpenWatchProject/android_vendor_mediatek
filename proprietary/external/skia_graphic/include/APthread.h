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

#ifndef _PTHREAD_WRAPPER_APTHREAD_H_
#define _PTHREAD_WRAPPER_APTHREAD_H_
#include "Runnable.h"

/*define a posix thread*/
class APthread: public NonCopyable {
public:
	enum status {
		init_status,
		idle_status,
		loaded_status,
		start_status,
		suspend_status,//phase out
		stop_status, // thread run over
		release_status
	};
	static const char *curStatus(enum status cs) {
		static const char *status[7] = { "init", "idle", "loaded", "start", "suspend", "stop",
			"release" };
		switch (cs) {
		case init_status: return status[0];
		case idle_status: return status[1];
		case loaded_status: return status[2];
		case start_status: return status[3];
		case suspend_status: return status[4];
		case stop_status: return status[5];
		case release_status: return status[6];
		}
		return "invalid status";
	}

	APthread();
	void createPthread();
	void setRunnable(Runnable *r);
	void waitUtilRunnableDone();
	/*stop pthread before destructor be invoked.*/
	void stop();

	/*Non-virtual, no subclass.*/
	~APthread();
	/*Starts the thread. Returns false if the thread could not be started.*/
	bool start();
	/*Waits for the thread to finish.
     * If the thread has not started, returns immediately.*/
	void join();
private:
	void *fData;
	enum status fAPthreadStatus;
};
#endif
