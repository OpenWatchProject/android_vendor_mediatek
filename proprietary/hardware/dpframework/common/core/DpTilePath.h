#ifndef __DP_TILE_PATH_H__
#define __DP_TILE_PATH_H__

#include "DpDataPath.h"

typedef DpPath<DpTileEngine, DpSingleThread> DpTilePathST;
typedef DpPath<DpTileEngine, DpMultiThread>  DpTilePathMT;

#endif  // __DP_TILE_PATH_H__
