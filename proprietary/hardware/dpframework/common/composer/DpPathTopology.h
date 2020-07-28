#ifndef __DP_PATH_TOPOLOGY_H__
#define __DP_PATH_TOPOLOGY_H__

#include "DpEngineBase.h"
#include "DpEngineType.h"

#define LOAD_BALANCE_BY_ENG_USAGE   0

typedef DpEngineType PathInfo[tTotal];

class DpPortAdapt;

class DpPathTopology
{
public:
    DpPathTopology();

    ~DpPathTopology();

    DP_STATUS_ENUM getPathInfo(STREAM_TYPE_ENUM scenario,
                               DpPortAdapt      &sourcePort,
                               DpPortAdapt      &targetPort,
                               uint64_t         &engFlag,
                               PathInfo         &pathInfo,
                               DpEngineType     &sourceEng,
                               DpEngineType     &targetEng,
                               uint32_t         numOutputPort,
                               uint32_t         channelInfo);

    DP_STATUS_ENUM getEngUsages();

private:
    #define IS_ENGINE_FREE(flag, engine)    (0 == (flag & (1LL << engine)))

    static const DpEngineType s_topology[tTotal];
    static const int32_t      s_engOrder[tTotal];
    static const bool         s_adjency[tTotal][tTotal];

    uint32_t                  m_distance[tTotal];
    uint32_t                  m_engUsages[CMDQ_MAX_ENGINE_COUNT];

    inline DpEngineType topology(int32_t index)
    {
        return s_topology[index];
    }

    inline bool isConnect(DpEngineType source,
                          DpEngineType target)
    {
        return s_adjency[source][target];
    }

    uint32_t getWeight(DpEngineType source,
                       DpEngineType target);

    bool sortPathInfo(DpEngineType source,
                      DpEngineType target,
                      PathInfo     &info,
                      uint32_t     length);

    bool connectEdge(DpEngineType startPoint,
                     DpEngineType endPoint,
                     PathInfo     &dataPath);

    bool connectPath(PathInfo &engInfo,
                     int32_t  length,
                     uint64_t &engFlag,
                     PathInfo &dataPath);

};

#endif  // __DP_PATH_INFO_H__
