#include "DpEngineMutex.h"
#include "DpEngineType.h"
#include "DpPathBase.h"
#include "DpCommand.h"
#include "mmsys_mutex.h"

DpEngineMutex::DpEngineMutex(DpPathBase *path)
    : m_pPath(path),
      m_mutexID(-1),
      m_mutexMod(0)
{
    int32_t index;
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_engineSOF[index] = -1;
    }
}


DpEngineMutex::~DpEngineMutex()
{
}


bool DpEngineMutex::require(DpCommand &command)
{
    DpPathBase::iterator iterator;
    DpEngineType         curType;
    uint32_t             mutexSOF;
    uint32_t             countSOF = 0;
    int32_t              index;

    // Default value
    m_mutexID = -1;
    m_mutexMod = 0;
    mutexSOF  = 0;
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_engineSOF[index] = -1;
    }

    for (iterator = m_pPath->begin(); iterator != m_pPath->end(); iterator++)
    {
        if (true == iterator->isOutputDisable())
        {
            continue;
        }

        curType = iterator->getEngineType();
        switch(curType)
        {
        /**********************************************
         * Name            MSB LSB
         * DISP_MUTEX_MOD   16   0
         *
         * Specifies which modules are in this mutex.
         * Every bit denotes a module. Bit definition:
         *  0 mdp_rdma0
         *  1 mdp_rsz0
         *  2 mdp_rsz1
         *  3 mdp_wdma
         *  4 mdp_wrot0
         *  5 mdp_tdshp
         **********************************************/
        case tTDSHP0:
            m_mutexMod |= 1 << 5;
            break;
        case tWROT0:
            m_engineSOF[countSOF] = tWROT0;
            m_mutexMod |= 1 << 4;
            countSOF++;
            break;
        case tWDMA:
            m_engineSOF[countSOF] = tWDMA;
            m_mutexMod |= 1 << 3;
            countSOF++;
            break;
        case tSCL1:
            m_engineSOF[countSOF] = tSCL1;
            m_mutexMod |= 1 << 2;
            countSOF++;
            break;
        case tSCL0:
            m_engineSOF[countSOF] = tSCL0;
            m_mutexMod |= 1 << 1;
            countSOF++;
            break;
        case tRDMA0:
            m_mutexMod |= 1 << 0;
            m_mutexID = DISP_MUTEX_MDP_FIRST + 1;
            break;
        case tIMGI:
            m_mutexID = DISP_MUTEX_MDP_FIRST;
            break;
        case tVENC:
            DPLOGI("tVENC trying to get mutex\n");
            break;
        default:
            break;
        }
    }

    if (m_mutexID == -1)
    {
        DPLOGE("no mutex assigned...\n");
        return false;
    }

    if (m_mutexMod != 0)
    {
        // Disable disp_mutex cfg
        MM_REG_WRITE(command, MM_MUTEX_CFG, 0x0, 0x00000001);

        // Set mutex modules
        DPLOGI("DpEngineMutex: MM_MUTEX_GET %p\n", MM_MUTEX_GET);


        DPLOGI("DpEngineMutex: MM_MUTEX_MOD 0x%08x = 0x%08x\n", MM_MUTEX_MOD, m_mutexMod);
        DPLOGI("DpEngineMutex: MM_MUTEX_SOF 0x%08x = 0x%08x\n", MM_MUTEX_SOF, mutexSOF);

        MM_REG_WRITE(command, MM_MUTEX_MOD, m_mutexMod, 0x07FFFFFF);
        MM_REG_WRITE(command, MM_MUTEX_SOF, mutexSOF, 0x00000007);

    }

    return true;
}


bool DpEngineMutex::release(DpCommand &command)
{
    if (-1 == m_mutexID)
    {
        DPLOGE("DpEngineMutex: incorrect mutex id\n");
        assert(0);
        return false;
    }

    if (0 != m_mutexMod)
    {
        int32_t index;
        DPLOGI("DpEngineMutex: release mutex token %d\n", m_mutexID);

        // Wait WROT SRAM shared to DISP RDMA
        for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
        {
            switch(m_engineSOF[index])
            {
                case tWROT0:
                    MM_REG_WAIT_NO_CLEAR(command, DpCommand::SYNC_WROT0_SRAM_READY);
                    break;
                default:
                    break;
            }
        }


        MM_REG_POLL(command, MM_MUTEX_CFG, 0x0, 0x00000001);

        // Enable the mutex
        MM_REG_WRITE(command, MM_MUTEX_EN, 0x1, 0x00000001);

        for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
        {
            // Wait mutex done and clear the module list
            switch(m_engineSOF[index])
            {
                case tSCL0:
                    MM_REG_WAIT(command, DpCommand::RSZ0_FRAME_START);
                    break;
                case tSCL1:
                    MM_REG_WAIT(command, DpCommand::RSZ1_FRAME_START);
                    break;
                case tWDMA:
                    MM_REG_WAIT(command, DpCommand::WDMA_FRAME_START);
                    break;
                case tWROT0:
                    MM_REG_WAIT(command, DpCommand::WROT0_FRAME_START);
                    break;
                default:
                    break;
            }
        }

        MM_REG_WRITE(command, MM_MUTEX_MOD, 0x0, 0x07FFFFFF);


        DPLOGI("DpEngineMutex: disable mutex %d begin\n", m_mutexID);
    }

    return true;
}
