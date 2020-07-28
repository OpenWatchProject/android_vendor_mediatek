#include "DpPathConnection.h"
#include "DpPathBase.h"
#include "DpWrapper_ISP.h"
#include "mmsys_config.h"
#include "tile_driver.h"
#include "tile_param.h"


bool DpPathConnection::queryMuxInfo()
{
    DpPathBase::iterator  iterator;
    DpEngineType          preType;
    DpEngineType          curType;
    uint32_t              index;
    uint32_t              inner;

    memset(m_mOutInfo, 0x0, sizeof(m_mOutInfo));
    memset(m_sInInfo, 0xFF, sizeof(m_sInInfo));
    memset(m_sOutInfo, 0xFF, sizeof(m_sOutInfo));

    for (iterator = m_pPath->begin(); iterator != m_pPath->end(); iterator++)
    {
        preType = iterator->getParentType();
        if (tNone == preType)
        {
            continue;
        }

        if (true == iterator->isOutputDisable())
        {
            continue;
        }

        curType = iterator->getEngineType();

        for (index = 0; index < MOUT_NUM; index++)
        {
            if (preType == s_moutMap[index].id)
            {
                for (inner = 0; inner < MOUT_BITS; inner++)
                {
                    if(s_moutMap[index].bit[inner] == curType)
                    {
                        if (m_mOutInfo[index] & (1 << inner))
                        {
                            return false;
                        }

                        m_mOutInfo[index] |= (1 << inner);
                        break;
                    }
                }
                if (inner == MOUT_BITS)
                {
                    return false;
                }

                break;
            }
        }

        for (index = 0; index < SEL_IN_NUM; index++)
        {
            if (curType == s_selInMap[index].id)
            {
                for (inner = 0; inner < SEL_IN_BITS; inner++)
                {
                    if(s_selInMap[index].bit[inner] == preType)
                    {
                        if (m_sInInfo[index] == inner)
                        {
                            return false;
                        }

                        m_sInInfo[index] = inner;
                        break;
                    }
                }
                if (inner == SEL_IN_BITS)
                {
                    return false;
                }

                break;
            }
        }

        for (index = 0; index < SEL_OUT_NUM; index++)
        {
            if (preType == s_selOutMap[index].id)
            {
                for (inner = 0; inner < SEL_OUT_BITS; inner++)
                {
                    if(s_selOutMap[index].bit[inner] == curType)
                    {
                        if (m_sOutInfo[index] == inner)
                        {
                            return false;
                        }

                        m_sOutInfo[index] = inner;
                        break;
                    }
                }
                if (inner == SEL_OUT_BITS)
                {
                    return false;
                }

                break;
            }
        }
    }

    return true;
}


DpPathConnection::DpPathConnection(DpPathBase *path)
    : m_pPath(path),
      m_connected(false)
{
}


DpPathConnection::~DpPathConnection()
{
}


DP_STATUS_ENUM DpPathConnection::enablePath(DpCommand &command)
{
    bool     status;
    int32_t  index;

    if (true == m_connected)
    {
        assert(0);
        return DP_STATUS_INVALID_STATE;
    }

    status = queryMuxInfo();
    if (false == status)
    {
        return DP_STATUS_INVALID_PATH;
    }

    for (index = 0; index < MOUT_NUM; index++)
    {
        if (m_mOutInfo[index] != 0)
        {
            MM_REG_WRITE_MASK(command, s_moutMap[index].reg, m_mOutInfo[index], 0xFFFFFFFF);
        }
    }

    for (index = 0; index < SEL_IN_NUM; index++)
    {
        if (m_sInInfo[index] != 0xFFFFFFFF)
        {
            MM_REG_WRITE_MASK(command, s_selInMap[index].reg, m_sInInfo[index], 0xFFFFFFFF);
        }
    }

    for (index = 0; index < SEL_OUT_NUM; index++)
    {
        if (m_sOutInfo[index] != 0xFFFFFFFF)
        {
            MM_REG_WRITE_MASK(command, s_selOutMap[index].reg, m_sOutInfo[index], 0xFFFFFFFF);
        }
    }

    m_connected = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPathConnection::disablePath(DpCommand &command)
{
    int32_t  index;

    if (false == m_connected)
    {
        assert(0);
        return DP_STATUS_INVALID_STATE;
    }

    for (index = 0; index < MOUT_NUM; index++)
    {
        if (m_mOutInfo[index] != 0)
        {
            MM_REG_WRITE_MASK(command, s_moutMap[index].reg, 0, 0xFFFFFFFF);
        }
    }

    for (index = 0; index < SEL_IN_NUM; index++)
    {
        if (m_sInInfo[index] != 0xFFFFFFFF)
        {
            MM_REG_WRITE_MASK(command, s_selInMap[index].reg, 0, 0xFFFFFFFF);
        }
    }

    for (index = 0; index < SEL_OUT_NUM; index++)
    {
        if (m_sOutInfo[index] != 0xFFFFFFFF)
        {
            MM_REG_WRITE_MASK(command, s_selOutMap[index].reg, 0, 0xFFFFFFFF);
        }
    }

    m_connected = false;

    return DP_STATUS_RETURN_SUCCESS;
}


