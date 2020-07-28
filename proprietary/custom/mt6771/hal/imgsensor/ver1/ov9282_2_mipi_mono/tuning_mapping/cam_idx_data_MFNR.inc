#define IDX_MASK_MFNR_DIM_NS    4
#define IDX_MASK_MFNR_FACTOR_SZ    4
#define IDX_MASK_MFNR_ENTRY_NS    7

static unsigned int _cam_mask_entry_MFNR_key0000[] = {0X00000078, 0X00000000, 0X00800000, 0X00000007, };
static unsigned int _cam_mask_entry_MFNR_key0001[] = {0X00000078, 0X00000000, 0X00400000, 0X00000007, };
static unsigned int _cam_mask_entry_MFNR_key0002[] = {0X00000078, 0X00000000, 0X00800000, 0X0000000B, };
static unsigned int _cam_mask_entry_MFNR_key0003[] = {0X00000078, 0X00000000, 0X00400000, 0X0000000B, };
static unsigned int _cam_mask_entry_MFNR_key0004[] = {0XC0000000, 0X00000003, 0XFFC00000, 0X0000000F, };
static unsigned int _cam_mask_entry_MFNR_key0005[] = {0X00000078, 0X00000000, 0XFFC00000, 0X0000000E, };
static unsigned int _cam_mask_entry_MFNR_key0006[] = {0XC0000000, 0X00000003, 0XFFC00000, 0X0000000E, };

static IDX_MASK_ENTRY _cam_mask_entry_MFNR[IDX_MASK_MFNR_ENTRY_NS] =
{
    {(unsigned int*)&_cam_mask_entry_MFNR_key0000, 0},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0001, 0},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0002, 1},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0003, 1},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0004, 0},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0005, 0},
    {(unsigned int*)&_cam_mask_entry_MFNR_key0006, 0},
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