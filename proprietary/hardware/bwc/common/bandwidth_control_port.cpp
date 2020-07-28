//for open()
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
}

//errno
#include    <errno.h>

//SMI kernel include
#include    <linux/ioctl.h>
#include    <string.h>
#include    <linux/disp_session.h>


#include    "bandwidth_control.h"
#include    "bandwidth_control_private.h"
#include    "BWCProfileAdapter.h"
#include    "BWCConfiguration.h"

//#ifdef FLAG_SUPPORT_SMI_SETTING
#if 1
#include    "mt_smi.h"
#endif

#ifdef BWC_RRC_SUPPORT
#include    "refresh_rate_control.h"
#endif

#define MAX_EMI_CTRL_STR_SIZE  (128)


BWCProfileAdapter profileAdaptor( profileSetting, total_bwc_profiles );
BWCProfileAdapter defaultProfileAdaptor(
    defaultProfileSetting,
        total_default_bwc_profiles );
BWCProfileHelper profileHelper(&defaultProfileAdaptor, &profileAdaptor);

/*=============================================================================
 SMI Bandwidth Information Query
 =============================================================================*/
BWC_MONITOR::BWC_MONITOR(){
    this->smi_fd = -1;
    this->start();
}

BWC_MONITOR::~BWC_MONITOR(){
    this->stop();
}

int BWC_MONITOR::start(){
    return 0;
}

int BWC_MONITOR::stop(){
    return 0;
}

unsigned int BWC_MONITOR::get_smi_bw_state(){
    return 1920 * 1080 * 4;
}

const char* BwcProfileType_GetStr( BWC_PROFILE_TYPE profile ){
   const char* profileName = profileHelper.getName(profile);
   if (profileName == NULL) {
       BWC_ERROR("Invalid profile_type = %d\n", (int)profile );
       return "Unknown profile.";
   }else{
       return profileName;
   }
}

/*=============================================================================
    SMI Bandwidth Control
  =============================================================================*/

extern "C"
{

static MTK_SMI_BWC_SCEN map_bwc_profile_to_smi(BWC_PROFILE_TYPE bwc_profile)
{
    int smi_profile_queried = profileHelper.getSMIProfile(bwc_profile);

    if(smi_profile_queried == -1)
        return SMI_BWC_SCEN_NORMAL;
    else
        return (MTK_SMI_BWC_SCEN)smi_profile_queried;

}
}
int BWC::smi_bw_ctrl_set(
    BWC_PROFILE_TYPE profile_type,
    BWC_VCODEC_TYPE codec_type,
    bool bOn ){
    BWC_UNUSED(codec_type);
//#ifdef FLAG_SUPPORT_SMI_SETTING
#if 1

    int                     smi_fd = -1;    //smi device driver node file descriptor
    MTK_SMI_BWC_CONFIG      cfg;

    smi_fd = open("/dev/MTK_SMI", O_RDONLY);

    if( -1 == smi_fd ){
        BWC_ERROR("Open SMI(/dev/MTK_SMI) driver file failed.:%s\n",
            strerror(errno));
        return -1;
    }

    cfg.b_on_off = ((0 < bOn) ? 1 : 0);
    cfg.scenario = profileHelper.getSMIProfile(profile_type);

    if( cfg.scenario == -1)
    {
        BWC_ERROR("Given profile type:%d cannot find bwc mapping profile", profile_type);
        close(smi_fd);
        return -1;
    }
    if( ioctl(smi_fd, MTK_IOC_SMI_BWC_CONFIG, &cfg) < 0 )
    {
        BWC_ERROR("MTK_IOC_SMI_BWC_CONFIG failed.:%s\n" , strerror(errno) );
        close(smi_fd);
        return -1;
    }

    close(smi_fd);

    // pass scene info to refresh rate ctrol
#ifdef BWC_RRC_SUPPORT
    RefreshRateControl rrc;
    rrc.setScenario((int)profile_type, bOn);
#endif
    return 0;

#else
    BWC_INFO("smi_bw_ctrl_set: NULL FUNCTION\n");
    return 0;
#endif
}

/*=============================================================================
    EMI Bandwidth Control
  =============================================================================*/
