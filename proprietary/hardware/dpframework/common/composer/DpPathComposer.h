#ifndef __DP_PATH_COMPOSER_H__
#define __DP_PATH_COMPOSER_H__

#include "DpComposerBase.h"
#include "DpPathTopology.h"

#define DP_COMPOSER_MAX_TEMP_PORT   32

class DpPathComposer: public DpComposerBase
{
public:
    DpPathComposer(DpStream *stream);

    virtual ~DpPathComposer();

private:
    DpStream         *m_pStream;
    DpPathTopology   m_topology;
    DpPortAdapt      *m_pTempPort[DP_COMPOSER_MAX_TEMP_PORT];
    DpBufferPoolBase *m_pTempPool[DP_COMPOSER_MAX_TEMP_PORT];
    uint32_t         m_tempCount;

    DP_STATUS_ENUM reset();

    virtual DP_STATUS_ENUM onCompose();

    template <class PathType, class NodeType>
    DP_STATUS_ENUM setupPath(STREAM_TYPE_ENUM scenario);

    template <class PathType, class NodeType>
    DP_STATUS_ENUM createPath(DpPortAdapt  *pInPort,
                              DpPortAdapt  *pOutPort,
                              PathInfo     &pathInfo,
                              DpEngineType source,
                              DpEngineType target,
                              PathType     *pDataPath);

    template <class PathType, class NodeType>
    DP_STATUS_ENUM mergePath(DpPortAdapt  *pInPort,
                             DpPortAdapt  *pOutPort,
                             PathInfo     &pathInfo,
                             DpEngineType source,
                             DpEngineType target,
                             PathType     *pDataPath);

    template <class PathType, class NodeType>
    DP_STATUS_ENUM createPathAndNew(DpPortAdapt  *pInPort,
                                    DpPortAdapt  *pOutPort,
                                    PathInfo     &pathInfo,
                                    DpEngineType source,
                                    DpEngineType target,
                                    PathType     *&pDataPath);
};

#endif  // __DP_PATH_COMPOSER_H__
