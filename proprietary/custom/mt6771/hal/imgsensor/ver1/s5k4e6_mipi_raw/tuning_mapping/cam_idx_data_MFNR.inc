#define IDX_MASK_MFNR_DIM_NS    4
#define IDX_MASK_MFNR_FACTOR_SZ    4
#define IDX_MASK_MFNR_ENTRY_NS    7

static unsigned int _cam_mask_entry_MFNR_key0000[] = {0X0000007C, 0X00000000, 0X01500000, 0X0000001E, };
static unsigned int _cam_mask_entry_MFNR_key0001[] = {0X0000007C, 0X00000000, 0X00D00000, 0X0000001E, };
static unsigned int _cam_mask_entry_MFNR_key0002[] = {0X0000007C, 0X00000000, 0X01500000, 0X0000002E, };
static unsigned int _cam_mask_entry_MFNR_key0003[] = {0X0000007C, 0X00000000, 0X00800000, 0X0000002E, };
static unsigned int _cam_mask_entry_MFNR_key0004[] = {0XF0000000, 0X00000003, 0XFF800000, 0X0000003F, };
static unsigned int _cam_mask_entry_MFNR_key0005[] = {0X0000F07C, 0X00000000, 0XFFD00000, 0X00000035, };
static unsigned int _cam_mask_entry_MFNR_key0006[] = {0XF0000000, 0X0000C003, 0XFF800000, 0X00000035, };

static IDX_MASK_ENTRY _cam_mask_entry_MFNR[IDX_MASK_MFNR_ENTRY_NS] =
{
    {(unsigned int*)&_cam_mask_entry_MFNR_key0000, 0, 0},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0001, 0, 1},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0002, 1, 2},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0003, 1, 3},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0004, 0, 17},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0005, 0, 21},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0006, 0, 22},
};

static unsigned short _cam_mask_dims_MFNR[] = 
{
    EDim_IspProfile,
    EDim_SensorMode,
    EDim_Flash,
    EDim_FaceDetection,
};

const IDX_MASK_T cam_mask_MFNR =
{
    {IDX_ALGO_MASK, IDX_MASK_MFNR_DIM_NS, (unsigned short*)&_cam_mask_dims_MFNR},
    {IDX_MASK_MFNR_ENTRY_NS, IDX_MASK_MFNR_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_mask_entry_MFNR}
};