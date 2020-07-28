#define DEBUG 1

#ifdef DEBUG
#define TLOGI(fmt...) DBG_LOG("[GZTEST]"fmt)
#else
#define TLOGI(fmt...)
#endif


#define INIT_UNITTESTS	\
	static uint _tests_total  = 0;	\
	static uint _tests_failed = 0;

#define RESET_UNITTESTS \
	do {	\
		_tests_total  = 0;	\
		_tests_failed = 0;	\
	} while (0)



#define REPORT_UNITTESTS \
{ \
	TLOGI("======== UT report =======\n"); \
	TLOGI("total test cases: %u\n", _tests_total); \
	TLOGI("FAILED test cases: %u\n", _tests_failed); \
	if (_tests_failed > 0) { \
		TLOGI("===========> UT FAILED\n"); \
	} else { \
		TLOGI("===========> UT SUCCESSED\n"); \
	} \
}


/*
 *   Begin and end test macro
 */
#define TEST_BEGIN(name) \
	bool _all_ok = true; \
	const char *_test = name; \
	_tests_total++;


#define TEST_END                                                \
{                                                               \
	if (_all_ok)                                            \
		TLOGI("%s: PASSED\n", _test);                   \
	else                                                    \
		TLOGI("%s: FAILED\n", _test);                   \
}

#define EXPECT_EQ(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	if (_e != _a) {                                         \
		TLOGI("FAIL: %s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}


#define EXPECT_EQ_EXIT(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	if (_e != _a) {                                         \
		TLOGI("%s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
		TEST_END                                        \
		return; \
	}                                                       \
}

#define EXPECT_NEQ(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	if (_e == _a) {                                         \
		TLOGI("%s: NOT expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}


#define EXPECT_NEQ_EXIT(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	if (_e == _a) {                                         \
		TLOGI("%s: NOT expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
		TEST_END                                        \
		return; \
	}                                                       \
}

#define EXPECT_LE(expected, actual, msg)				\
{									\
	const typeof(actual) _e = expected;				\
	const typeof(actual) _a = actual;				\
	if (_e > _a) {							\
		TLOGI("%s: NOT expected " #expected 			\
			" (%d), less-than-or-equal-to actual " #actual	\
			" (%d)\n", msg, (int)_e, (int)_a);		\
		_tests_failed++;					\
		_all_ok = false;					\
	}								\
}

