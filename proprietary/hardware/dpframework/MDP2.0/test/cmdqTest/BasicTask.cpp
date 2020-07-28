#include "TestDefine.h"
#include "DpBlitStream.h"
#include "DpReadBMP.h"
#include "DpDriver.h"
#include "DpEngineType.h"

#include <pthread.h>
#include <fcntl.h>
#include <linux/ion_drv.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "cmdq_driver.h"

#ifdef WIN32
#include"windows.h"
#define sleep(n) Sleep(n)
#endif


class DpDummyBlock : public DpCommandBlock
{
public:
	uint32_t m_dummyBuffer[6];
    uint32_t *m_readbackRegs;
	uint32_t m_numRead;

	DpDummyBlock():
        m_readbackRegs(NULL)
		,m_numRead(0)
	{
		DPLOGE("DpDummyBlock::DpDummyBlock()\n");
        buildCommandWriteTest();
	}

    void buildCommandWriteTest()
    {
		m_dummyBuffer[0] = 0x00010005;  // wait value 0x00010005
		m_dummyBuffer[1] = 0x04010894;  // wait mmsys_config_dummy(14000894)
		m_dummyBuffer[2] = 0x00000001;
		m_dummyBuffer[3] = 0x40000000;
		m_dummyBuffer[4] = 0x00000008;
		m_dummyBuffer[5] = 0x10000000;
        DPLOGE("m_dummyBuffer[1]::0x%08x\n", m_dummyBuffer[1]);
    }

    void buildCommandWaitNoClearTest()
    {
		m_dummyBuffer[0] = 0x00008001;  // wait value 1, and no clear
		m_dummyBuffer[1] = 0x2000012c;  // wait event 300
		m_dummyBuffer[2] = 0x00000001;
		m_dummyBuffer[3] = 0x40000000;
		m_dummyBuffer[4] = 0x00000008;
		m_dummyBuffer[5] = 0x10000000;
    }

    void buildInvalidSecurityCommandTest()
    {
		m_dummyBuffer[0] = 0x00010005;  // wait value 0x00010005
		m_dummyBuffer[1] = 0x04100894;  // subsys 16 + 0x894
		m_dummyBuffer[2] = 0x00000001;
		m_dummyBuffer[3] = 0x40000000;
		m_dummyBuffer[4] = 0x00000008;
		m_dummyBuffer[5] = 0x10000000;
        DPLOGE("m_dummyBuffer[1]::0x%08x\n", m_dummyBuffer[1]);
    }

    void buildCommandWithSecureHandle()
    {
		m_dummyBuffer[0] = 0xBFC00000;  // handle: 0xBFC00000
		m_dummyBuffer[1] = 0x04014F00;  // MDP WDMA dst addr 0x14004F00
		m_dummyBuffer[2] = 0x00000001;
		m_dummyBuffer[3] = 0x40000000;
		m_dummyBuffer[4] = 0x00000008;
		m_dummyBuffer[5] = 0x10000000;
        DPLOGE("m_dummyBuffer[1]::0x%08x\n", m_dummyBuffer[1]);
    }

    virtual uint32_t getScenario()
	{
		DPLOGE("DpDummyBlock::getScenario\n");
		return 15;
	}

    virtual uint32_t getPriority()
	{
		DPLOGE("DpDummyBlock::getPriority\n");
		return 0;
	}

    virtual uint32_t getEngineFlag()
	{
		DPLOGE("%s\n", __FUNCTION__);
		// use RDMA0 for sample
		return 0x10;
	}

    virtual uint32_t getCurMode()
	{
		DPLOGE("%s\n", __FUNCTION__);
		return 0;
	}

