#ifndef __IMFLLEVENTS_H__
#define __IMFLLEVENTS_H__

#include "IMfllEventListener.h"

#include <utils/RefBase.h> // android::sp
#include <vector> // std::vector

using android::sp;

namespace mfll {

/**
 *  IMfllEvents is an operator for MFLL usage, like to register a new event or
 *  remove one.
 */
class IMfllEvents : public android::RefBase {
public:
    static IMfllEvents* createInstance(void);
    virtual void destroyInstance(void);

public:
    /* If t is EventType_Size, register all the events */
    virtual void registerEventListener(const enum EventType &t, const sp<IMfllEventListener> &event) = 0;

    /**
     *  To register an event listener at the first position
     */
    virtual void registerEventListenerAsFirst(const enum EventType &t, const sp<IMfllEventListener> &event) = 0;

    /* If t is EventType_Size, remove all listener from all events */
    virtual void removeEventListener(const enum EventType &t, const IMfllEventListener *event) = 0;

/* operations "on" */
public:
    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL) = 0;

/* operations "done" */
public:
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL) = 0;

protected:
    virtual ~IMfllEvents(void){};
}; /* class MfllEvents */
}; /* namespace mfll */

#endif//__IMFLLEVENTS_H__
