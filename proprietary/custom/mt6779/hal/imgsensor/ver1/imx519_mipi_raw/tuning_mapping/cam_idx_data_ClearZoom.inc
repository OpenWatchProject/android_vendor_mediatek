#define IDX_DATA_ClearZoom_DIM_NS    6
#define IDX_DATA_ClearZoom_FACTOR_SZ    3
#define IDX_DATA_ClearZoom_ENTRY_NS    23

static unsigned int _cam_data_entry_ClearZoom_key0000[] = {0X000001FC, 0X80700000, 0X00016700, };
static unsigned int _cam_data_entry_ClearZoom_key0001[] = {0X000001FC, 0X40700000, 0X00016700, };
static unsigned int _cam_data_entry_ClearZoom_key0002[] = {0X000001FC, 0X80700000, 0X0001A700, };
static unsigned int _cam_data_entry_ClearZoom_key0003[] = {0X000001FC, 0X40700000, 0X0001A700, };
static unsigned int _cam_data_entry_ClearZoom_key0004[] = {0X00000001, 0X40040000, 0X000F7D02, };
static unsigned int _cam_data_entry_ClearZoom_key0005[] = {0X00000001, 0X80040000, 0X00016500, };
static unsigned int _cam_data_entry_ClearZoom_key0006[] = {0X00000001, 0X00040000, 0X000F7D01, };
static unsigned int _cam_data_entry_ClearZoom_key0007[] = {0X00000001, 0X40040000, 0X000FBD02, };
static unsigned int _cam_data_entry_ClearZoom_key0008[] = {0X00000001, 0X80040000, 0X0001A500, };
static unsigned int _cam_data_entry_ClearZoom_key0009[] = {0X00000001, 0X00040000, 0X000FBD01, };
static unsigned int _cam_data_entry_ClearZoom_key0010[] = {0X00000002, 0X40080000, 0X000FFD02, };
static unsigned int _cam_data_entry_ClearZoom_key0011[] = {0X00CC0000, 0XC0000000, 0X000FFFFF, };
static unsigned int _cam_data_entry_ClearZoom_key0012[] = {0XFC000000, 0XC000000F, 0X000FC7FF, };
static unsigned int _cam_data_entry_ClearZoom_key0013[] = {0X00000600, 0XC0000000, 0X000FFFFF, };
static unsigned int _cam_data_entry_ClearZoom_key0014[] = {0X0001E000, 0XC0000000, 0X000FC7FF, };
static unsigned int _cam_data_entry_ClearZoom_key0015[] = {0X0001E1FC, 0XC0700000, 0X000FDBFF, };
static unsigned int _cam_data_entry_ClearZoom_key0016[] = {0X00000001, 0X80040000, 0X00015900, };
static unsigned int _cam_data_entry_ClearZoom_key0017[] = {0X00000001, 0X80040000, 0X00019900, };
static unsigned int _cam_data_entry_ClearZoom_key0018[] = {0X000001FC, 0X80700000, 0X0006FF00, };
static unsigned int _cam_data_entry_ClearZoom_key0019[] = {0X00000001, 0X80040000, 0X0006FF00, };
static unsigned int _cam_data_entry_ClearZoom_key0020[] = {0XFFFFFFFF, 0XBFFFFFFF, 0X000FFF00, };
static unsigned int _cam_data_entry_ClearZoom_key0021[] = {0XFFFFFFFF, 0X7FFFFFFF, 0X000FFFF8, };
static unsigned int _cam_data_entry_ClearZoom_key0022[] = {0XFFFFFFFF, 0X3FFFFFFF, 0X000FFF07, };

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
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0015, 0, 18, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0016, 1, 19, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0017, 1, 20, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0018, 0, 22, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0019, 1, 23, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0020, 0, 24, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0021, 1, 25, 0, 0},
    {(unsigned int*)&_cam_data_entry_ClearZoom_key0022, 1, 26, 0, 0},
};

static unsigned short _cam_data_dims_ClearZoom[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_FrontBin,
    EDim_Flash,
    EDim_FaceDetection,
    EDim_Zoom,
};

static unsigned short _cam_data_expand_ClearZoom[] = 
{0, 0, 0};

const IDX_MASK_T cam_data_ClearZoom =
{
    {IDX_ALGO_MASK, IDX_DATA_ClearZoom_DIM_NS, (unsigned short*)&_cam_data_dims_ClearZoom, (unsigned short*)&_cam_data_expand_ClearZoom},
    {IDX_DATA_ClearZoom_ENTRY_NS, IDX_DATA_ClearZoom_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_ClearZoom}
};