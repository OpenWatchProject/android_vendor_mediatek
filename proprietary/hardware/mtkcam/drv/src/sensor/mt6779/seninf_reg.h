#ifndef _SENINF_REG_H_
#define _SENINF_REG_H_

#define PACKING volatile
#include "seninf_cam_mux.h"
#include "seninf_top.h"
#include "seninf1_mux.h"
#include "seninf1.h"
#include "seninf1_csi2.h"
#include "seninf_tg1.h"

#include "mipi_rx_ana_cdphy_csi0a.h" //0x11c8 0000
#include "csi0_cphy_top.h" //0x11c8 3000
#include "csi0_dphy_top.h" //0x11c8 2000

typedef unsigned int FIELD;

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif

#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)

#define SENINF_READ_REG(RegBase, RegName) (RegBase->RegName.Raw)
#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

#endif // _SENINF_REG_H_
