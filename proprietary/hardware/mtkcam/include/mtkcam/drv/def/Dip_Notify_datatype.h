#ifndef _DIP_NOTIFY_DATATYPE_H_
#define _DIP_NOTIFY_DATATYPE_H_

/******************************************************************************
 *
 * @struct EFeatureModule
 * @brief module enum for feature
 * @details
 *
 ******************************************************************************/
enum EDipModule
{
    EDipModule_NONE      = 0x0000,
    EDipModule_SRZ1      = 0x0001,
    EDipModule_SRZ2      = 0x0002,
    EDipModule_SRZ3      = 0x0004,
    EDipModule_SRZ4      = 0x0008,
    EDipModule_SRZ5      = 0x0010,
};

/*******************************************************************************
* Pipe ModuleParam Info (Descriptor).
********************************************************************************/
struct DipModuleCfg
{
    EDipModule   eDipModule;
    MVOID*      moduleStruct;
public:     //// constructors.

    DipModuleCfg()
        : eDipModule(EDipModule_NONE)
        , moduleStruct(NULL)
    {
    }
    //
};

/******************************************************************************
 *
 * @struct SrzSize
 * @brief srz in/out size setting
 * @details
 *
 ******************************************************************************/
struct _SRZ_SIZE_INFO_
{
 public:
     MUINT32 in_w;
     MUINT32 in_h;
     MUINT32 out_w;
     MUINT32 out_h;
    MUINT32	crop_x;
    MUINT32	crop_y;
    MUINT32	crop_floatX; /* x float precise - 32 bit */
    MUINT32	crop_floatY; /* y float precise - 32 bit */
    unsigned long	crop_w;
    unsigned long	crop_h;

    _SRZ_SIZE_INFO_()
        : in_w(0x0)
        , in_h(0x0)
        , out_w(0x0)
        , out_h(0x0)
        , crop_x(0x0)
        , crop_y(0x0)
        , crop_floatX(0x0)
        , crop_floatY(0x0)
        , crop_w(0x0)
        , crop_h(0x0)
    {}
};

#endif
