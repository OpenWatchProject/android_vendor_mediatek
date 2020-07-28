#ifndef __RP_AGPS_MESSAGE_H__
#define __RP_AGPS_MESSAGE_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/RefBase.h>
#include <Parcel.h>

/*****************************************************************************
 * Name Space Declaration
 *****************************************************************************/
using ::android::RefBase;
using ::android::sp;
using ::android::Parcel;

/*****************************************************************************
 * Class RpAgpsMessage
 *****************************************************************************/
class RpAgpsMessage : public virtual RefBase {
private:
    RpAgpsMessage() : m_id(-1), m_parcel(NULL){}
    RpAgpsMessage(const RpAgpsMessage& o);
    RpAgpsMessage& operator=(const RpAgpsMessage& o);
    virtual ~RpAgpsMessage();

public:
    // Get the message ID
    // RETURNS: message id
    int32_t getId() const {
        return m_id;
    }

    // Get the message payload
    // RETURNS: a parcel as the message payload
    Parcel* getParcel() const {
        return m_parcel;
    }

    // Create a message that is sent to AGPS working thread
    // RETURNS: the reference of the message
    static sp<RpAgpsMessage> obtainMessage(
        int32_t id,     // [IN] message id
        Parcel* parcel  // [IN] message payload
        );

// Implement
private:
    // Message Id
    int32_t m_id;
    // Message Payload
    Parcel *m_parcel;
};

#endif /* __RP_AGPS_MESSAGE_H__ */