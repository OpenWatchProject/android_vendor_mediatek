# Define project-specific configuration options here.
#
# A configuration option is defined in the simple format:
#   <KEY> = <VALUE>
# where spaces around the = character is optional.
#
# You can define the same configuration options appeared in platform.mk with
# different values. The options defined here take higher precedence.
#
# **** DO NOT **** define anything other than configuration options here.
# If you need to customize project-specific source files, compiler flags
# or required libraries, add them to CompilerOption.mk.

###############################################################################
# The following options are for FPGA, EVB, or chip verification. They should
# be deleted or set to "no" on phone projects. The former is preferred.
###############################################################################
CFG_FPGA_UART_SUPPORT = no
CFG_TESTSUITE_SUPPORT = no
CFG_TEST_PROGRAM_SUPPORT = no
CFG_SLT_SUPPORT = no
CFG_DVT_SUPPORT = no

###############################################################################
# Cache options
#   When the cache options are turned on, more code and data than the SRAM size
#   can be loaded and run on SCP. However, DRAM is slower than SRAM. Therefore,
#   performance will possibly degrade when the cache options are turned on.
###############################################################################
CFG_CACHE_SUPPORT = yes
CFG_DYNAMIC_DRAM_V2 = yes
CFG_L1C_ISR_SUPPORT = yes
# CFG_DMGR_DEBUG = 0xff
CFG_SENSORS_INDRAM = yes

###############################################################################
# Project options begin here.
###############################################################################
CFG_MTK_VOW_SUPPORT = yes
CFG_MTK_VOW_2E2K_SUPPORT = no
CFG_MTK_VOW_DUAL_MIC_SUPPORT = no
CFG_MTK_VOW_BARGE_IN_SUPPORT = yes
CFG_MTK_VOW_VENDOR_MTK_SUPPORT = yes
CFG_MTK_VOW_VENDOR_AMAZON_SUPPORT = no
CFG_MTK_VOW_VENDOR_GOOGLE_SUPPORT = no


CFG_CHRE_SUPPORT = yes
CFG_CONTEXTHUB_FW_SUPPORT = yes
CFG_ACCGYRO_SUPPORT = yes
CFG_LSM6DSM_SUPPORT = yes
CFG_ALSPS_SUPPORT = yes
CFG_CM36558_SUPPORT = yes
CFG_MAGNETOMETER_SUPPORT = yes
CFG_MAG_CALIBRATION_IN_AP = yes
CFG_AKM09918_SUPPORT = yes
CFG_BAROMETER_SUPPORT = yes
CFG_BMP280_SUPPORT = yes
CFG_STEP_COUNTER_SUPPORT = yes
CFG_STEP_DETECTOR_SUPPORT = yes
CFG_SIGNIFICANT_MOTION_SUPPORT = yes
CFG_WAKEUP_SUPPORT = yes
CFG_STATIONARY_SUPPORT = yes
CFG_MOTION_SUPPORT = yes
CFG_WIN_ORIENTATION_SUPPORT = yes
CFG_OVERLAY_INIT_SUPPORT = yes
CFG_OVERLAY_DEBUG_SUPPORT = yes
CFG_LIFT_SUPPORT = yes
CFG_GLANCE_SUPPORT = yes
CFG_PICKUP_SUPPORT = yes
CFG_LIFT_PUTDOWN_SUPPORT = yes
