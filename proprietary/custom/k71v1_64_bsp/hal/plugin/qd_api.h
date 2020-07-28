#ifndef _QD_API_H_
#define _QD_API_H_

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************************
// Get Version
//****************************************************************************************
extern unsigned int SonyQdLibGetVersion(void);

//****************************************************************************************
// Defines
//****************************************************************************************

#define D_SONY_QD_LIB_FULL_IMAGE_WIDTH		(5184)
#define	D_SONY_QD_LIB_FULL_IMAGE_HEIGHT		(3880)
#define	D_SONY_QD_LIB_FULL_IMAGE_CID		(0x3)

//****************************************************************************************
// Initialize / Finalize
//****************************************************************************************
typedef struct{
	//---- single static defect ----
	int sg_dfct_num;		// the number of single static defect
	int *sg_dfct_addr;		// [ u25 ( upper-u13 = x-addr, lower-u12 = y-addr ) ]
	//---- FD static defect ----
	int fd_dfct_num;		// the number of FD static defect
	int *fd_dfct_addr;		// [ u25 ( upper-u13 = x-addr, lower-u12 = y-addr ) ]
} SonyQdLibDfctTbl_t;

#define D_SONY_QD_LIB_GAIN_KNOT_H		(7)
#define D_SONY_QD_LIB_GAIN_KNOT_V		(5)
typedef struct{
	unsigned char gain[4*4][D_SONY_QD_LIB_GAIN_KNOT_H*D_SONY_QD_LIB_GAIN_KNOT_V];	// gain table ( every 4 x 4 colors have own table )
} SonyQdLibGainTbl_t;

extern int SonyQdLibInitialize
						(SonyQdLibDfctTbl_t *p_dfcttbl, SonyQdLibGainTbl_t *p_gaintbl);

extern int SonyQdLibFinalize
						(void);


//****************************************************************************************
// Execute
//****************************************************************************************
typedef struct{
	int				fd;					// fd for using ION buffer
	unsigned short	*p_addr;			// [ u10 ] type of the output image pixel
} SonyQdLibImageOut_t;

typedef struct{
	int				fd;					// fd for using ION buffer
	unsigned short	*p_addr;			// [ u10 ] type of the input image pixel
} SonyQdLibImageIn_t;

typedef struct{
	// Image geometric imformation
	int				width;				// Image width
	int				height;				// Image height
	int				offset_x;			// Left-top pixel x-offset of full image
	int				offset_y;			// Left-top pixel y-offset of full image
	int				cid;				// Color location ( Left-top is 0:R 1:Gr 2:Gb 3:B  others:R )
	// White balance information
	unsigned short	wb_gain_r;			// [ u8.8, 0x0100 - 0xFFFF ] white balance gain (R)  : 0x0100 = x1.0
	unsigned short	wb_gain_gr;			// [ u8.8, 0x0100 - 0xFFFF ] white balance gain (Gr) : 0x0100 = x1.0
	unsigned short	wb_gain_gb;			// [ u8.8, 0x0100 - 0xFFFF ] white balance gain (Gb) : 0x0100 = x1.0
	unsigned short	wb_gain_b;			// [ u8.8, 0x0100 - 0xFFFF ] white balance gain (B)  : 0x0100 = x1.0
	// Gain information
	unsigned char	analog_gain;		// [ u8,     0x00 -   0xFF ] analog gain (SMIA) : 0 = x1.0
	unsigned short	digital_gain;		// [ u8.8, 0x0100 - 0xFFFF ] digital gain : 0x0100 = x1.0
	// Library controls and settings
	int				mode;				// Performance mode ( 0:Quality 1:Balanced 2:Speed  others:Quality )
	int				apc_mode;			// APC mode for Quality ( 0:Off 1:LL 2:L 3:M 4:H othres:Off )

} SonyQdLibConfig_t;

extern int SonyQdLibExecute
						(SonyQdLibImageOut_t *p_dst, SonyQdLibImageIn_t *p_src, SonyQdLibConfig_t *p_conf);



#ifdef __cplusplus
}
#endif

#endif	// _QD_API_H_
// Copyright 2018 Sony Semiconductor Solutions
