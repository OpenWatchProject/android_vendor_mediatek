#define IDX_DATA_SL2F_DIM_NS    2
#define IDX_DATA_SL2F_FACTOR_SZ    1
#define IDX_DATA_SL2F_ENTRY_NS    10

static unsigned int _cam_data_entry_SL2F_key0000[] = {0X0000FC01, };
static unsigned int _cam_data_entry_SL2F_key0001[] = {0X0000FC02, };
static unsigned int _cam_data_entry_SL2F_key0002[] = {0X0000FC04, };
static unsigned int _cam_data_entry_SL2F_key0003[] = {0X0000FC08, };
static unsigned int _cam_data_entry_SL2F_key0004[] = {0X0000FC10, };
static unsigned int _cam_data_entry_SL2F_key0005[] = {0X0000FC20, };
static unsigned int _cam_data_entry_SL2F_key0006[] = {0X0000FC40, };
static unsigned int _cam_data_entry_SL2F_key0007[] = {0X0000FC80, };
static unsigned int _cam_data_entry_SL2F_key0008[] = {0X0000FD00, };
static unsigned int _cam_data_entry_SL2F_key0009[] = {0X0000FE00, };

static IDX_MASK_ENTRY _cam_data_entry_SL2F[IDX_DATA_SL2F_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_SL2F_key0000, 0, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0001, 1, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0002, 2, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0003, 3, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0004, 4, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0005, 5, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0006, 6, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0007, 7, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0008, 8, 26, 0, 0},
    {(unsigned int*)&_cam_data_entry_SL2F_key0009, 9, 26, 0, 0},
};

static unsigned short _cam_data_dims_SL2F[] = 
{
    EDim_SensorMode,
    EDim_LV,
};

static unsigned short _cam_data_expand_SL2F[] = 
{0, 0, 0};

const IDX_MASK_T cam_data_SL2F =
{
    {IDX_ALGO_MASK, IDX_DATA_SL2F_DIM_NS, (unsigned short*)&_cam_data_dims_SL2F, (unsigned short*)&_cam_data_expand_SL2F},
    {IDX_DATA_SL2F_ENTRY_NS, IDX_DATA_SL2F_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_SL2F}
};