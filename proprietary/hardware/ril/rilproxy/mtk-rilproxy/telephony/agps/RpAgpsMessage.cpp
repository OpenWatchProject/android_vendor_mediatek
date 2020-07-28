/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpAgpsMessage.h"
#include "RpAgpsLog.h"

/*****************************************************************************
 * Class RpAgpsMessage
 *****************************************************************************/
RpAgpsMessage::~RpAgpsMessage() {
    //AGPS_LOGD("DestroyMessage %d, 0x%x", m_id, m_parcel);
    if (m_parcel != NULL) {
        delete(m_parcel);
        m_parcel = NULL;
    }
}


sp<RpAgpsMessage> RpAgpsMessage::obtainMessage(int id, Parcel* parcel) {
    sp<RpAgpsMessage> new_msg = new RpAgpsMessage();
    AGPS_LOGD("obtainMessage %d, 0x%x", id, parcel);
    new_msg->m_id = id;
    new_msg->m_parcel = parcel;
    return new_msg;
}