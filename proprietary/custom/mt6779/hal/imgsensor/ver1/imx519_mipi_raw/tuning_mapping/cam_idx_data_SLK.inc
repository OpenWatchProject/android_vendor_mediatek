#define IDX_DATA_SLK_DIM_NS    1
#define IDX_DATA_SLK_FACTOR_SZ    1
#define IDX_DATA_SLK_ENTRY_NS    10

static unsigned int _cam_data_entry_SLK_key0000[] = {0X00000001, };
static unsigned int _cam_data_entry_SLK_key0001[] = {0X00000002, };
static unsigned int _cam_data_entry_SLK_key0002[] = {0X00000004, };
static unsigned int _cam_data_entry_SLK_key0003[] = {0X00000008, };
static unsigned int _cam_data_entry_SLK_key0004[] = {0X00000010, };
static unsigned int _cam_data_entry_SLK_key0005[] = {0X00000020, };
static unsigned int _cam_data_entry_SLK_key0006[] = {0X00000040, };
static unsigned int _cam_data_entry_SLK_key0007[] = {0X00000080, };
static unsigned int _cam_data_entry_SLK_key0008[] = {0X00000100, };
static unsigned int _cam_data_entry_SLK_key0009[] = {0X00000200, };

static IDX_MASK_ENTRY _cam_data_entry_SLK[IDX_DATA_SLK_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_SLK_key0000, 0, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0001, 1, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0002, 2, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0003, 3, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0004, 4, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0005, 5, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0006, 6, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0007, 7, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0008, 8, 27, 0, 0},
    {(unsigned int*)&_cam_data_entry_SLK_key0009, 9, 27, 0, 0},
};

static unsigned short _cam_data_dims_SLK[] = 
{
    EDim_SensorMode,
};

static unsigned short _cam_data_expand_SLK[] = 
{0, 0, 0};

const IDX_MASK_T cam_data_SLK =
{
    {IDX_ALGO_MASK, IDX_DATA_SLK_DIM_NS, (unsigned short*)&_cam_data_dims_SLK, (unsigned short*)&_cam_data_expand_SLK},
    {IDX_DATA_SLK_ENTRY_NS, IDX_DATA_SLK_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_SLK}
};