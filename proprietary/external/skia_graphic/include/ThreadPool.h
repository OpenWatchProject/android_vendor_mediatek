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

#ifndef INCLUDE_THREADPOOL_H_
#define INCLUDE_THREADPOOL_H_
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include "JoinThreads.h"
#include "NonCopyable.h"
#include "threadsafe_queue.h"
#include "SkBitmap.h"
#include "SkPaint.h"

namespace MtkSkia {

/*1. singleton. create when skia is loaded into process and auto release 
 *before process exit.
 *2. create thread after boot completed
 *3. use pool when queue is empty*/
class ThreadPool : public NonCopyable {
private:
    int fCurThreadNum;
    int fInitThreadNum;
	/*NOTE:  the order of declaration of the members is important:
	 *both the done flag and the worker_queue must be declared before the threads vector,
	 *which must in turn be declared before the joiner.*/
	std::atomic_bool done;
	threadsafe_queue<std::function<void()>> work_queue;
	std::vector<std::thread> threads;
	JoinThreads joiner;
	static ThreadPool *sInstance;

	class AutoRelease {
	public:
		AutoRelease();
		~AutoRelease();
	};
	static AutoRelease sAutoRelease;

	/*run in each thread of pool*/
	void handleTask();

	ThreadPool();

public:
    void startPool(int initThreadNum);
    bool isPoolStated() {return fInitThreadNum != 0;}
    
	~ThreadPool();
    /*function work after startPool*/
	template<typename FunctionType>
	void submit(FunctionType f) {
		std::function<void()> tmp = std::bind(f);
		work_queue.push(tmp);
	}
	static ThreadPool *getInstance() {return sInstance;}
    /*not thread safe, do not use*/
	bool isThreadPoolFree() {return work_queue.empty();}
};

}



#endif /* INCLUDE_THREADPOOL_H_ */
