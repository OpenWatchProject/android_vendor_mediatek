#define IDX_MASK_ClearZoom_DIM_NS    4
#define IDX_MASK_ClearZoom_FACTOR_SZ    4
#define IDX_MASK_ClearZoom_ENTRY_NS    21

static unsigned int _cam_mask_entry_ClearZoom_key0000[] = {0X0000007C, 0X00000000, 0X01500000, 0X0000001E, };
static unsigned int _cam_mask_entry_ClearZoom_key0001[] = {0X0000007C, 0X00000000, 0X00D00000, 0X0000001E, };
static unsigned int _cam_mask_entry_ClearZoom_key0002[] = {0X0000007C, 0X00000000, 0X01500000, 0X0000002E, };
static unsigned int _cam_mask_entry_ClearZoom_key0003[] = {0X0000007C, 0X00000000, 0X00800000, 0X0000002E, };
static unsigned int _cam_mask_entry_ClearZoom_key0004[] = {0X00000001, 0X00000000, 0X00800040, 0X0000001E, };
static unsigned int _cam_mask_entry_ClearZoom_key0005[] = {0X00000001, 0X00000000, 0X01000040, 0X00000012, };
static unsigned int _cam_mask_entry_ClearZoom_key0006[] = {0X00000001, 0X00000000, 0X02000040, 0X0000001E, };
static unsigned int _cam_mask_entry_ClearZoom_key0007[] = {0X00000001, 0X00000000, 0X00800040, 0X0000002E, };
static unsigned int _cam_mask_entry_ClearZoom_key0008[] = {0X00000001, 0X00000000, 0X01000040, 0X00000022, };
static unsigned int _cam_mask_entry_ClearZoom_key0009[] = {0X00000001, 0X00000000, 0X02000040, 0X0000002E, };
static unsigned int _cam_mask_entry_ClearZoom_key0010[] = {0X00000002, 0X00000000, 0X00800080, 0X0000003E, };
static unsigned int _cam_mask_entry_ClearZoom_key0011[] = {0X00000000, 0X0000071C, 0XFF81C000, 0X0000003F, };
static unsigned int _cam_mask_entry_ClearZoom_key0012[] = {0X00000000, 0X0000C000, 0XFF800000, 0X0000003F, };
static unsigned int _cam_mask_entry_ClearZoom_key0013[] = {0X01C70000, 0X00000000, 0XFF800700, 0X0000003F, };
static unsigned int _cam_mask_entry_ClearZoom_key0014[] = {0XF0000000, 0X00000003, 0XFF800000, 0X0000003F, };
static unsigned int _cam_mask_entry_ClearZoom_key0015[] = {0X00000300, 0X00000000, 0XFF800000, 0X0000003F, };
static unsigned int _cam_mask_entry_ClearZoom_key0016[] = {0X0000F000, 0X00000000, 0XFF800000, 0X00000033, };
static unsigned int _cam_mask_entry_ClearZoom_key0017[] = {0X0000F07C, 0X00000000, 0XFFD00000, 0X00000035, };
static unsigned int _cam_mask_entry_ClearZoom_key0018[] = {0XF0000000, 0X0000C003, 0XFF800000, 0X00000035, };
static unsigned int _cam_mask_entry_ClearZoom_key0019[] = {0X00000001, 0X00000000, 0X01000040, 0X00000014, };
static unsigned int _cam_mask_entry_ClearZoom_key0020[] = {0X00000001, 0X00000000, 0X01000040, 0X00000024, };

static IDX_MASK_ENTRY _cam_mask_entry_ClearZoom[IDX_MASK_ClearZoom_ENTRY_NS] =
{
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0000, 0, 0},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0001, 0, 1},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0002, 0, 2},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0003, 0, 3},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0004, 1, 4},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0005, 1, 5},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0006, 1, 6},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0007, 1, 7},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0008, 1, 8},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0009, 1, 9},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0010, 1, 10},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0011, 1, 12},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0012, 0, 14},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0013, 1, 15},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0014, 0, 17},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0015, 1, 18},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0016, 0, 20},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0017, 0, 21},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0018, 0, 22},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0019, 1, 23},
    {(unsigned int*)&_cam_mask_entry_ClearZoom_key0020, 1, 24},
};

static unsigned short _cam_mask_dims_ClearZoom[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

const IDX_MASK_T cam_mask_ClearZoom =
{
    {IDX_ALGO_MASK, IDX_MASK_ClearZoom_DIM_NS, (unsigned short*)&_cam_mask_dims_ClearZoom},
    {IDX_MASK_ClearZoom_ENTRY_NS, IDX_MASK_ClearZoom_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_mask_entry_ClearZoom}
};