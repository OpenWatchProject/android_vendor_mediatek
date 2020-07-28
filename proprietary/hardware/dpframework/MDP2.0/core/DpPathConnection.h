#ifndef __DP_PATH_CONNECTION_H__
#define __DP_PATH_CONNECTION_H__

#include "DpDataType.h"
#include "DpEngineType.h"
#include "DpPlatform.h"

class DpPathBase;
struct TILE_PARAM_STRUCT;

#define MOUT_NUM        int32_t(sizeof(s_moutMap) / sizeof(mout_t))
#define MOUT_BITS       (sizeof(s_moutMap[0].bit) / sizeof(int32_t))

#define SEL_IN_NUM      int32_t(sizeof(s_selInMap) / sizeof(sel_t))
#define SEL_IN_BITS     (sizeof(s_selInMap[0].bit) / sizeof(int32_t))

#define SEL_OUT_NUM     int32_t(sizeof(s_selOutMap) / sizeof(sout_t))
#define SEL_OUT_BITS    (sizeof(s_selOutMap[0].bit) / sizeof(int32_t))

class DpPathConnection
{
public:
    DpPathConnection(DpPathBase *path);

    ~DpPathConnection();

#ifdef WPE_TILE
    DP_STATUS_ENUM initTilePath(struct TILE_PARAM_STRUCT *, struct WPE_TPIPE_CONFIG_STRUCT * = 0);
#else
    DP_STATUS_ENUM initTilePath(struct TILE_PARAM_STRUCT *);
#endif //WPE_TILE

    DP_STATUS_ENUM enablePath(DpCommand&);

    DP_STATUS_ENUM disablePath(DpCommand&);

private:
    // Hardware Dependency
    typedef struct mout_t
    {
        int32_t         id;
        int32_t         bit[5];
        uint32_t        reg;
    } mout_t;

    typedef struct sout_t
    {
        int32_t         id;
        int32_t         bit[2];
        uint32_t        reg;
    } sout_t;

    typedef struct selection_s
    {
        int32_t         id;
        int32_t         bit[8];
        uint32_t        reg;
    } sel_t;

    static const mout_t s_moutMap[MOUT_MAP_SIZE];
    static const sel_t  s_selInMap[SEL_IN_SIZE];
    static const sout_t s_selOutMap[SOUT_MAP_SIZE];

    DpPathBase          *m_pPath;
    bool                m_connected;
    uint32_t            m_mOutInfo[tTotal];
    uint32_t            m_sInInfo[tTotal];
    uint32_t            m_sOutInfo[tTotal];

    bool queryMuxInfo();
};

#endif  // __DP_PATH_CONNECTION_H__
