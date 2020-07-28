/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "TestCase.h"

template TestRegistry* TestRegistry::gHead;

TestReporter::TestReporter()
{
    this->resetReport();
}


TestReporter::~TestReporter()
{
}


void TestReporter::resetReport()
{
    m_pCurTest  = NULL;
    m_testCount = 0;
    memset(m_testStatus, 0, sizeof(m_testStatus));
}

void TestReporter::startTestCase(TestCase *pTest)
{
    assert(NULL == m_pCurTest);

    m_pCurTest = pTest;
    this->onStart(pTest);

    m_testCount += 1;
    m_curStatus = true;    // we're optimistic
}


void TestReporter::endTestCase(TestCase *pTest)
{
    assert(pTest == m_pCurTest);

    this->onEnd(pTest);
    m_pCurTest = NULL;
}


///////////////////////////////////////////////////////////////////////////////
TestCase::TestCase()
    : m_pReporter(NULL)
{
}

TestCase::~TestCase()
{
}

void TestCase::setReporter(TestReporter *pReporter)
{
    m_pReporter = pReporter;
}


const char* TestCase::getName()
{
    if (m_testName.size() == 0)
    {
        this->onGetName(&m_testName);
    }

#if CONFIG_FOR_OS_ANDROID
    return m_testName.lockBuffer(m_testName.size());
#else
    return m_testName.c_str();
#endif  // CONFIG_FOR_OS_ANDROID
}

bool TestCase::runTest()
{
    m_pReporter->startTestCase(this);

    this->onRunTest(m_pReporter);

    m_pReporter->endTestCase(this);

    return m_pReporter->getCurStatus();
}


bool TestCase::verify(void     *pBuffer,
                      uint32_t size)
{
    return this->onVerify(pBuffer,
                          size);
}
///////////////////////////////////////////////////////////////////////////////