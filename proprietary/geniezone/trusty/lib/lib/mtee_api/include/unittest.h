/*
 * Copyright (C) 2014-2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <trusty_std.h>
#include <tz_private/log.h>


#define UT_LOG(fmt, ...) MTEE_LOG(MTEE_LOG_LVL_PRINTF, "[MTEE UT]" fmt, ## __VA_ARGS__)

#define INIT_UNITTESTS	\
	static uint _tests_total  = 0;	\
	static uint _tests_failed = 0;

#define RESET_UNITTESTS \
	_tests_total  = 0;	\
	_tests_failed = 0;

#define REPORT_UNITTESTS \
{ \
	UT_LOG("======== UT report =======\n"); \
	UT_LOG("total test cases: %u\n", _tests_total); \
	UT_LOG("FAILED test cases: %u\n", _tests_failed); \
	if (_tests_failed > 0) { \
		UT_LOG("===========> UT FAILED\n"); \
	} else { \
		UT_LOG("===========> UT SUCCESSED\n"); \
	} \
}


/*
 *   Begin and end test macro
 */
#define TEST_BEGIN(name)                                        \
	bool _all_ok = true;                                    \
	const char *_test = name;                               \
	UT_LOG("%s:\n", _test);


#define TEST_END                                                \
{                                                               \
	if (_all_ok)                                            \
		UT_LOG("%s: PASSED\n", _test);                   \
	else                                                    \
		UT_LOG("%s: FAILED\n", _test);                   \
}

/*
 * EXPECT_* macros to check test results.
 */
#define EXPECT_EQ(expected, actual, msg)                        \
{                                                               \
	__typeof__(actual) _e = expected;                           \
	__typeof__(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_e != _a) {                                         \
		UT_LOG("FAIL: %s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

#define EXPECT_NEQ(expected, actual, msg)                        \
{                                                               \
	__typeof__(actual) _e = expected;                           \
	__typeof__(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_e == _a) {                                         \
		UT_LOG("FAIL: %s: NOT expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}
#define EXPECT_GT(expected, actual, msg)                        \
{                                                               \
	__typeof__(actual) _e = expected;                           \
	__typeof__(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_e <= _a) {                                         \
		UT_LOG("FAIL: %s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",                \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

#define EXPECT_GE_ZERO(actual, msg)                             \
{                                                               \
	__typeof__(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_a < 0) {                                           \
		UT_LOG("FAIL: %s: expected >= 0 "                     \
		    "actual " #actual " (%d)\n", msg, (int)_a); \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}


#define EXPECT_GT_ZERO(actual, msg)                             \
{                                                               \
	__typeof__(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_a <= 0) {                                          \
		UT_LOG("FAIL: %s: expected > 0 "                      \
		    "actual " #actual " (%d)\n", msg, (int)_a); \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

