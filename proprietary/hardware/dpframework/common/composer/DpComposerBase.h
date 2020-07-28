#ifndef __DP_COMPOSER_BASE_H__
#define __DP_COMPOSER_BASE_H__

#include "DpList.h"
#include "DpProfiler.h"

class DpPathBase;
class DpStream;

class DpComposerBase
{
public:
    typedef ListPtr<DpPathBase> PathList;
    typedef PathList::iterator  iterator;

    DpComposerBase()
    {
    }

    virtual ~DpComposerBase()
    {
    }

    DP_STATUS_ENUM compose()
    {
        DP_TRACE_CALL();
        return onCompose();
    }

    iterator begin()
    {
        return m_pathList.begin();
    }

    iterator end()
    {
        return m_pathList.end();
    }

protected:
    PathList m_pathList;

    virtual DP_STATUS_ENUM onCompose() = 0;  
};

#endif  // __DP_COMPOSER_BASE_H__
