#define IDX_DATA_AINR_DIM_NS    4
#define IDX_DATA_AINR_FACTOR_SZ    3
#define IDX_DATA_AINR_ENTRY_NS    7

static unsigned int _cam_data_entry_AINR_key0000[] = {0X000001FC, 0XA0500000, 0X00001F00, };
static unsigned int _cam_data_entry_AINR_key0001[] = {0X000001FC, 0X60500000, 0X00001F00, };
static unsigned int _cam_data_entry_AINR_key0002[] = {0X000001FC, 0XA0500000, 0X00002F00, };
static unsigned int _cam_data_entry_AINR_key0003[] = {0X000001FC, 0X60500000, 0X00002F00, };
static unsigned int _cam_data_entry_AINR_key0004[] = {0XFC000000, 0XC000000F, 0X00003FFF, };
static unsigned int _cam_data_entry_AINR_key0005[] = {0X0001E1FC, 0XE0500000, 0X000036FF, };
static unsigned int _cam_data_entry_AINR_key0006[] = {0XFC000000, 0XC003F00F, 0X000036FF, };

static IDX_MASK_ENTRY _cam_data_entry_AINR[IDX_DATA_AINR_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_AINR_key0000, 0, 0, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0001, 0, 1, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0002, 0, 2, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0003, 0, 3, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0004, 0, 17, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0005, 0, 21, 0, 0},
    {(unsigned int*)&_cam_data_entry_AINR_key0006, 0, 22, 0, 0},
};

static unsigned short _cam_data_dims_AINR[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

static unsigned short _cam_data_expand_AINR[] = 
{0, 0, 1};

const IDX_MASK_T cam_data_AINR =
{
    {IDX_ALGO_MASK, IDX_DATA_AINR_DIM_NS, (unsigned short*)&_cam_data_dims_AINR, (unsigned short*)&_cam_data_expand_AINR},
    {IDX_DATA_AINR_ENTRY_NS, IDX_DATA_AINR_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_AINR}
};