#ifndef __PQ_PICTUREMODE_H__
#define __PQ_PICTUREMODE_H__

#include <stdio.h>
#include "ddp_drv.h"
#include "cust_color.h"
#include "PQServiceCommon.h"

class PQPictureMode
{
public:
    PQPictureMode() {
        m_pq_param = NULL;
        m_pq_mode = PQ_PIC_MODE_STANDARD;
    }

    ~PQPictureMode() {}

    DISP_PQ_PARAM *getPQParam(int32_t scenario_index) {
        if (scenario_index >= PQ_SCENARIO_COUNT || m_pq_mode == PQ_PIC_MODE_USER_DEF)
            scenario_index = 0;

        return m_pq_param + scenario_index;
    };
    DISP_PQ_PARAM *getPQParamImage(void) {
        return getPQParam(getScenarioIndex(SCENARIO_PICTURE));
    };
    DISP_PQ_PARAM *getPQParamVideo(void) {
        return getPQParam(getScenarioIndex(SCENARIO_VIDEO));
    };
    static int32_t getScenarioIndex(int32_t scenario)
    {
        int32_t scenario_index;

        if (scenario ==  SCENARIO_PICTURE)
        {
            scenario_index = 0;
        }
        else if (scenario == SCENARIO_VIDEO || scenario == SCENARIO_VIDEO_CODEC)
        {
            scenario_index = 1;
        }
        else if (scenario == SCENARIO_ISP_PREVIEW || scenario == SCENARIO_ISP_CAPTURE)
        {
            scenario_index = 2;
        }
        else
        {
            scenario_index = 0;
        }

        return scenario_index;
    }

protected:
    DISP_PQ_PARAM *m_pq_param;
    int32_t m_pq_mode;
};

class PicModeStandard : public PQPictureMode
{
public:
    PicModeStandard(DISP_PQ_PARAM *pqparam_table) {
        m_pq_mode = PQ_PIC_MODE_STANDARD;
        m_pq_param = pqparam_table + m_pq_mode * PQ_SCENARIO_COUNT;
    };
    ~PicModeStandard() {}
};

class PicModeVivid : public PQPictureMode
{
public:
    PicModeVivid(DISP_PQ_PARAM *pqparam_table) {
        m_pq_mode = PQ_PIC_MODE_VIVID;
        m_pq_param = pqparam_table + m_pq_mode * PQ_SCENARIO_COUNT;
    };
    ~PicModeVivid() {}
};

class PicModeUserDef : public PQPictureMode
{
public:
    PicModeUserDef(DISP_PQ_PARAM *pqparam_table) {
        m_pq_mode = PQ_PIC_MODE_USER_DEF;
        m_pq_param = pqparam_table + m_pq_mode * PQ_SCENARIO_COUNT;
    };
    ~PicModeUserDef() {}

    DISP_PQ_PARAM *getPQUserDefParam(void) {
        return getPQParam(getScenarioIndex(SCENARIO_PICTURE));
    };

};
#endif
