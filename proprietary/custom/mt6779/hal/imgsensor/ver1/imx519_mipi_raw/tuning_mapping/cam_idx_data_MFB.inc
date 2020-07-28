#define IDX_DATA_MFB_DIM_NS    5
#define IDX_DATA_MFB_FACTOR_SZ    3
#define IDX_DATA_MFB_ENTRY_NS    8

static unsigned int _cam_data_entry_MFB_key0000[] = {0X00000080, 0X80000000, 0X00005900, };
static unsigned int _cam_data_entry_MFB_key0001[] = {0X00000080, 0X40000000, 0X00005900, };
static unsigned int _cam_data_entry_MFB_key0002[] = {0X00000080, 0X80000000, 0X00006900, };
static unsigned int _cam_data_entry_MFB_key0003[] = {0X00000080, 0X40000000, 0X00006900, };
static unsigned int _cam_data_entry_MFB_key0004[] = {0X00000000, 0XC0000004, 0X0003F1FF, };
static unsigned int _cam_data_entry_MFB_key0005[] = {0X00000080, 0XC0000000, 0X0003F6FF, };
static unsigned int _cam_data_entry_MFB_key0006[] = {0X00000080, 0X80000000, 0X0001BF00, };
static unsigned int _cam_data_entry_MFB_key0007[] = {0X00000080, 0X80000004, 0X0003FF00, };

static IDX_MASK_ENTRY _cam_data_entry_MFB[IDX_DATA_MFB_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_MFB_key0000, 0, 0, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0001, 0, 1, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0002, 8, 2, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0003, 8, 3, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0004, 0, 14, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0005, 0, 18, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0006, 0, 22, 0, 0},
    {(unsigned int*)&_cam_data_entry_MFB_key0007, 0, 24, 0, 0},
};

static unsigned short _cam_data_dims_MFB[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
    EDim_Zoom,
};

static unsigned short _cam_data_expand_MFB[] = 
{0, 0, 1};

const IDX_MASK_T cam_data_MFB =
{
    {IDX_ALGO_MASK, IDX_DATA_MFB_DIM_NS, (unsigned short*)&_cam_data_dims_MFB, (unsigned short*)&_cam_data_expand_MFB},
    {IDX_DATA_MFB_ENTRY_NS, IDX_DATA_MFB_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_MFB}
};