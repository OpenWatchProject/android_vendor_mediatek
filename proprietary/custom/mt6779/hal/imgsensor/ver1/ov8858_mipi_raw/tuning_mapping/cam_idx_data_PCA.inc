#define IDX_DATA_PCA_DIM_NS    0
#define IDX_DATA_PCA_FACTOR_SZ    0
#define IDX_DATA_PCA_ENTRY_NS    1

static unsigned int _cam_data_entry_PCA_key0000[] = {};

static IDX_MASK_ENTRY _cam_data_entry_PCA[IDX_DATA_PCA_ENTRY_NS] =
{
    {(unsigned int*)&_cam_data_entry_PCA_key0000, 0, 26, 0, 0},
};

static unsigned short _cam_data_dims_PCA[] = 
{
};

static unsigned short _cam_data_expand_PCA[] = 
{0, 0, 0};

const IDX_MASK_T cam_data_PCA =
{
    {IDX_ALGO_MASK, IDX_DATA_PCA_DIM_NS, (unsigned short*)&_cam_data_dims_PCA, (unsigned short*)&_cam_data_expand_PCA},
    {IDX_DATA_PCA_ENTRY_NS, IDX_DATA_PCA_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_data_entry_PCA}
};