    virtual void* getFrameBaseSW()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return m_dummyBuffer;
	}

    virtual uint32_t getFrameSize()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return sizeof(m_dummyBuffer);
	}

    virtual void* getTileBaseSW()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return NULL;
	}

    virtual uint32_t getTileSize()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return 0;
	}

	virtual void setDumpOrder(uint32_t dumpOrder)
	{
        DPLOGE("%s\n", __FUNCTION__);
		return;
	}

    virtual uint32_t getDumpOrder()
	{
	    DPLOGE("%s\n", __FUNCTION__);
		return 0;
	}

    virtual void getMaxXY(uint32_t& x,uint32_t& y){}

    virtual uint32_t* getReadbackRegsPointer()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return m_readbackRegs;
	}

    virtual uint32_t* getNumReadbackRegsPointer()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return &m_numRead;
	}

    virtual uint32_t getISPDebugDumpRegs()
	{
        DPLOGE("%s\n", __FUNCTION__);
		return 0;
	}

    virtual void setPathOwner(void* pCurrentPathOwner)
	{
	}

#if CONFIG_FOR_SEC_VIDEO_PATH
    virtual void* getSecureAddrMD() 
    {
        return NULL;
    };

    virtual uint32_t getSecureAddrCount()
    {
        return 0;
    };
#endif    
};

void cmdqDriverTestQueryUsage(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
	DpDriver *pDrv;
    uint32_t engUsages[MAX_NUM_READBACK_REGS];
    char *module_name;

	DPLOGE("=== %s begin === \n", __FUNCTION__);

    pDrv = DpDriver::getInstance();
    status = pDrv->queryEngUsages(engUsages);

    for(int i =0; i < MAX_NUM_READBACK_REGS; i ++)
    {
        DP_GET_ENGINE_NAME(i, module_name);
        DPLOGE("MDP modules( %s) usage: %d\n", module_name, engUsages[i]);
    }

	DpDriver::destroyInstance();

	DPLOGE("=== %s end ===\n", __FUNCTION__);
}

void cmdqDriverTestSubmitSecure(TestReporter *pReporter)
{
    int driverID = 0;
    int status;

    cmdqCommandStruct param;
    DpDummyBlock block;
    //block.buildInvalidSecurityCommandTest();

    DPLOGE("=== %s begin === \n", __FUNCTION__);

    driverID = open("/dev/mtk_cmdq", O_RDONLY, 0);
    do
    {
        memset(&param, 0, sizeof(param));
        param.scenario     = block.getScenario();
        param.priority     = block.getPriority();
        param.engineFlag   = block.getEngineFlag();
        param.pVABase      = (unsigned long)block.getFrameBaseSW();
        param.blockSize    = block.getFrameSize() +
                             block.getTileSize();
        
        DPLOGE("blockSize:%d(%d:%d)\n", param.blockSize, block.getFrameSize(), block.getTileSize());

        param.debugRegDump = block.getISPDebugDumpRegs();

        param.regRequest.regAddresses = (unsigned long)block.getReadbackRegsPointer();
        param.regRequest.count = *(block.getNumReadbackRegsPointer());
        param.regValue.count = param.regRequest.count;
        param.regValue.regValues = param.regRequest.regAddresses;

		#ifdef CMDQ_SECURE_SUPPORT
        param.secData.isSecure = 1;
        param.secData.addrMetadataCount = 0;
        param.secData.addrMetadatas= NULL;
        param.secData.enginesNeedDAPC = (0x1 << CMDQ_ENG_MDP_WDMA); //0x0;
        param.secData.enginesNeedPortSecurity = (0x1 << CMDQ_ENG_MDP_WDMA); //0x0;
        #endif

        status = ioctl(driverID, CMDQ_IOCTL_EXEC_COMMAND, &param);
        if (0 != status)
        {
            DPLOGE("allocate PA Buffer failed(%d)\n", status);
            break;
        }

    }while(0);

    if(0 != driverID)
    {
        close(driverID);
    }

    DPLOGE("=== %s end ===\n", __FUNCTION__);

}