static int emi_ctrl_str_generate(
    BWC_PROFILE_TYPE profile_type,
    BWC_VCODEC_TYPE codec_type,
    bool bOn,
    char* out_str ){
    const char *p_cmdstr_profile = NULL;
    char *p_cmdstr_switch  = NULL;
    int max_length = 32;
    BWC_UNUSED(codec_type);

    p_cmdstr_profile = profileHelper.getEmiProfileStr(profile_type);
    if( p_cmdstr_profile == NULL ) {
        BWC_ERROR("Invalid profile_type = %d\n", (int)profile_type );
        return -1;
    }

    p_cmdstr_switch = ( bOn == true ) ? (char*)" ON":(char*)" OFF";

#ifdef __STDC_LIB_EXT1__
    strncpy_s( out_str, max_length, p_cmdstr_profile, max_length );
    strncat_s( out_str, max_length, p_cmdstr_switch, max_length );
#else
    strncpy( out_str, p_cmdstr_profile, max_length );
    strncat( out_str, p_cmdstr_switch, max_length );
#endif

    return 0;

}

int BWC::emi_bw_ctrl_set(
    BWC_PROFILE_TYPE profile_type,
    BWC_VCODEC_TYPE codec_type,
    bool bOn,
    char* out_str ){
    const char  *con_sce_file = "";
#if defined(BWC_EMI_V2)
    con_sce_file = "/sys/bus/platform/drivers/emi_ctrl/concurrency_scenario";
#elif defined(BWC_EMI_V1)
    con_sce_file = "/sys/bus/platform/drivers/mem_bw_ctrl/concurrency_scenario";
#endif
    int         fd;

    if (!out_str) {
        BWC_ERROR("String hanlder is unavailable\n");
        return -1;
    }

    if( emi_ctrl_str_generate( profile_type, codec_type, bOn, out_str ) < 0 ){
        BWC_ERROR("emi_ctrl_str_generate failed!\n");
        return -1;
    }

    fd = open(con_sce_file, O_WRONLY);

    if( fd == -1 ){
        BWC_ERROR("fail to open mem_bw_ctrl driver file\n");
        return -1;
    }else{
        /* enable my scenario before running my application*/
        if (write(fd, out_str, strlen(out_str)) < 0) {
            BWC_ERROR("fail to write mem_bw_ctrl driver file\n");
            close(fd);
            return -1;
        }
    }

    close(fd);

    return 0;
}

/*=============================================================================
    EMI DDR TYPE Get
  =============================================================================*/
void BWC::emi_ddr_type_string_get(char* out_str, int len){
    BWC_UNUSED(out_str);
    BWC_UNUSED(len);
}
BWC::EMI_DDR_TYPE BWC::emi_ddr_type_get( void ){
    char ddr_type_str[MAX_EMI_CTRL_STR_SIZE] = {0};
    emi_ddr_type_string_get(ddr_type_str, MAX_EMI_CTRL_STR_SIZE);

    //Mapping DDR type
    if( strncmp(ddr_type_str, "LPDDR2", strlen("LPDDR2")) == 0 ){
        return EDT_LPDDR2;
    }

    if( strncmp(ddr_type_str, "DDR3", strlen("DDR3")) == 0 ){
        return EDT_DDR3;
    }

    if( strncmp(ddr_type_str, "LPDDR3", strlen("LPDDR3")) == 0 ){
        return EDT_LPDDR3;
    }

    if( strncmp(ddr_type_str, "mDDR", strlen("mDDR")) == 0 ){
        return EDT_mDDR;
    }

    return EDT_NONE;

}

/*=============================================================================
    Modem Speed Control
  =============================================================================*/

int BWC::modem_speed_profile_set( BWC::MODEM_SPEED_PROFILE profile ){
    BWC_UNUSED(profile);
    return 0;
}

/*=============================================================================
 BWCHelper
 =============================================================================*/

