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

#ifndef INCLUDE_JOINTHREADS_H_
#define INCLUDE_JOINTHREADS_H_
#include <vector>
#include <thread>

class JoinThreads {
	std::vector<std::thread>& threads;
	public:
	explicit JoinThreads(std::vector<std::thread>& threads_)
	: threads(threads_) {}
	~JoinThreads() {
		for(unsigned long i=0;i<threads.size();++i) {
			if(threads[i].joinable())
			    threads[i].join();
		}
	}
};



#endif /* INCLUDE_JOINTHREADS_H_ */
