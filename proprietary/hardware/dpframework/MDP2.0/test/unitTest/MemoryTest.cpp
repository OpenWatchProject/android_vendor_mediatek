#include "TestDefine.h"
#include "DpMemoryProxy.h"
#include "DpRandGen.h"

void memoryTestCase0(TestReporter *pReporter)
{
    DpMemoryProxy *pMemory;
    uint8_t       *pGolden;
    uint8_t       *pBuffer;
    int32_t       status;

    return;

    DPLOGI("Start memoryTestCase0\n");

    pGolden = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pGolden), "Allocate golden buffer failed\n");

    pMemory = new DpMemoryProxy(-1, 640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pMemory), "Allocate memory proxy failed\n");

    pBuffer = (uint8_t*)pMemory->mapVA();
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pBuffer), "Map virtual memory failed\n");

    DPLOGI("Mapped virtual memory; %p\n", pBuffer);

    utilRNGGenRand(pGolden,
                   640 * 480 * 2);

    memcpy(pBuffer, pGolden, 640 * 480 * 2);

    status = memcmp(pBuffer, pGolden, 640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (0 == status), "Write memory buffer failed\n");

    delete pMemory;
}

DEFINE_TEST_CASE("MemoryTest0", MemoryTest0, memoryTestCase0)


void memoryTestCase1(TestReporter *pReporter)
{
    DpMemoryProxy *pMemory;
    uint8_t       *pGolden;
    uint8_t       *pBuffer;
    int32_t       status;

    return;

    DPLOGI("Start memoryTestCase1\n");

    pGolden = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pGolden), "Allocate golden buffer failed\n");

    pMemory = new DpMemoryProxy((void*)NULL, 640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pMemory), "Allocate memory proxy failed\n");

    pBuffer = (uint8_t*)pMemory->mapVA();
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pBuffer), "Map virtual memory failed\n");

    DPLOGI("Mapped virtual memory; %p\n", pBuffer);

    utilRNGGenRand(pGolden,
                   640 * 480 * 2);

    memcpy(pBuffer, pGolden, 640 * 480 * 2);

    status = memcmp(pBuffer, pGolden, 640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (0 == status), "Write memory buffer failed\n");

    delete pMemory;
}

DEFINE_TEST_CASE("MemoryTest1", MemoryTest1, memoryTestCase1)
