#define IDX_MASK_PCA_DIM_NS    0
#define IDX_MASK_PCA_FACTOR_SZ    0
#define IDX_MASK_PCA_ENTRY_NS    0


static IDX_MASK_ENTRY _cam_mask_entry_PCA[IDX_MASK_PCA_ENTRY_NS] =
{
};

static unsigned short _cam_mask_dims_PCA[] = 
{
};

const IDX_MASK_T cam_mask_PCA =
{
    {IDX_ALGO_MASK, IDX_MASK_PCA_DIM_NS, (unsigned short*)&_cam_mask_dims_PCA},
    {IDX_MASK_PCA_ENTRY_NS, IDX_MASK_PCA_FACTOR_SZ, (IDX_MASK_ENTRY*)&_cam_mask_entry_PCA}
};