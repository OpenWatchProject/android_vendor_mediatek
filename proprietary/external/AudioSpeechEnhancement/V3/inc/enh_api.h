#ifndef _ENH_API_H
#define _ENH_API_H

typedef signed short   Word16;
typedef signed int     Word32;
typedef unsigned short uWord16;
typedef unsigned int   uWord32;

typedef struct {

    uWord32 enhance_pars[60];   
    Word32 App_table;
    Word32 Fea_Cfg_table;
    Word32 MIC_DG;
    Word32 sample_rate;
    Word32 frame_rate;
    Word32 MMI_ctrl;
    Word32 RCV_DG;      // for VoIP, 0xE3D, downlink PGA cost-down
    Word16 DMNR_cal_data[76];
    Word16 Compen_filter[270];  
    Word16 PCM_buffer[1920];
    Word16 EPL_buffer[4800];
    Word32 Device_mode;
    Word32 MMI_MIC_GAIN;
    Word32 Near_end_vad;

    Word32 *SCH_mem; // caster to (SCH_mem_struct*) in every alloc function
} SPH_ENH_ctrl_struct;

Word32  ENH_API_Get_Memory ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );
Word16  ENH_API_Alloc ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word32* mem_ptr );
Word16  ENH_API_Rst   ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );
void ENH_API_Process  ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );
Word16  ENH_API_Free  ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );
Word16  ENH_API_Get_Version  ( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );

#endif
