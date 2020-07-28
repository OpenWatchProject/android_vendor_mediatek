/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBasics.h"
#include "RpAgpsHandler.h"
#include "RpAgpsdAdapter.h"
#include "RfxRootController.h"
#include "RpAgpsNSlotController.h"
#include "RfxMainThread.h"

/*****************************************************************************
 * Class RpAgpsHandler
 *****************************************************************************/
void RpAgpsHandler::sendMessage(sp<Looper> looper) {
    looper->sendMessage(this, m_dummyMsg);
}


/*****************************************************************************
 * Class RpAgpsWorkingThreadHandler
 *****************************************************************************/
void RpAgpsWorkingThreadHandler::handleMessage(const Message& message) {
    RFX_UNUSED(message);
    RpAgpsdAdapter *adapter = RFX_OBJ_GET_INSTANCE(RpAgpsdAdapter);
    adapter->processOutputMessage(m_msg);
}


/*****************************************************************************
 * Class RpAgpsMainThreadHandler
 *****************************************************************************/
void RpAgpsMainThreadHandler::onHandleMessage(const Message& message) {
    RFX_UNUSED(message);
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RpAgpsNSlotController *ctrl =
            (RpAgpsNSlotController *)root->findController(
                    RFX_OBJ_CLASS_INFO(RpAgpsNSlotController));
    if (ctrl != NULL) {
        ctrl->onHandleAgpsMessage(m_msg);
    }
}
