
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __TEST_CASE_H__
#define __TEST_CASE_H__

#include "DpDataType.h"
#include "DpFRegistry.h"
#include "DpLogger.h"

#if CONFIG_FOR_OS_ANDROID
   #include <String8.h>
   using namespace android;
   #define String      String8
#else
   #include "String"
   using namespace std;
   #define String      string
#endif  // CONFIG_FOR_OS_ANDROID

class TestReporter;

class TestCase
{
public:
    TestCase();

    virtual ~TestCase();

    TestReporter* getReporter() const
    {
        return m_pReporter;
    }

    void setReporter(TestReporter*);

    const char* getName();

    bool runTest(); // return true on success
    
    bool verify(void*,
                uint32_t);  // return true on success;

protected:
    TestReporter *m_pReporter;
    String       m_testName;

    virtual void onGetName(String*) = 0;

    virtual void onRunTest(TestReporter*) = 0;

    virtual bool onVerify(void*,
                          uint32_t) = 0;
};


typedef DpFRegistry<TestCase*, const char*> TestRegistry;


class TestReporter
{
public:
    TestReporter();

    enum Result
    {
        kPassed,  // must begin with 0
        kFailed,

        /////
        kLastResult = kFailed
    };

    virtual ~TestReporter();

	const char* getTestName()
	{
		if(m_pCurTest)
		{
			return m_pCurTest->getName();
        }

		return NULL;
	}

    void resetReport();

    int countTests() const
    {
        return m_testCount;
    }

    int countResults(Result result)
    {
        assert((uint32_t)result <= kLastResult);
        return m_testStatus[result];
    }

    void startTestCase(TestCase*);

    void endTestCase(TestCase*);

    // helpers for tests
    bool assertTrue(bool cond, const char desc[])
    {
        if (!cond)
        {
            this->report(desc, kFailed);
            return false;
        }
   
        return true;
   }

    bool assertFalse(bool cond, const char desc[])
    {
        if (cond)
        {
            this->report(desc, kFailed);
            return false;
        }
    
        return true;
    }

    bool verifyBitTrue(void    *pBuffer,
                       uint32_t size)
    {
        bool status;

        status = m_pCurTest->verify(pBuffer, size);
        if (false == status)
        {
            this->report("bit-true failed", kFailed);
        }

        return true;
    }

    void reportFailed(const char desc[])
    {
        this->report(desc, kFailed);
    }

    void reportFailed(const String &desc)
    {
    #if CONFIG_FOR_OS_ANDROID
        String temp;

        temp = desc;
        this->report(temp.lockBuffer(temp.size()), kFailed);
    #else
        this->report(desc.c_str(), kFailed);
    #endif  // CONFIG_FOR_OS_ANDROID
    }

    bool getCurStatus() const
    {
        return m_curStatus;
    }

protected:
    virtual void onStart(TestCase *pTest)
    {
       printf("%s begin\n", pTest->getName());
    }

    virtual void onReport(const char desc[], Result result)
    {
        printf("%s: %d\n", desc, result);
    }

    virtual void onEnd(TestCase *pTest)
    {
        printf("%s end\n", pTest->getName());
    }

private:
    TestCase *m_pCurTest;
    int32_t  m_testCount;
    int32_t  m_testStatus[kLastResult + 1];
    bool     m_curStatus;


    void report(const char desc[],
                Result     result)
    {
        if (NULL == desc)
        {
            desc = "<no description>";
        }

        this->onReport(desc, result);

        m_testStatus[result] += 1;
        if (kFailed == result)
        {
            m_curStatus = false;
        }
    }
};


#define REPORTER_ASSERT_MESSAGE(pReporter, condition, message)                                                                  \
    do {                                                                                                                        \
        String string;                                                                                                          \
        char   temp[256];                                                                                                       \
                                                                                                                                \
        if (false == pReporter->assertTrue((condition), message))                                                               \
        {                                                                                                                       \
            sprintf(temp, "%s: assert check failed in %s:%d", pReporter->getTestName(), __FILE__, __LINE__);                    \
            string.append(temp);                                                                                                \
            pReporter->reportFailed(string);                                                                                    \
            while(1);                                                                                                           \
        }                                                                                                                       \
    } while(0)

#define REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer, size)                                                                      \
    do {                                                                                                                        \
        String string;                                                                                                          \
        char   temp[256];                                                                                                       \
                                                                                                                                \
        if (false == pReporter->verifyBitTrue((void*)pBuffer, size))                                                            \
        {                                                                                                                       \
            sprintf(temp, "%s: verify bit-true failed in %s:%d", pReporter->getTestName(), __FILE__, __LINE__);                 \
            string.append(temp);                                                                                                \
            pReporter->reportFailed(string);                                                                                    \
            while(1);                                                                                                           \
        }                                                                                                                       \
    } while(0)

#endif  // __TEST_CASE_H__