int BWCHelper::set_bwc_mm_property( int propterty_id, int value1, int value2 ){
    MTK_SMI_BWC_INFO_SET cfg;
    int smi_fd = -1;

    smi_fd = open("/dev/MTK_SMI", O_RDONLY);

    if( -1 == smi_fd ){
        BWC_ERROR("Open SMI(/dev/MTK_SMI) driver file failed.:%s\n",
            strerror(errno));
        return -1;
    }

    cfg.property = propterty_id;
    cfg.value1 = value1;
    cfg.value2 = value2;

    if( ioctl(smi_fd, MTK_IOC_SMI_BWC_INFO_SET, &cfg) < 0 ){
        BWC_ERROR("MTK_IOC_SMI_BWC_INFO_SET failed.:%s\n", strerror(errno));
        close(smi_fd);
        return -1;
    }


    smi_fd = close(smi_fd);

    if( -1 == smi_fd ){
        BWC_ERROR("Close SMI(/dev/MTK_SMI) driver file failed.:%s\n",
            strerror(errno));
        return -1;
    }

    return 0;

}

int BWCHelper::get_bwc_mm_property( MTK_SMI_BWC_MM_INFO_USER * properties ){

    int smi_fd = -1;

    smi_fd = open("/dev/MTK_SMI", O_RDONLY);

    if( -1 == smi_fd ){
        BWC_ERROR("Open SMI(/dev/MTK_SMI) driver file failed.:%s\n",
            strerror(errno));
        return -1;
    }


    if( ioctl(smi_fd, MTK_IOC_SMI_BWC_INFO_GET, (void *)properties) < 0 ){
        BWC_ERROR("MTK_IOC_SMI_BWC_INFO_GET failed.:%s\n", strerror(errno));
        close(smi_fd);
        return -1;
    }

    smi_fd = close(smi_fd);

    if( -1 == smi_fd ){
        BWC_ERROR("Close SMI(/dev/MTK_SMI) driver file failed.:%s\n",
            strerror(errno));
        return -1;
    }

    return 0;

}

void BWCHelper::profile_add(
    MTK_SMI_BWC_MM_INFO_USER * properties,
    BWC_PROFILE_TYPE profile ){

    properties->concurrent_profile = (properties->concurrent_profile | (1
        << profile));
    this->set_bwc_mm_property(SMI_BWC_USER_INFO_CON_PROFILE,
        properties->concurrent_profile, 0);
}

void BWCHelper::profile_remove(
    MTK_SMI_BWC_MM_INFO_USER * properties,
    BWC_PROFILE_TYPE profile ){

    properties->concurrent_profile = (properties->concurrent_profile & (~(1
        << profile)));
    this->set_bwc_mm_property(SMI_BWC_USER_INFO_CON_PROFILE,
        properties->concurrent_profile, 0);
}

int BWCHelper::profile_get( MTK_SMI_BWC_MM_INFO_USER * properties ){
    return properties->concurrent_profile;

}


extern "C"
{

static unsigned int g_mmdvfs_camera_mode = MMDVFS_CAMERA_MODE_FLAG_DEFAULT;

static void mmdvfs_parse_arg(va_list arg_ptr, MTK_MMDVFS_CMD *cmd)
{
    while (1)
    {
        mmdvfs_parameter_enum type;
        type = (mmdvfs_parameter_enum)va_arg(arg_ptr, int);
        if (type == MMDVFS_PARAMETER_EOF)
        {
            break;
        }
        unsigned int value;

        value = va_arg(arg_ptr, unsigned int);

        switch (type)
        {
            case MMDVFS_SENSOR_SIZE:
                cmd->sensor_size = value;
                break;
            case MMDVFS_SENSOR_FPS:
                cmd->sensor_fps = value;
                break;
            case MMDVFS_CAMERA_MODE_PIP:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_PIP;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_PIP;
                }
                break;
            case MMDVFS_CAMERA_MODE_VFB:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_VFB;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_VFB;
                }
                break;
            case MMDVFS_CAMERA_MODE_EIS_2_0:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_EIS_2_0;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_EIS_2_0;
                }
                break;
            case MMDVFS_CAMERA_MODE_IVHDR:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_IVHDR;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_IVHDR;
                }
                break;
            case MMDVFS_CAMERA_MODE_MVHDR:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_MVHDR;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_MVHDR;
                }
                break;
            case MMDVFS_CAMERA_MODE_ZVHDR:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_ZVHDR;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_ZVHDR;
                }
                break;
            case MMDVFS_CAMERA_MODE_DUAL_ZOOM:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_DUAL_ZOOM;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_DUAL_ZOOM;
                }
                break;
            case MMDVFS_CAMERA_MODE_STEREO:
                cmd->camera_mode &= ~MMDVFS_CAMERA_MODE_FLAG_STEREO;
                if (value)
                {
                    cmd->camera_mode |= MMDVFS_CAMERA_MODE_FLAG_STEREO;
                }
                break;
            case MMDVFS_VENC_SIZE:
                cmd->venc_size = value;
                break;
            case MMDVFS_PREVIEW_SIZE:
                cmd->preview_size = value;
                break;
            default:
                BWC_ERROR("unknown parameter %d", type);
            break;
        }
    }
}

