#ifndef _CAM_IDX_STRUCT_INT_H_
#define _CAM_IDX_STRUCT_INT_H_

typedef struct
{
	unsigned char   type;
	unsigned short  dim_ns;
	unsigned short  *dims;
} IDX_BASE_T;

// ***** type for MASK *****

typedef struct
{
	unsigned int    *key;
	unsigned short  value;
	unsigned short  scenario;
} IDX_MASK_ENTRY;

typedef struct
{
	unsigned int    entry_ns;
	unsigned int    key_sz;
	IDX_MASK_ENTRY* entry;
} IDX_MASK_DATA_T;

typedef struct
{
	IDX_BASE_T      base;
	IDX_MASK_DATA_T data;
} IDX_MASK_T;

typedef struct
{
        unsigned short* dim_sz;
        unsigned short* idx_array;
        unsigned short* scenario_array;
} IDX_DM_DATA_T;

typedef struct
{
	IDX_BASE_T      base;
	IDX_DM_DATA_T data;
} IDX_DM_T;


typedef struct IDX_MODULE_ARRAY_T
{
    unsigned short  idx_factor_ns[NSIspTuning::EDim_NUM];
    IDX_BASE_T*     modules[NSIspTuning::EModule_NUM];
    unsigned short scenario_ns;
    char (*scenarios)[][64];
} IDX_MODULE_ARRAY, *PIDX_MODULE_ARRAY;

#endif
