typedef enum {
  MTEE_CELL_INFO_TYPE_UNKNOWN = 0,
  MTEE_CELL_INFO_TYPE_GSM    = 1,
  MTEE_CELL_INFO_TYPE_CDMA   = 2,
  MTEE_CELL_INFO_TYPE_LTE    = 3,
  MTEE_CELL_INFO_TYPE_WCDMA  = 4
} MTEE_CellInfoType;

typedef struct {
    int mcc;    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown */
    int mnc;    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown */
    int lac;    /* 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
    int cid;    /* 16-bit GSM Cell Identity described in TS 27.007, 0..65535, INT_MAX if unknown  */
    int arfcn;  /* 16-bit GSM Absolute RF channel number, INT_MAX if unknown */
    uint8_t bsic;/* 6-bit Base Station Identity Code, 0xFF if unknown */
} MTEE_CellIdentityGsm;

typedef struct {
    int mcc;    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown  */
    int mnc;    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown  */
    int lac;    /* 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
    int cid;    /* 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknown  */
    int psc;    /* 9-bit UMTS Primary Scrambling Code described in TS 25.331, 0..511, INT_MAX if unknown */
    int uarfcn; /* 16-bit UMTS Absolute RF Channel Number, INT_MAX if unknown */
} MTEE_CellIdentityWcdma;

typedef struct {
    int mcc;    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown  */
    int mnc;    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown  */
    int ci;     /* 28-bit Cell Identity described in TS ???, INT_MAX if unknown */
    int pci;    /* physical cell id 0..503, INT_MAX if unknown  */
    int tac;    /* 16-bit tracking area code, INT_MAX if unknown  */
    int earfcn; /* 18-bit LTE Absolute RC Channel Number, INT_MAX if unknown */
} MTEE_CellIdentityLte;

typedef struct {
    int networkId;      /* Network Id 0..65535, INT_MAX if unknown */
    int systemId;       /* CDMA System Id 0..32767, INT_MAX if unknown  */
    int basestationId;  /* Base Station Id 0..65535, INT_MAX if unknown  */
    int longitude;      /* Longitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -2592000
                         * to 2592000, both values inclusive (corresponding to a range of -180
                         * to +180 degrees). INT_MAX if unknown */

    int latitude;       /* Latitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -1296000
                         * to 1296000, both values inclusive (corresponding to a range of -90
                         * to +90 degrees). INT_MAX if unknown */
} MTEE_CellIdentityCdma;

typedef struct {
  MTEE_CellInfoType  cellInfoType;   /* cell type for selecting from union CellInfo */
  union {
    MTEE_CellIdentityGsm     gsm;
    MTEE_CellIdentityCdma    cdma;
    MTEE_CellIdentityLte     lte;
    MTEE_CellIdentityWcdma   wcdma;
  } CellInfo;
} MTEE_CellInfo;

typedef struct {
  MTEE_CellInfo cell1;
  MTEE_CellInfo cell2; // valid when UE camp on CDMA cell.
} MTEE_CellInfoList;

typedef struct MTEE_CellInfo_info {
	void *buffer;
	int size;
} MTEE_CellInfo_info_t;

typedef struct MTEE_CellInfo_Handle {
	MTEE_CellInfo_info_t info;
} MTEE_CellInfo_Handle_t;

MTEE_CellInfo_Handle_t *MTEE_OpenCellInfo(cellinfo_buffer_t *cellinfo_buffer);
TZ_RESULT MTEE_GetCellInfo(MTEE_CellInfoList *cell);

extern TZ_RESULT TA_Mem_QueryChunkmem (uint32_t mem_handle, MTEE_MEM_PARAM *param, uint64_t in_key);
extern TZ_RESULT TA_Mem_QueryChmDebugInfo (uint32_t chm_handle, uint64_t in_key);
extern TZ_RESULT TA_Mem_QueryChmPA (uint32_t mem_handle, uint64_t in_key);

// SHM struct
typedef struct {
    uint32_t magic_numbe;
    uint32_t last_index;
    uint8_t string[10][2048];
} SHM_Info;