static int mmdvfs_smi_fd = -1;
static int mmdvfs_qos_smi_fd = -1;

mmdvfs_step_enum mmdvfs_query(int scenario, ...)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif
    cmd.camera_mode = MMDVFS_CAMERA_MODE_FLAG_DEFAULT;

    // parse args
    va_list arg_ptr;
    va_start(arg_ptr, scenario);
    mmdvfs_parse_arg(arg_ptr, &cmd);
    va_end(arg_ptr);

    BWC_INFO("MMDVFS QUERY %d\n", cmd.camera_mode);

    cmd.type = MTK_MMDVFS_CMD_TYPE_QUERY;
    cmd.scen = map_bwc_profile_to_smi((BWC_PROFILE_TYPE)scenario);

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;
    }

    return (mmdvfs_step_enum)cmd.ret;
}

/* Macro used to resovling step setting ioctl command */
#define MMDVFS_IOCTL_CMD_STEP_FIELD_LEN (8)
#define MMDVFS_IOCTL_CMD_STEP_FIELD_MASK (0xFF)
#define MMDVFS_IOCTL_CMD_MMCLK_FIELD_MASK (0xFF00)

int mmdvfs_resovle_step_from_cmd(int step_mmclk_cmd) {
	return (step_mmclk_cmd & MMDVFS_IOCTL_CMD_STEP_FIELD_MASK);
}

int mmdvfs_resovle_mmclk_from_cmd(int step_mmclk_cmd) {
	/* Get clk from the command (bit 8-15) */
	return ((step_mmclk_cmd & MMDVFS_IOCTL_CMD_MMCLK_FIELD_MASK) >> MMDVFS_IOCTL_CMD_STEP_FIELD_LEN);
}

int mmdvfs_get_current_mmclk_step(void)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif

    BWC_INFO("MMDVFS Get Current Step\n");

    cmd.type = MTK_MMDVFS_CMD_TYPE_GET;

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;
    }

    return (int) cmd.ret;
}

int mmdvfs_get_current_step(void)
{
		int step_mmclk = mmdvfs_get_current_mmclk_step();

		return mmdvfs_resovle_step_from_cmd(step_mmclk);
}

int mmdvfs_get_current_mmclk(void)
{
		int step_mmclk = mmdvfs_get_current_mmclk_step();

		return mmdvfs_resovle_mmclk_from_cmd(step_mmclk);
}

void mmdvfs_platform_config(int boost_disable, int ddr_type)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif

    BWC_INFO("MMDVFS Get Current Step\n");

    cmd.type = MTK_MMDVFS_CMD_TYPE_CONFIG;
    cmd.boost_disable = boost_disable;
    cmd.ddr_type = ddr_type;

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;
    }

    return;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX_QOS_PARAM_NUM   (5)
static MMDVFS_QOS_PARAMETER* mmdvfs_qos_param[MAX_QOS_PARAM_NUM];

