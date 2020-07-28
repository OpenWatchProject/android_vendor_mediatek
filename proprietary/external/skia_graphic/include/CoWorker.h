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

#ifndef SRC_COWORKER_H_
#define SRC_COWORKER_H_
#include "APthread.h"
#include <queue>
extern "C" {
#include "pthread.h"
#include "stdio.h"
}

/*singleton*/
class CoWorker: public NonCopyable {
public:
	static CoWorker* getInstance() {return sInstance;}
	bool requestWorker(unsigned int);
	bool dispatchJobs(std::queue<Runnable *> *runnableQ);
    unsigned int getWorkerNum();
	~CoWorker();
private:
	CoWorker();
	bool canDispatch();
    void startAndWaitDone();

	std::queue<APthread *> fFreeThreadQ;
	std::queue<APthread *> fBusyThreadQ;
    std::queue<APthread *> fWaitThreadQ;  
	const unsigned int fThreadNum; //cannot change
	pthread_mutex_t fMutex;
	static CoWorker *sInstance;
	/*auto delete singleton avoid memory leakage*/
	class AutoGC {
	public:
		~AutoGC();
	};

	static AutoGC sAGC;
};


#endif /* SRC_COWORKER_H_ */
