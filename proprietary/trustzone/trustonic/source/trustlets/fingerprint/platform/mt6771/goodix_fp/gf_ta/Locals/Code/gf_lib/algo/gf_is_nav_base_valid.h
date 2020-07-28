/***************************************************************************
* Copyright (C), www.goodix.com
* Description:     Oswego 的导航功能
* Function List:
* 1.  实现 Milan C 的导航功能
* History:
2016/09/13 - zhoudewen - 增加对坏点的判断
***************************************************************************/
#ifndef MOVEDETECT_ALGORITHM_H_
#define MOVEDETECT_ALGORITHM_H_ 610201

#ifdef __cplusplus
extern "C"
{
#endif
    typedef int    int32_nvg;
    typedef unsigned int       uint32_nvg;

    typedef unsigned short     uint16_nvg;
    typedef unsigned short     NVG_RAWDATA_TYPE; //rawdata data type

    typedef short      int16_nvg;
    typedef char       int8_nvg;

#ifndef NAVI_INT32_MAX
#define NAVI_INT32_MAX 0x7FFFFFFF
#endif

    //输出的状态
    typedef enum NVG_RESULT_
    {
        NVG_RESULT_NONE = 0,      //无,输入的起始状态
        NVG_RESULT_LEFT,      //左
        NVG_RESULT_RIGHT,      //右
        NVG_RESULT_UP,      //上
        NVG_RESULT_DOWN,      //下
        NVG_RESULT_CLICK,      //单击
        NVG_RESULT_TOUCH_SMALL,      //小范围接触
        NVG_RESULT_FAST,      //划动（单击）太快
        NVG_RESULT_WRONG,      //错误划动的方向（设定上下划,而进行左右滑动等）
        NVG_RESULT_NO_REASON,      //未识别
        NVG_RESULT_SMALL_UP,       // 还没有用到,预留
        NVG_RESULT_SMALL_DOWN,    // 还没有用到,预留
        NVG_RESULT_SMALL_LEFT,    // 还没有用到,预留
        NVG_RESULT_SMALL_RIGHT,    // 还没有用到,预留
        NVG_RESULT_SMALL_UP_LEFT,    // 还没有用到,预留
        NVG_RESULT_SMALL_UP_RIGHT,    // 还没有用到,预留
        NVG_RESULT_SMALL_DOWN_LEFT,    // 还没有用到,预留
        NVG_RESULT_SMALL_DOWN_RIGHT,    // 还没有用到,预留

        NVG_RESULT_EXCEPTION = NAVI_INT32_MAX    /* make sure enum takes 4 bytes */
    }EM_NVG_RESULT;

    //SENSOR的类别
    typedef enum SENSOR_TYPE_
    {
        NVG_SENSOR_MILAN_A = 1, //MILAN A
        NVG_SENSOR_MILAN_B,//MILAN B
        NVG_SENSOR_MILAN_C,//MILAN C
        SENSOR_TYPE_EXCEPTION = NAVI_INT32_MAX    /* make sure enum takes 4 bytes */
    }EM_NVG_SENSOR_TYPE;

    //导航判别的方向
    typedef enum NVG_MODE_
    {
        ALL_DIRECTIONS = 1,//全方向
        HOR_DIRECTIONS,//水平方向
        VER_DIRECTIONS,//竖直方向
        NVG_MODE_EXCEPTION = NAVI_INT32_MAX    /* make sure enum takes 4 bytes */
    }EM_NVG_MODE;

    //手势的类型
    typedef enum NVG_STATUS_
    {
        NVG_NORMAL_FRAME = 0,    //normal frame
        NVG_PRESS_UP = 2,//抬手
        NVG_PRESS_DOWN = 3,//按压
        NVG_FIRST_FRAME = 4,    //没有用到,预留
        NVG_LAST_FRAME = 5,    //没有用到,预留
        NVG_STATUS_EXCEPTION = NAVI_INT32_MAX    /* make sure enum takes 4 bytes */
    }EM_NVG_STATUS;

    //系统的参数
    typedef struct NVG_CONFIG_
    {
        EM_NVG_MODE  emMode;//navigate mode 1: 4 directions 2:horizontal 3:vertical
        EM_NVG_SENSOR_TYPE emSensorType;// sensor type
        int32_nvg nCoverType;// sensor covertype, The default is 0

        int32_nvg nInertiaX; /* 左右方向的迟钝程度,默认为1,值越大越迟钝 */
        int32_nvg nInertiaY; /* 上下方向的迟钝程度,默认为1,值越大越迟钝 */
        int32_nvg nStaticX; /* 判断两帧为静止的SAD移动X方向阈值,此阈值应小于等于inertiaX,阈值越大,越容易判断为静止 */
        int32_nvg nStaticY; /* 判断两帧为静止的SAD移动Y方向阈值,此阈值应小于等于inertiaY,阈值越大,越容易判断为静止*/

        int32_nvg nSumSadXoffThr;/*总移动判为移动的X方向阈值*/
        int32_nvg nSumSadYoffThr;/*总移动判为移动的Y方向阈值*/

        int32_nvg nMaxNvgFrameNum;/* 导航输出结果前的最大帧数,默认为20 */

        int32_nvg nTouchThreshold; //按压的最小有效阈值,由emSensorType和nCoverType决定
        int32_nvg nCenterNumThre;  //计算中心法时,认为该中心值有效的最小阈值
        int32_nvg nFastTouchSensitivity; //是否进行挽救判断标志

        int32_nvg nSadMoveNumThre;  //判断为划动的最小SAD数量
        int32_nvg nSadClickNumThre;  //判断为单击的最小SAD数量

        int32_nvg nCenterMoveXYThre;    //中心偏移量判断为滑动的阈值

        void *pReservedPtr;// reserved point
    }ST_NVG_CONFIG;// sensor && mode && debug information

    //导航输入的类型
    typedef struct NVG_INPUT_
    {
        NVG_RAWDATA_TYPE * pusCurrRaw;//指向当前的数据
        const NVG_RAWDATA_TYPE * pusPrevRaw;//指向前一帧的数据,预留
        const NVG_RAWDATA_TYPE * pusBase;//指向Base的数据
        EM_NVG_STATUS emNvgStatus;//手指按压状态        
        void * pReservedPtr;// reserved point
    } ST_NVG_INPUT;

    void MilanASeriesInitNvgConfig(ST_NVG_CONFIG * pstNvgConfig,
        EM_NVG_SENSOR_TYPE emSensorType);

    const int8_nvg * MilanASeriesGetNvgVersion(void);

    //每次传输1帧数据
    void MilanASeriesNvgMoveDetect(ST_NVG_INPUT * pstNvgInput,
        EM_NVG_RESULT * pemResult,
        const ST_NVG_CONFIG * pkstNvgConfig);

    //输出的类型
    typedef enum
    {
        NVG_BASE_INVALID = 0,//PCdata不可作为基准PCdata      
        NVG_BASE_VALID = 1,//PCdata可以作为基准PCdata    
        NVG_BASE_PARA_ERR = 2,//输入参数错误
        NVG_BASE_PARA__EXCEPTION = NAVI_INT32_MAX,/* make sure enum takes 4 bytes */
    }EM_NVG_BASE_RET_TYPE;

    //输出的类型
    typedef enum
    {
        NVG_CHOOSE_BASE = 0,//PCdata不可作为基准PCdata      
        NVG_CHOOSE_RAW = 1,//PCdata可以作为基准PCdata    
        NVG_CHOOSE_PARA_ERR = 2,//输入参数错误
        NVG_CHOOSE_PARA__EXCEPTION = NAVI_INT32_MAX,/* make sure enum takes 4 bytes */
    }EM_NVG_CHOOSE_BASE_TYPE;

    //isbasevalid的输入
    typedef struct NVG_BASE_DATA_
    {
        int32_nvg nTemperature;    //采集 puwData 中PCdata时的温度计检测值
        int32_nvg nRowNum;    //puwData 中PCdata共有多少行
        int32_nvg nColNum;    //puwData 中PCdata共有多少列
        const NVG_RAWDATA_TYPE *pusData;    //指向存放PCdata空间的指针
        EM_NVG_SENSOR_TYPE emSensorType;//芯片型号
        const uint16_nvg * pkusMicroAdjustDac;  //微调标志，指向Milan C Image的微调标志位
        void *pReservedPtr;// reserved point
    }ST_NVG_BASE_DATA;

    int32_nvg NvgGetRawDataConsistency(const NVG_RAWDATA_TYPE * pkusRawData);
    int32_nvg NvgGetRawDataMean(const NVG_RAWDATA_TYPE * pkusRawData);

    EM_NVG_BASE_RET_TYPE IsNaviBaseValid(ST_NVG_BASE_DATA *pstBase, const ST_NVG_BASE_DATA *pkstRawData);
    EM_NVG_CHOOSE_BASE_TYPE ChooseNaviValidBase(ST_NVG_BASE_DATA *pstBase, const ST_NVG_BASE_DATA *pkstRawData);       
    
#ifdef __cplusplus
}
#endif

#endif /* MOVEDETECT_ALGORITHM_H_ */