static unsigned int calculate_cam_bw(void)
{
    int i;
    double sum = 0;
    unsigned int result = 0;

    for (i = 0; i < MAX_QOS_PARAM_NUM; i++)
    {
        if (mmdvfs_qos_param[i] == NULL)
            break;
        /* vertical blanking */
        double vb = 1.0;
        unsigned int sensor_fps = mmdvfs_qos_param[i]->sensor_fps;
        double table, ccu_r, ccu, capture, pd, video, yuv, rawi, aa, total;
        bool is_smvr = mmdvfs_qos_param[i]->is_smvr;

        table = ccu = capture = pd = video = yuv = rawi = aa = total = 0;

        if (mmdvfs_qos_param[i]->vb_ratio != 1.0)
            vb = 1.0 / (1.0 - mmdvfs_qos_param[i]->vb_ratio);

        table = ((is_smvr?1000000.0:3000000.0) * sensor_fps * vb);
        ccu_r = ((is_smvr?1500000.0:2000000.0) * MIN(sensor_fps, 30) * vb);
        ccu = (2000000.0 * MIN(sensor_fps, 30) * vb);
        capture = ((is_smvr?0:mmdvfs_qos_param[i]->raw_bbp) *
            mmdvfs_qos_param[i]->sensor_size *
            sensor_fps * vb);
        pd = (mmdvfs_qos_param[i]->pd_size * sensor_fps * vb);
        video = (2.25 * mmdvfs_qos_param[i]->video_size * sensor_fps * vb);
        if (mmdvfs_qos_param[i]->yuv_enable)
            yuv = ((is_smvr?500000.0:5000000.0) * sensor_fps * vb);
        if (mmdvfs_qos_param[i]->rawi_enable)
            rawi = ((is_smvr?0:1.25) * mmdvfs_qos_param[i]->sensor_size *
                sensor_fps * vb);
        if (mmdvfs_qos_param[i]->vb_time != (is_smvr?0.1:0.3))
            aa = (1536 / (mmdvfs_qos_param[i]->vb_time - (is_smvr?0.1:0.3)));

        total = (table + ccu + ccu_r + MAX(capture, aa) + pd + video + yuv + rawi);
        BWC_INFO("cam_id=%d sen_fps=%d sen_size=%d pd=%d video=%d vb_ratio=%f vb_time=%f rawi=%d yuv=%d raw_bbp=%f is_smvr=%d",
            mmdvfs_qos_param[i]->camera_id, sensor_fps, mmdvfs_qos_param[i]->sensor_size, mmdvfs_qos_param[i]->pd_size,
            mmdvfs_qos_param[i]->video_size, mmdvfs_qos_param[i]->vb_ratio, mmdvfs_qos_param[i]->vb_time, mmdvfs_qos_param[i]->rawi_enable,
            mmdvfs_qos_param[i]->yuv_enable, mmdvfs_qos_param[i]->raw_bbp, mmdvfs_qos_param[i]->is_smvr);
        BWC_INFO("cam_id=%d table=%f ccu=%f ccu_r=%f capture=%f aa=%f pd=%f video=%f yuv=%f rawi=%f total=%f\n",
            mmdvfs_qos_param[i]->camera_id, table, ccu, ccu_r, capture, aa, pd, video, yuv, rawi, total);
        sum += total;
    }
    result = sum / 1000000;
    BWC_INFO("cal_cam_bw hrt bw=%d\n", result);
    return result;
}

static void mmdvfs_qos_parse_arg(va_list arg_ptr, int num)
{
    int i, j;

    if (num < 1 || num > MAX_QOS_PARAM_NUM)
      return;

    for (i = 0; i < num; i++)
    {
        MMDVFS_QOS_PARAMETER* param_ptr =
            (MMDVFS_QOS_PARAMETER*) va_arg(
                arg_ptr, MMDVFS_QOS_PARAMETER*);
        unsigned int camera_id = param_ptr->camera_id;
        for (j = 0; j < MAX_QOS_PARAM_NUM; j++)
        {
            if (mmdvfs_qos_param[j] == NULL) {
                mmdvfs_qos_param[j] =
                    (MMDVFS_QOS_PARAMETER*) malloc(
                        sizeof(MMDVFS_QOS_PARAMETER));
                memcpy(mmdvfs_qos_param[j],
                    param_ptr, sizeof(MMDVFS_QOS_PARAMETER));
                break;
            } else if (mmdvfs_qos_param[j]->camera_id == camera_id) {
                memcpy(mmdvfs_qos_param[j],
                    param_ptr, sizeof(MMDVFS_QOS_PARAMETER));
                break;
            }
        }
    }
}