void cmdqDriverTestSubmitSecureWithAddrMetadata(TestReporter *pReporter)
{
#ifdef CMDQ_SECURE_SUPPORT
    int driverID = 0;
    int status;

    cmdqCommandStruct param;
    DpDummyBlock block;
    block.buildCommandWithSecureHandle();

    cmdqSecAddrMetadataStruct addrs;
    addrs.baseHandle = 0xBFC00000;
    addrs.instrIndex = 0;
    addrs.type = CMDQ_SAM_H_2_MVA;
    addrs.port = 14; // M4U port
    addrs.offset = 0; 
    addrs.size = 100;
    
   
    DPLOGE("=== %s begin === \n", __FUNCTION__);

    driverID = open("/dev/mtk_cmdq", O_RDONLY, 0);
    do
    {
        DPLOGE("============= handle translation with DAPC flag =============\n");    
        memset(&param, 0, sizeof(param));
        param.scenario     = block.getScenario();
        param.priority     = block.getPriority();
        param.engineFlag   = block.getEngineFlag();
        param.pVABase      = (uint32_t*)block.getFrameBaseSW();
        param.blockSize    = block.getFrameSize() +
                             block.getTileSize();
 
        param.debugRegDump = block.getISPDebugDumpRegs();

        param.regRequest.regAddresses = block.getReadbackRegsPointer();
        param.regRequest.count = *(block.getNumReadbackRegsPointer());
        param.regValue.count = param.regRequest.count;
        param.regValue.regValues = param.regRequest.regAddresses;

        param.secData.isSecure = 1;
        param.secData.addrMetadataCount = 1;
        param.secData.addrMetadatas= (void*)(&addrs);
        DPLOGE("secData.addrMetadatas(%x), count:%d\n", param.secData.addrMetadatas, param.secData.addrMetadataCount);
        param.secData.enginesNeedDAPC = (0x1 << CMDQ_ENG_MDP_WDMA);
        param.secData.enginesNeedPortSecurity = (0x1 << CMDQ_ENG_MDP_WDMA);

        status = ioctl(driverID, CMDQ_IOCTL_EXEC_COMMAND, &param);
        if (0 != status)
        {
            DPLOGE("allocate PA Buffer failed(%d)\n", status);
            break;
        }

        DPLOGE("============= failed case: command  =============\n");            
        param.secData.enginesNeedDAPC = (0x1 << CMDQ_ENG_MDP_RDMA0);
        param.secData.enginesNeedPortSecurity = (0x1 << CMDQ_ENG_MDP_RDMA0);
        status = ioctl(driverID, CMDQ_IOCTL_EXEC_COMMAND, &param);
        if (0 != status)
        {
            DPLOGE("allocate PA Buffer failed(%d)\n", status);
            break;
        }

    }while(0);

    if(0 != driverID)
    {
        close(driverID);
    }

    DPLOGE("=== %s end ===\n", __FUNCTION__);
#endif

}

void cmdqDriverTestSubmit(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
	DpDriver *pDrv;
	DpDummyBlock block;

	DPLOGE("=== %s begin === \n", __FUNCTION__);

    pDrv = DpDriver::getInstance();
	pDrv->execCommand(block);
	DpDriver::destroyInstance();

	DPLOGE("=== %s end ===\n", __FUNCTION__);
}

void cmdqDriverTestSubmitAsync(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
	DpDriver *pDrv;
	DpDummyBlock block;
	DpJobID handle = 0;
    DpReadbackRegs readBackRegs;

	DPLOGE("=== %s begin === \n", __FUNCTION__);

    pDrv = DpDriver::getInstance();

	DPLOGE("Test submit pDrv=0x%08x\n", pDrv);

	pDrv->submitCommand(block, &handle, 0);

    // wait frame done
    memset(&readBackRegs,0,sizeof(readBackRegs));
    status = pDrv->waitFramedone(handle, readBackRegs);
    if(DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: wait complete stream failed(%d)\n", status);
    }

	// close handle!
	DPLOGE("Test destroy Instance handle = 0x%08x\n", handle);
    DpDriver::destroyInstance();

	DPLOGE("=== %s end ===\n", __FUNCTION__);
}

