#define IDX_DATA_MIXER3_DIM_NS    5
#define IDX_DATA_MIXER3_FACTOR_SZ    4
#define IDX_DATA_MIXER3_ENTRY_NS    17

static unsigned int _cam_data_entry_MIXER3_key0000[] = {0X00000040, 0X00000000, 0X00800000, 0X0000060F, };
static unsigned int _cam_data_entry_MIXER3_key0001[] = {0X00000040, 0X00000000, 0X00800000, 0X0000002F, };
static unsigned int _cam_data_entry_MIXER3_key0002[] = {0X00000040, 0X00000000, 0X00800000, 0X000007EF, };
static unsigned int _cam_data_entry_MIXER3_key0003[] = {0X00000040, 0X00000000, 0X00400000, 0X0000060F, };
static unsigned int _cam_data_entry_MIXER3_key0004[] = {0X00000040, 0X00000000, 0X00400000, 0X0000002F, };
static unsigned int _cam_data_entry_MIXER3_key0005[] = {0X00000040, 0X00000000, 0X00400000, 0X000007EF, };
static unsigned int _cam_data_entry_MIXER3_key0006[] = {0X00000040, 0X00000000, 0X00800000, 0X000007F7, };
static unsigned int _cam_data_entry_MIXER3_key0007[] = {0X00000040, 0X00000000, 0X00400000, 0X000007F7, };
static unsigned int _cam_data_entry_MIXER3_key0008[] = {0X00000000, 0X00000002, 0XFFC00000, 0X0000061F, };
static unsigned int _cam_data_entry_MIXER3_key0009[] = {0X00000000, 0X00000002, 0XFFC00000, 0X0000003F, };
static unsigned int _cam_data_entry_MIXER3_key0010[] = {0X00000000, 0X00000002, 0XFFC00000, 0X000007FF, };
static unsigned int _cam_data_entry_MIXER3_key0011[] = {0X00000040, 0X00000000, 0XFFC00000, 0X0000061A, };
static unsigned int _cam_data_entry_MIXER3_key0012[] = {0X00000040, 0X00000000, 0XFFC00000, 0X0000003A, };
static unsigned int _cam_data_entry_MIXER3_key0013[] = {0X00000040, 0X00000000, 0XFFC00000, 0X000007FA, };
static unsigned int _cam_data_entry_MIXER3_key0014[] = {0X00000000, 0X00000002, 0XFFC00000, 0X0000061A, };
static unsigned int _cam_data_entry_MIXER3_key0015[] = {0X00000000, 0X00000002, 0XFFC00000, 0X0000003A, };
static unsigned int _cam_data_entry_MIXER3_key0016[] = {0X00000000, 0X00000002, 0XFFC00000, 0X000007FA, };

static IDX_MASK_ENTRY _cam_data_entry_MIXER3[IDX_DATA_MIXER3_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_MIXER3_key0000, 0, 0, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0001, 11, 0, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0002, 1, 0, 6, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0003, 0, 1, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0004, 11, 1, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0005, 1, 1, 6, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0006, 12, 2, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0007, 12, 3, 0, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0008, 0, 17, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0009, 11, 17, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0010, 1, 17, 6, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0011, 0, 21, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0012, 11, 21, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0013, 1, 21, 6, 0},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0014, 0, 22, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0015, 11, 22, 6, 1},
    {(unsigned int*)&_cam_data_entry_MIXER3_key0016, 1, 22, 6, 0},
};

static unsigned short _cam_data_dims_MIXER3[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
    EDim_LV,
};

static unsigned short _cam_data_expand_MIXER3[] = 
{0, 0, 1};

const IDX_MASK_T cam_data_MIXER3 =
{
    {IDX_ALGO_MASK, IDX_DATA_MIXER3_DIM_NS, (unsigned short*)&_cam_data_dims_MIXER3, (unsigned short*)&_cam_data_expand_MIXER3},
    {IDX_DATA_MIXER3_ENTRY_NS, IDX_DATA_MIXER3_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_MIXER3}
};