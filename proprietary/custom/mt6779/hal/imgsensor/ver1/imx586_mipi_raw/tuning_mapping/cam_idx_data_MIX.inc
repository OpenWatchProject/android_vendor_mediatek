#define IDX_DATA_MIX_DIM_NS    4
#define IDX_DATA_MIX_FACTOR_SZ    3
#define IDX_DATA_MIX_ENTRY_NS    7

static unsigned int _cam_data_entry_MIX_key0000[] = {0X00000100, 0X80000000, 0X00001100, };
static unsigned int _cam_data_entry_MIX_key0001[] = {0X00000100, 0X40000000, 0X00001100, };
static unsigned int _cam_data_entry_MIX_key0002[] = {0X00000100, 0X80000000, 0X00002100, };
static unsigned int _cam_data_entry_MIX_key0003[] = {0X00000100, 0X40000000, 0X00002100, };
static unsigned int _cam_data_entry_MIX_key0004[] = {0X00000000, 0XC0000008, 0X000031FF, };
static unsigned int _cam_data_entry_MIX_key0005[] = {0X00000100, 0XC0000000, 0X000036FF, };
static unsigned int _cam_data_entry_MIX_key0006[] = {0X00000000, 0XC0000008, 0X000036FF, };

static IDX_MASK_ENTRY _cam_data_entry_MIX[IDX_DATA_MIX_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_MIX_key0000, 0, 0, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0001, 0, 1, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0002, 8, 2, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0003, 8, 3, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0004, 0, 17, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0005, 0, 21, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIX_key0006, 0, 22, 0, 0},
};

static unsigned short _cam_data_dims_MIX[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

static unsigned short _cam_data_expand_MIX[] = 
{0, 0, 1};

const IDX_MASK_T cam_data_MIX =
{
    {IDX_ALGO_MASK, IDX_DATA_MIX_DIM_NS, (unsigned short*)&_cam_data_dims_MIX, (unsigned short*)&_cam_data_expand_MIX},
    {IDX_DATA_MIX_ENTRY_NS, IDX_DATA_MIX_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_MIX}
};