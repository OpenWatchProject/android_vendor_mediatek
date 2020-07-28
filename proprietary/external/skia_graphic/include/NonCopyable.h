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

#ifndef _PTHREAD_WRAPPER_NONCOPYABLE_H_
#define _PTHREAD_WRAPPER_NONCOPYABLE_H_
#define USE_CPP0X

/* NonCopyable is the base class for objects that do not want to
 * be copied. It hides its copy-constructor and its assignment-operator.
 */

#ifdef USE_CPP0X
class NonCopyable {
public:
	NonCopyable() = default;

	NonCopyable(NonCopyable&&) = delete;
	NonCopyable& operator= (NonCopyable&&) = delete;

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator= (const NonCopyable&) = delete;
};
#else
class NonCopyable {
public:
	NonCopyable() {}
private:
	NonCopyable(const NonCopyable&);
	NonCopyable& operator= (const NonCopyable&);
};
#endif

#endif