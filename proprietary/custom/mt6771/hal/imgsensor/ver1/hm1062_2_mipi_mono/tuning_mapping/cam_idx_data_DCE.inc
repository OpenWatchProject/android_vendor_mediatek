#define IDX_MASK_DCE_DIM_NS    4
#define IDX_MASK_DCE_FACTOR_SZ    4
#define IDX_MASK_DCE_ENTRY_NS    23

static unsigned int _cam_mask_entry_DCE_key0000[] = {0X0000007C, 0X00000000, 0X00800000, 0X00000007, };
static unsigned int _cam_mask_entry_DCE_key0001[] = {0X0000007C, 0X00000000, 0X00400000, 0X00000007, };
static unsigned int _cam_mask_entry_DCE_key0002[] = {0X0000007C, 0X00000000, 0X00800000, 0X0000000B, };
static unsigned int _cam_mask_entry_DCE_key0003[] = {0X0000007C, 0X00000000, 0X00400000, 0X0000000B, };
static unsigned int _cam_mask_entry_DCE_key0004[] = {0X00000001, 0X00000000, 0X00400040, 0X00000007, };
static unsigned int _cam_mask_entry_DCE_key0005[] = {0X00000001, 0X00000000, 0X00800040, 0X00000007, };
static unsigned int _cam_mask_entry_DCE_key0006[] = {0X00000001, 0X00000000, 0X01000040, 0X00000007, };
static unsigned int _cam_mask_entry_DCE_key0007[] = {0X00000001, 0X00000000, 0X00400040, 0X0000000B, };
static unsigned int _cam_mask_entry_DCE_key0008[] = {0X00000001, 0X00000000, 0X00800040, 0X0000000B, };
static unsigned int _cam_mask_entry_DCE_key0009[] = {0X00000001, 0X00000000, 0X01000040, 0X0000000B, };
static unsigned int _cam_mask_entry_DCE_key0010[] = {0X00000002, 0X00000000, 0X00400080, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0011[] = {0X00000002, 0X00000000, 0X01000080, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0012[] = {0X00000000, 0X0000071C, 0XFFC1C000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0013[] = {0X00000000, 0X000038E0, 0XFFCE0000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0014[] = {0X00000000, 0X0000C000, 0XFFC00000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0015[] = {0X01C70000, 0X00000000, 0XFFC00700, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0016[] = {0X0E380000, 0X00000000, 0XFFC03800, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0017[] = {0XF0000000, 0X00000003, 0XFFC00000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0018[] = {0X00000300, 0X00000000, 0XFFC00000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0019[] = {0X00000C00, 0X00000000, 0XFFC00000, 0X0000000F, };
static unsigned int _cam_mask_entry_DCE_key0020[] = {0X0000F000, 0X00000000, 0XFFC00000, 0X0000000D, };
static unsigned int _cam_mask_entry_DCE_key0021[] = {0X0000F07C, 0X00000000, 0XFFC00000, 0X0000000E, };
static unsigned int _cam_mask_entry_DCE_key0022[] = {0XF0000000, 0X0000C003, 0XFFC00000, 0X0000000E, };

static IDX_MASK_ENTRY _cam_mask_entry_DCE[IDX_MASK_DCE_ENTRY_NS] =
{
    {(unsigned int*)&_cam_mask_entry_DCE_key0000, 0},
    {(unsigned int*)&_cam_mask_entry_DCE_key0001, 0},
    {(unsigned int*)&_cam_mask_entry_DCE_key0002, 0},
    {(unsigned int*)&_cam_mask_entry_DCE_key0003, 0},
    {(unsigned int*)&_cam_mask_entry_DCE_key0004, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0005, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0006, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0007, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0008, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0009, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0010, 2},
    {(unsigned int*)&_cam_mask_entry_DCE_key0011, 2},
    {(unsigned int*)&_cam_mask_entry_DCE_key0012, 3},
    {(unsigned int*)&_cam_mask_entry_DCE_key0013, 4},
    {(unsigned int*)&_cam_mask_entry_DCE_key0014, 5},
    {(unsigned int*)&_cam_mask_entry_DCE_key0015, 6},
    {(unsigned int*)&_cam_mask_entry_DCE_key0016, 7},
    {(unsigned int*)&_cam_mask_entry_DCE_key0017, 8},
    {(unsigned int*)&_cam_mask_entry_DCE_key0018, 1},
    {(unsigned int*)&_cam_mask_entry_DCE_key0019, 2},
    {(unsigned int*)&_cam_mask_entry_DCE_key0020, 0},
    {(unsigned int*)&_cam_mask_entry_DCE_key0021, 9},
    {(unsigned int*)&_cam_mask_entry_DCE_key0022, 10},
};

static unsigned short _cam_mask_dims_DCE[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

const IDX_MASK_T cam_mask_DCE =
{
    {IDX_ALGO_MASK, IDX_MASK_DCE_DIM_NS, (unsigned short*)&_cam_mask_dims_DCE},
    {IDX_MASK_DCE_ENTRY_NS, IDX_MASK_DCE_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_mask_entry_DCE}
};