int mmdvfs_qos_set(int num, ...)
{
    MTK_MMDVFS_QOS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif

    // parse args
    va_list arg_ptr;
    va_start(arg_ptr, num);
    mmdvfs_qos_parse_arg(arg_ptr, num);
    va_end(arg_ptr);

    cmd.type = MTK_MMDVFS_QOS_CMD_TYPE_SET;
    cmd.max_cam_bw = calculate_cam_bw();

    if (mmdvfs_qos_smi_fd == -1)
    {
        mmdvfs_qos_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_qos_smi_fd, MTK_IOC_MMDVFS_QOS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_QOS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_qos_smi_fd);
        mmdvfs_qos_smi_fd = -1;

        return -1;
    }

    return (int)cmd.ret;
}

int mmdvfs_set(int scenario, ...)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif
    cmd.camera_mode = g_mmdvfs_camera_mode;

    // parse args
    va_list arg_ptr;
    va_start(arg_ptr, scenario);
    mmdvfs_parse_arg(arg_ptr, &cmd);
    va_end(arg_ptr);

    g_mmdvfs_camera_mode = cmd.camera_mode;

    BWC_INFO("MMDVFS SET %d\n", g_mmdvfs_camera_mode);

    cmd.type = MTK_MMDVFS_CMD_TYPE_SET;
    cmd.scen = map_bwc_profile_to_smi((BWC_PROFILE_TYPE)scenario);

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;

        return -1;
    }

    return (int)cmd.ret;
}

int mmdvfs_direct_mmclk_step_set(int scenario, int step, int mmclk)
{
    MTK_MMDVFS_CMD cmd;
    int step_mmclk_config_cmd = 0;

#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif

    BWC_INFO("MMDVFS STEP SET %d\n", step);

    cmd.type = MTK_MMDVFS_CMD_TYPE_STEP_SET;
    cmd.scen = (MTK_SMI_BWC_SCEN)scenario;

    /* Put step in the command (bit 0-7) */
    step_mmclk_config_cmd = step & MMDVFS_IOCTL_CMD_STEP_FIELD_MASK ;
    /* Put mmclk in the command (bit 8-15) */
    step_mmclk_config_cmd |= (mmclk << MMDVFS_IOCTL_CMD_STEP_FIELD_LEN) & MMDVFS_IOCTL_CMD_MMCLK_FIELD_MASK;

    BWC_INFO("mmdvfs_direct_mmclk_step_set: step=%d, mmclk=%d, config_cmd=0x%x\n", step, step, mmclk);

    cmd.step = step_mmclk_config_cmd;

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;

        return -1;
    }

    return (int)cmd.ret;
}

int mmdvfs_vpu_step_set(int step)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif
    cmd.camera_mode = step;

    BWC_INFO("mmdvfs_vpu_step_set %d\n", cmd.camera_mode);

    cmd.type = MTK_MMDVFS_CMD_TYPE_VPU_STEP_SET;
    cmd.scen = SMI_BWC_SCEN_NORMAL;

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;

        return -1;
    }

    return (int)cmd.ret;
}

int mmdvfs_vpu_step_get(void)
{
    MTK_MMDVFS_CMD cmd;
#ifdef __STDC_LIB_EXT1__
    memset_s(&cmd, sizeof(cmd), 0, sizeof(cmd));
#else
    memset(&cmd, 0, sizeof(cmd));
#endif

    BWC_INFO("mmdvfs_vpu_step_set %d\n", cmd.camera_mode);

    cmd.type = MTK_MMDVFS_CMD_TYPE_VPU_STEP_GET;
    cmd.scen = SMI_BWC_SCEN_NORMAL;

    if (mmdvfs_smi_fd == -1)
    {
        mmdvfs_smi_fd = open("/dev/MTK_SMI", O_RDONLY);
    }

    if (ioctl(mmdvfs_smi_fd, MTK_IOC_MMDVFS_CMD, &cmd) < 0)
    {
        BWC_ERROR("MTK_IOC_MMDVFS_CMD failed.:%s\n" , strerror(errno) );
        close(mmdvfs_smi_fd);
        mmdvfs_smi_fd = -1;

        return -1;
    }

    return (int)cmd.ret;
}

}


