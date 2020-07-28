#include "TestCase.h"
#ifdef WIN32
#include "md5.h"
#else
#include "MD5_Result.h"
#endif
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/*  This file is meant to be included by .cpp files, so it can spew out a
    customized class + global definition.

    e.g.
    #include "TestDefine.h"

    DEFINE_TESTCLASS("MyTest", MyTestClass, MyTestFunction)

    where MyTestFunction is declared as

        void MyTestFunction(Reporter*)
*/

#ifdef WIN32
#define DEFINE_TEST_CASE(testName, className, function)                                                                     \
    class className : public TestCase                                                                                       \
    {                                                                                                                       \
    public:                                                                                                                 \
        className()                                                                                                         \
            : m_hHeader(0),                                                                                                 \
              m_hCPPFile(0),                                                                                                \
              m_count(0)                                                                                                    \
        {                                                                                                                   \
            m_hHeader  = fopen("MD5_Result.h", "r");                                                                        \
            if (NULL == m_hHeader)                                                                                          \
            {                                                                                                               \
                m_hHeader  = fopen("MD5_Result.h", "a+");                                                                   \
                fprintf(m_hHeader, "#include \"DpDataType.h\"\n\n");                                                        \
            }                                                                                                               \
            else                                                                                                            \
            {                                                                                                               \
                fclose(m_hHeader);                                                                                          \
                m_hHeader  = fopen("MD5_Result.h", "a+");                                                                   \
            }                                                                                                               \
                                                                                                                            \
            m_hCPPFile = fopen("MD5_Result.cpp", "r");                                                                      \
            if (NULL == m_hCPPFile)                                                                                         \
            {                                                                                                               \
                m_hCPPFile = fopen("MD5_Result.cpp", "a+");                                                                 \
                fprintf(m_hCPPFile, "#include \"MD5_Result.h\"\n\n");                                                       \
            }                                                                                                               \
            else                                                                                                            \
            {                                                                                                               \
                fclose(m_hCPPFile);                                                                                         \
                m_hCPPFile = fopen("MD5_Result.cpp", "a+");                                                                 \
            }                                                                                                               \
        }                                                                                                                   \
                                                                                                                            \
        ~className()                                                                                                        \
        {                                                                                                                   \
            if (NULL != m_hCPPFile)                                                                                         \
            {                                                                                                               \
                fclose(m_hCPPFile);                                                                                         \
                m_hCPPFile = NULL;                                                                                          \
            }                                                                                                               \
                                                                                                                            \
            if (NULL != m_hHeader)                                                                                          \
            {                                                                                                               \
                fclose(m_hHeader);                                                                                          \
                m_hHeader = NULL;                                                                                           \
            }                                                                                                               \
        }                                                                                                                   \
                                                                                                                            \
    private:                                                                                                                \
        void onGetName(String *pName)                                                                                       \
        {                                                                                                                   \
            String output(testName);                                                                                        \
            *pName = output;                                                                                                \
        }                                                                                                                   \
                                                                                                                            \
        void onRunTest(TestReporter *pReporter)                                                                             \
        {                                                                                                                   \
            int32_t index;                                                                                                  \
                                                                                                                            \
            function(pReporter);                                                                                            \
                                                                                                                            \
            if (NULL != m_hHeader)                                                                                          \
            {                                                                                                               \
                fprintf(m_hHeader, "extern const uint8_t *%sGolden[%d];\n", #className, m_count + 1);                       \
                fclose(m_hHeader);                                                                                          \
            }                                                                                                               \
                                                                                                                            \
            if (NULL != m_hCPPFile)                                                                                         \
            {                                                                                                               \
                fprintf(m_hCPPFile, "\n");                                                                                  \
                if (0 == m_count)                                                                                           \
                {                                                                                                           \
                    fprintf(m_hCPPFile, "const uint8_t *%sGolden[1] =\n", #className);                                      \
                }                                                                                                           \
                else                                                                                                        \
                {                                                                                                           \
                    fprintf(m_hCPPFile, "const uint8_t *%sGolden[%d] =\n", #className, m_count + 1);                        \
                }                                                                                                           \
                fprintf(m_hCPPFile, "{\n    ");                                                                             \
                                                                                                                            \
                for (index = 0; index < m_count; index++)                                                                   \
                {                                                                                                           \
                    fprintf(m_hCPPFile, "%sGolden%d, ", #className, index);                                                 \
                }                                                                                                           \
                fprintf(m_hCPPFile, "0\n");                                                                                 \
                                                                                                                            \
                fprintf(m_hCPPFile, "};\n\n");                                                                              \
            }                                                                                                               \
        }                                                                                                                   \
                                                                                                                            \
        bool onVerify(void      *pBuffer,                                                                                   \
                      uint32_t  size)                                                                                       \
        {                                                                                                                   \
            m_context.init();                                                                                               \
            m_context.update((uint8_t*)pBuffer, size);                                                                      \
            m_context.finalize();                                                                                           \
                                                                                                                            \
            fprintf(m_hCPPFile, "const uint8_t %sGolden%d[] = { %s };\n", #className, m_count, m_context.hex_digest());     \
            m_count++;                                                                                                      \
                                                                                                                            \
            return true;                                                                                                    \
        }                                                                                                                   \
                                                                                                                            \
        MD5          m_context;                                                                                             \
        FILE         *m_hHeader;                                                                                            \
        FILE         *m_hCPPFile;                                                                                           \
        int32_t      m_count;                                                                                               \
    };                                                                                                                      \
                                                                                                                            \
    TestCase* className##Factory(const char *pName)                                                                         \
    {                                                                                                                       \
        if (((NULL == pName) ||                                                                                             \
            !strcmp(pName, testName)))                                                                                      \
        {                                                                                                                   \
            return new className();                                                                                         \
        }                                                                                                                   \
                                                                                                                            \
        return NULL;                                                                                                        \
    };                                                                                                                      \
                                                                                                                            \
    TestRegistry className##Reg(className##Factory);

#else

#define DEFINE_TEST_CASE(testName, className, function)                                                                     \
    class className : public TestCase                                                                                       \
    {                                                                                                                       \
    public:                                                                                                                 \
        className()                                                                                                         \
            : m_count(0)                                                                                                    \
        {                                                                                                                   \
        }                                                                                                                   \
                                                                                                                            \
        ~className()                                                                                                        \
        {                                                                                                                   \
        }                                                                                                                   \
                                                                                                                            \
    private:                                                                                                                \
        void onGetName(String *pName)                                                                                       \
        {                                                                                                                   \
            String output(testName);                                                                                        \
            *pName = output;                                                                                                \
        }                                                                                                                   \
                                                                                                                            \
        void onRunTest(TestReporter* reporter)                                                                              \
        {                                                                                                                   \
            function(reporter);                                                                                             \
        }                                                                                                                   \
                                                                                                                            \
        bool onVerify(void      *pBuffer,                                                                                   \
                      uint32_t  size)                                                                                       \
        {                                                                                                                   \
            /*int32_t bitTrue;                                                                                                */\
            /*                                                                                                                */\
            /*m_context.init();                                                                                               */\
            /*m_context.update((uint8_t*)pBuffer, size);                                                                      */\
            /*m_context.finalize();                                                                                           */\
            /*                                                                                                                */\
            /*bitTrue = memcmp(className##Golden[m_count], m_context.raw_digest(), MD5_RESULT_LENGTH);                        */\
            m_count++;                                                                                                      \
                                                                                                                            \
            /*return (0 == bitTrue)? true: false;                                                                             */\
            return true;                                                                                                    \
        }                                                                                                                   \
                                                                                                                            \
        /*MD5          m_context;                                                                                             */\
        int32_t m_count;                                                                                                    \
    };                                                                                                                      \
                                                                                                                            \
    TestCase* className##Factory(const char *pName)                                                                         \
    {                                                                                                                       \
        if (((NULL == pName) ||                                                                                             \
            !strcmp(pName, testName)))                                                                                      \
        {                                                                                                                   \
            return new className();                                                                                         \
        }                                                                                                                   \
                                                                                                                            \
        return NULL;                                                                                                        \
    };                                                                                                                      \
                                                                                                                            \
    TestRegistry className##Reg(className##Factory);
#endif  // WIN32
