#include "DpNotifier.h"
#include "DpDriver.h"
#include "DpDataType.h"

DP_STATUS_ENUM notifyScenario(DP_SCENARIO_ENUM scenario)
{
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    
    if(scenario != DP_SCENARIO_NONE &&
       scenario != DP_SCENARIO_FORCE_MMDVFS)
    {
        status = DpDriver::getInstance()->notifyEngineWROT();
    }
    return status;
}