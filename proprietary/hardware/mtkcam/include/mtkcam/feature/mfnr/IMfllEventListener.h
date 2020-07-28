#ifndef __IMFLLEVENTLISTENER_H__
#define __IMFLLEVENTLISTENER_H__

#include "MfllTypes.h"

#include <utils/RefBase.h> // android::sp
#include <vector> // std::vector

using android::sp;
using std::vector;

namespace mfll {
/**
 *  THIS IS A PURE VIRTUAL CLASS.
 *
 *  A basic event template, we won't REALLY implement this class because user
 *  may want to implement customized event by inheriting this class
 */
class IMfllEventListener : public android::RefBase {
public:
    /**
     *  The event which is invoked while an operaion is just being to be executed.
     *  @param t                - enum EventType
     *  @param status           - A event status pass in, listener can modifiy the value directly and
     *                            it will be pass to the next listener and operation.
     *  @param *mfllCore        - A pointer to the caller (MfllCore)
     *  @param *param1          - Some events pass this argument. See MfllType.h for more information
     *  @param *param2          - Some events pass this argument. See MfllType.h for more information
     *  @notice                 - This function is called synchronized therefore do not block operation flow.
     */
    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL) = 0;

    /**
     *  The event which is invoked while an operaion has been executed done.
     *  @param t                - enum EventType
     *  @param status           - A event status pass in, listener can modifiy the value directly and
     *                            it will be pass to the next listener, but operation cannot get the status.
     *  @param *mfllCore        - A pointer to the caller (MfllCore)
     *  @param *param1          - Some events pass this argument. See MfllType.h for more information
     *  @param *param2          - Some events pass this argument. See MfllType.h for more information
     *  @notice                 - This function is called synchronized therefore do not block operation flow.
     */
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL) = 0;

    /**
     *  For performance consent, here please returns what events should be listened.
     *
     *  @return                 - A vector contains EventType to listen.
     */
    virtual vector<enum EventType> getListenedEventTypes(void) = 0;

protected:
    virtual ~IMfllEventListener(void){};
};
};//namespace mfll
#endif//__IMFLLEVENTLISTENER_H__

