#ifndef _ISP_REG_H_
#define _ISP_REG_H_

//include for reg size
#include "camera_isp.h"

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
#ifndef MUINT64
typedef uint64_t MUINT64;
#endif


#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)

/**
    REG size for each module is 0x1000
*/
#define REG_SIZE        (ISP_REG_RANGE)
#define CAM_BASE_RANGE  (REG_SIZE)
#define CAM_BASE_RANGE_SPECIAL  (sizeof(cam_reg_t)) //for CQ special baseaddress
#define UNI_BASE_RANGE  (REG_SIZE/2)
#define CAMSV_BASE_RANGE    (REG_SIZE/2)
#define FDVT_BASE_RANGE (REG_SIZE)
#define WPE_BASE_RANGE  (REG_SIZE)
#define RSC_BASE_RANGE  (REG_SIZE)
#define DPE_BASE_RANGE  (REG_SIZE)

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */
#include "isp_reg_cam.h"

/* auto insert ralf auto gen above */

#endif // _ISP_REG_H_
