#define IDX_DATA_YNRS_DIM_NS    1
#define IDX_DATA_YNRS_FACTOR_SZ    2
#define IDX_DATA_YNRS_ENTRY_NS    1

static unsigned int _cam_data_entry_YNRS_key0000[] = {0X00000000, 0X03800000, };

static IDX_MASK_ENTRY _cam_data_entry_YNRS[IDX_DATA_YNRS_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_YNRS_key0000, 0, 25, 0, 0},
};

static unsigned short _cam_data_dims_YNRS[] = 
{
    EDim_IspProfile,
};

static unsigned short _cam_data_expand_YNRS[] = 
{0, 0, 1};

const IDX_MASK_T cam_data_YNRS =
{
    {IDX_ALGO_MASK, IDX_DATA_YNRS_DIM_NS, (unsigned short*)&_cam_data_dims_YNRS, (unsigned short*)&_cam_data_expand_YNRS},
    {IDX_DATA_YNRS_ENTRY_NS, IDX_DATA_YNRS_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_YNRS}
};