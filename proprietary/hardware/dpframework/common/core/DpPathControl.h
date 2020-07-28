#ifndef __DP_PATH_CONTROL_H__
#define __DP_PATH_CONTROL_H__

#include "DpDataType.h"
#include "DpPathComposer.h"

class DpStream;

class DpPathControl
{
public:
    DpPathControl(DpStream *stream);

    ~DpPathControl();

    DP_STATUS_ENUM initStream()
    {
        return onInitStream();
    }

    DP_STATUS_ENUM startStream()
    {
        return onStartStream();
    }

    DP_STATUS_ENUM waitStream()
    {
        return onWaitStream();
    }

    DP_STATUS_ENUM stopStream()
    {
        return onStopStream();
    }

    DP_STATUS_ENUM dumpDebugStream()
    {
        return onDumpDebugStream();
    }

    DpPathComposer& getComposer()
    {
        return m_composer;
    }

protected:
    DP_STATUS_ENUM onInitStream();

    DP_STATUS_ENUM onStartStream();

    DP_STATUS_ENUM onWaitStream();

    DP_STATUS_ENUM onStopStream();

    DP_STATUS_ENUM onDumpDebugStream();

    DpPathComposer m_composer;
};

#endif  // __DP_PATH_CONTROL_H__
