
#ifndef __TLDAPC_INDEX_H__
#define __TLDAPC_INDEX_H__

/* Master's Index */
#define DAPC_MASTER_INDEX_SPI0      9
#define DAPC_MASTER_INDEX_SPI1      1

/* Slave's Domain Setting */
#define DOMAIN_AP                   0
#define DOMAIN_MD                   1
#define DOMAIN2                     2
#define DOMAIN3                     3
#define DOMAIN4                     4
#define DOMAIN5                     5
#define DOMAIN6                     6
#define DOMAIN7                     7

/* Master's Transaction */
#define SECURE_TRANSACTION          1
#define NON_SECURE_TRANSACTION      0

/* Slave's Access Permission Setting */
#define NO_PROTECTION                     0
#define SECURE_RW_ONLY                    1
#define SECURE_RW_AND_NON_SECURE_R_ONLY   2
#define NO_ACCESSIBLE                     3


/* Slave's Index */
#define DAPC_INDEX_SPI0        72
#define DAPC_INDEX_SPI1        78


#define DAPC_INDEX_DISP_OVL0    113
#define DAPC_INDEX_DISP_OVL1    114
#define DAPC_INDEX_DISP_RDMA0   115
#define DAPC_INDEX_DISP_RDMA1   116
#define DAPC_INDEX_DISP_WDMA0   117
#define DAPC_INDEX_DISP_WDMA1   128
#define DAPC_INDEX_DISP_COLOR   118
#define DAPC_INDEX_DISP_CCORR   119
#define DAPC_INDEX_DISP_AAL     120
#define DAPC_INDEX_DISP_GAMMA   121
#define DAPC_INDEX_DISP_DITHER  122

#define DAPC_INDEX_DSI               123
#define DAPC_INDEX_DPI               124
#define DAPC_INDEX_MM_MUTEX          125
#define DAPC_INDEX_SMI_LARB0         126
#define DAPC_INDEX_SMI_COMMON        127
#define DAPC_INDEX_IMGSYS_CONFIG     133
#define DAPC_INDEX_IMGSYS_SMI_LARB2  134

#define DAPC_INDEX_IMGSYS_CAM0       137
#define DAPC_INDEX_IMGSYS_CAM1       138
#define DAPC_INDEX_IMGSYS_CAM2       139
#define DAPC_INDEX_IMGSYS_CAM3       140
#define DAPC_INDEX_IMGSYS_SENINF     141
#define DAPC_INDEX_IMGSYS_CAMSV      142
#define DAPC_INDEX_IMGSYS_FDVT       144
#define DAPC_INDEX_IMGSYS_MIPI_RX    145
#define DAPC_INDEX_IMGSYS_CAM4       146
#define DAPC_INDEX_IMGSYS_CAM5       147
#define DAPC_INDEX_IMGSYS_CAM6       148

#define DAPC_INDEX_VDECSYS_GLOBAL_CONFIGURATION       149
#define DAPC_INDEX_SMI_LARB1                150
#define DAPC_INDEX_VDEC_FULL_TOP            151
#define DAPC_INDEX_VENC_GLOBAL_CON          152
#define DAPC_INDEX_SMI_LARB3                153
#define DAPC_INDEX_VENC                     154
#define DAPC_INDEX_JPEG_ENC                 155
#define DAPC_INDEX_JPEG_DEC                 156    
  

#endif // __TLDAPC_INDEX_H__
