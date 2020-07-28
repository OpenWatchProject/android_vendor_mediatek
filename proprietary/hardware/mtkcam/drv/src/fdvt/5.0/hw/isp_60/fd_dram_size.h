#define fdvt_fdmode_rs_confi_size 432 // 384 bits * 9 / 8
#define fdvt_fdmode_fd_confi_size 13200 // 1600 bits * 66 / 8
#define fdvt_fdmode_yuv2rgb_confi_size 80 // 640 bits / 8

#define fdvt_attrmode_rs_confi_size 144 // 384 bits * 3 / 8
#define fdvt_attrmode_fd_confi_size 3600 // 1600 bits * 18 / 8
#define fdvt_attrmode_yuv2rgb_confi_size 80 // 640 bits / 8 

#define fdvt_posemode_rs_confi_size 144 // 384 bits * 3 / 8
#define fdvt_posemode_fd_confi_size 3600 // 1600 bits * 18 / 8
#define fdvt_posemode_yuv2rgb_confi_size 80 // 640 bits / 8 

#define fdvt_fd_result_size 49152 // 384 * 1024 / 8
#define fdvt_fd_loop_num 66
#define fdvt_fd_rpn0_loop_num 65
#define fdvt_fd_rpn1_loop_num 39
#define fdvt_fd_rpn2_loop_num 19

#define fdvt_attr_loop_num 18
#define fdvt_race_output_regression 16
#define fdvt_gender_output_regression 17

#define fdvt_pose_loop_num 18
#define fdvt_rip_output_regression 16
#define fdvt_rop_output_regression 17

#define input_WDMA_WRA_num 4
#define output_WDMA_WRA_num 4
#define kernel_RDMA_RA_num 2

#define MAX_ENQUE_FRAME_NUM 10