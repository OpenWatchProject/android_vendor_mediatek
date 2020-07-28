#ifndef __RP_AGPS_SLOT_CONTROLLER_H__
#define __RP_AGPS_SLOT_CONTROLLER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"

/*****************************************************************************
 * Class RpAgpsSlotController
 *****************************************************************************/
/*
 * RpAgpsSlotController is a slot controller which is created by controller
 * factory, which is used to handle GPS related AT command:
 *     AT^GPSTCPCONNREQ
 *     AT^GPSTCPCLOSEREQ
 * which is also used to trigger APN set up and send AT (AT^GPSTCpCONNIND)
 * to MD3.
 */
class RpAgpsSlotController : public RfxController {
    RFX_DECLARE_CLASS(RpAgpsSlotController);

public:
    // Constructor
    RpAgpsSlotController();

    // Destructor
    virtual ~RpAgpsSlotController();

    // Callback when the result of setup APN is got
    void onApnSetResult(
        bool result // [IN] the result to setup APN
        );

    // Calback when the state changed of data connection
    void onDataConnectionChanged(
        RfxStatusKeyEnum key,  // [IN] the key in status manager
        RfxVariant oldValue,   // [IN] the old data connection status
        RfxVariant newValue    // [IN] the new data connection status
        );

    // Used for auto test code
    // Enable test mode
    void setTestMode(
        bool test // [IN] true to run in auto test mode
        ) {
            m_testMode = test;
    }
    // Used for auto test code
    // set as OP09 or common
    void setOp09(
        bool op09 // [IN] true to run OP09 flow
        ) {
        m_mockOp09 = op09;
     }
// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleUrc(const sp<RfxMessage>& message);

// Implement
private:
    // Event in GPS URC from MD3
    enum GpsEvent {
        REQUEST_DATA_CONNECTION = 0, // MD3 request to setup APN
        CLOSE_DATA_CONNECTION,       // MD3 request to destroy APN
    };

    // Register/unRegister Data Connection state
    void registerDataConnectionStateChanged();
    void unregisterDataConnectionStateChanged();

    // Check data connnection status
    bool isDataOff();

    // Send the APN set up status to MD3
    void setConnectionState(int state);

    // Request to AGPSD to control APN
    void controlApn(int event, bool force);

    // if already request to setup APN, set as true
    bool m_settingApn;

    // run in auto test mode or not
    bool m_testMode;

    // valid in auto test mode, used to determine if OP09
    bool m_mockOp09;

    // if CTWAP set done;
    bool m_isWap;
};

#endif /* __RP_AGPS_SLOT_CONTROLLER_H__ */