void cmdqDriverTestSubmitAsyncNoWait(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
	DpDriver *pDrv;
	DpDummyBlock block;
	DpJobID handle = 0;

	DPLOGE("=== %s begin === \n", __FUNCTION__);

    pDrv = DpDriver::getInstance();

	DPLOGE("Test submit pDrv=0x%08x\n", pDrv);

	pDrv->submitCommand(block, &handle, 0);

	DPLOGE("Test destroy Instance handle = 0x%08x\n", handle);

	// close handle!
	DpDriver::destroyInstance();

	DPLOGE("=== %s end ===\n", __FUNCTION__);
}

//DEFINE_TEST_CASE("CmdqDriver", CmdqDriver, cmdqDriverTestAsyncNoWait)

void cmdqDriverTestAddressRequest(TestReporter *pReporter)
{
    int driverID = 0;
    int status;

    cmdqWriteAddressStruct paramWrite;
    cmdqReadAddressStruct paramRead;
    const uint32_t MAX_PABUFFER_COUNT = 3;
    uint32_t PABuffer[MAX_PABUFFER_COUNT] = {0L};
    uint32_t valueBuffer[MAX_PABUFFER_COUNT] = {0xDEADDEAD};
    const uint32_t readGorden = 0xcdcdabab;

	DPLOGE("=== %s begin === \n", __FUNCTION__);

    driverID = open("/dev/mtk_cmdq", O_RDONLY, 0);
    do
    {
        // request to allocate write address buffer
        memset(&paramWrite,0,sizeof(paramWrite));
        paramWrite.count = MAX_PABUFFER_COUNT;

        status = ioctl(driverID, CMDQ_IOCTL_ALLOC_WRITE_ADDRESS, &paramWrite);
        if (0 != status)
        {
            DPLOGE("allocate PA Buffer failed(%d)\n", status);
            break;
        }

        for (int i =0; i < MAX_PABUFFER_COUNT; i++)
        {
            PABuffer[i] = paramWrite.startPA + (i << 2);
            DPLOGE("PABuffer[%2d]=0x%08x, paStart=0x%08x\n", i, PABuffer[i], paramWrite.startPA);
        }

        // request to read dam buffer content via ioctl
        // note the buffer content should be 0xcdcdabab
        memset(&paramRead,0,sizeof(paramRead));
        paramRead.count = MAX_PABUFFER_COUNT;
        paramRead.dmaAddresses = (unsigned long)&(PABuffer[0]);
        paramRead.values = (unsigned long)&(valueBuffer[0]);

        status = ioctl(driverID, CMDQ_IOCTL_READ_ADDRESS_VALUE, &paramRead);
        if (0 != status)
        {
            DPLOGE("read addr value failed(%d)\n", status);
            break;
        }

        for (int i =0; i < MAX_PABUFFER_COUNT; i++)
        {
            if(readGorden != valueBuffer[i])
            {
                DPLOGE("read error, PABuffer[%2d]=0x%08x, valueBuffer[%2d]=%08x\n", i, PABuffer[i], i, valueBuffer[i]);
            }
            DPLOGE("PABuffer[%2d]=0x%08x, valueBuffer[%2d]=%08x\n", i, PABuffer[i], i, valueBuffer[i]);
        }

        // request to free write address buffer
        // note paramWrite.count & paramWrite.startPA has inited after allocation done

        status = ioctl(driverID, CMDQ_IOCTL_FREE_WRITE_ADDRESS, &paramWrite);
        if (0 != status)
        {
            DPLOGE("free PA Buffer failed(%d)\n", status);
            break;
        }

    }while(0);

    if(0 != driverID)
    {
        close(driverID);
    }

	DPLOGE("=== %s end ===\n", __FUNCTION__);
}

int main(void)
{
    // normal path

    cmdqDriverTestQueryUsage(NULL);
    cmdqDriverTestSubmit(NULL);
    cmdqDriverTestAddressRequest(NULL);
    cmdqDriverTestSubmitAsync(NULL);
    cmdqDriverTestSubmitAsyncNoWait(NULL);

    // secure path
    //cmdqDriverTestSubmitSecure(NULL);
    //cmdqDriverTestSubmitSecureWithAddrMetadata(NULL);
	return 0;
}
