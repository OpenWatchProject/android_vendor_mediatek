#define IDX_DATA_ClearZoom_DIM_NS    4
#define IDX_DATA_ClearZoom_FACTOR_SZ    3
#define IDX_DATA_ClearZoom_ENTRY_NS    21

static unsigned int _cam_data_entry_ClearZoom_key0000[] = {0X000001FC, 0XA0500000, 0X00001F00, };
static unsigned int _cam_data_entry_ClearZoom_key0001[] = {0X000001FC, 0X60500000, 0X00001F00, };
static unsigned int _cam_data_entry_ClearZoom_key0002[] = {0X000001FC, 0XA0500000, 0X00002F00, };
static unsigned int _cam_data_entry_ClearZoom_key0003[] = {0X000001FC, 0X60500000, 0X00002F00, };
static unsigned int _cam_data_entry_ClearZoom_key0004[] = {0X00000001, 0X40040000, 0X00001F00, };
static unsigned int _cam_data_entry_ClearZoom_key0005[] = {0X00000001, 0X80040000, 0X00001100, };
static unsigned int _cam_data_entry_ClearZoom_key0006[] = {0X00000001, 0X00040000, 0X00001F01, };
static unsigned int _cam_data_entry_ClearZoom_key0007[] = {0X00000001, 0X40040000, 0X00002F00, };
static unsigned int _cam_data_entry_ClearZoom_key0008[] = {0X00000001, 0X80040000, 0X00002100, };
static unsigned int _cam_data_entry_ClearZoom_key0009[] = {0X00000001, 0X00040000, 0X00002F01, };
static unsigned int _cam_data_entry_ClearZoom_key0010[] = {0X00000002, 0X40080000, 0X00003F00, };
static unsigned int _cam_data_entry_ClearZoom_key0011[] = {0X00000000, 0XC0000330, 0X00003FFF, };
static unsigned int _cam_data_entry_ClearZoom_key0012[] = {0X00000000, 0XC003F000, 0X00003FFF, };
static unsigned int _cam_data_entry_ClearZoom_key0013[] = {0X00CC0000, 0XC0000000, 0X00003FFF, };
static unsigned int _cam_data_entry_ClearZoom_key0014[] = {0XFC000000, 0XC000000F, 0X00003FFF, };
static unsigned int _cam_data_entry_ClearZoom_key0015[] = {0X00000600, 0XC0000000, 0X00003FFF, };
static unsigned int _cam_data_entry_ClearZoom_key0016[] = {0X0001E000, 0XC0000000, 0X000031FF, };
static unsigned int _cam_data_entry_ClearZoom_key0017[] = {0X0001E1FC, 0XE0500000, 0X000036FF, };
static unsigned int _cam_data_entry_ClearZoom_key0018[] = {0XFC000000, 0XC003F00F, 0X000036FF, };
static unsigned int _cam_data_entry_ClearZoom_key0019[] = {0X00000001, 0X80040000, 0X00001600, };
static unsigned int _cam_data_entry_ClearZoom_key0020[] = {0X00000001, 0X80040000, 0X00002600, };

static IDX_MASK_ENTRY _cam_data_entry_ClearZoom[IDX_DATA_ClearZoom_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0000, 0, 0, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0001, 0, 1, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0002, 0, 2, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0003, 0, 3, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0004, 1, 4, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0005, 1, 5, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0006, 1, 6, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0007, 1, 7, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0008, 1, 8, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0009, 1, 9, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0010, 1, 10, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0011, 1, 12, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0012, 0, 14, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0013, 1, 15, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0014, 0, 17, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0015, 1, 18, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0016, 0, 20, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0017, 0, 21, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0018, 0, 22, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0019, 1, 23, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0020, 1, 24, 0, 0},
};

static unsigned short _cam_data_dims_ClearZoom[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

static unsigned short _cam_data_expand_ClearZoom[] = 
{0, 0, 0};

const IDX_MASK_T cam_data_ClearZoom =
{
    {IDX_ALGO_MASK, IDX_DATA_ClearZoom_DIM_NS, (unsigned short*)&_cam_data_dims_ClearZoom, (unsigned short*)&_cam_data_expand_ClearZoom},
    {IDX_DATA_ClearZoom_ENTRY_NS, IDX_DATA_ClearZoom_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_ClearZoom}
};