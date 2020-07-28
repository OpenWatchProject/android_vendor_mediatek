/*
 * =====================================================================================
 *
 *       Filename:  akm_apis.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2017å¹?6æœ?7æ—?18æ—?0åˆ?8ç§? *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <FreeRTOSConfig.h>
#include <platform_mtk.h>
#include "FreeRTOS.h"
#include "task.h"

#include "akm_apis.h"

// struct AKL_SCL_PRMS *prm = NULL;


#define AKL_SCL_PRMS_SIZE   8192//5256
#define AKL_NV_PRMS_SIZE    64


static uint8_t g_prm[AKL_SCL_PRMS_SIZE]__attribute__((aligned(8)));
static uint8_t g_nvdata[AKL_NV_PRMS_SIZE]__attribute__((aligned(8)));
static float g_offset[3];

#define CONVERT_AKSC_TO_MICROTESLA    (0.06f)

#ifdef CFG_FAST_CALIBRATION_SUPPORT
#define AKM_DATA_FILTER_SIZE   8
int akm_buffer_count = 0;
akm_data_filter_buffer *akm_data_buffer_p = NULL;
static int16_t AKM_InsertDataToBuffer(akm_data_filter_buffer *data_buffer_tmp)
 {
    akm_data_filter_buffer *temp_p = akm_data_buffer_p;
    akm_data_filter_buffer *p = NULL;

    akm_buffer_count ++;
    if (akm_data_buffer_p == NULL)
    {
        akm_data_buffer_p = data_buffer_tmp;
    }
    else
    {
        while (temp_p->next != NULL)
        {
            if ((data_buffer_tmp->data_package.time_ns > temp_p->data_package.time_ns)
                &&(data_buffer_tmp->data_package.time_ns < temp_p->next->data_package.time_ns))
            {
                p = temp_p->next;
                temp_p->next = data_buffer_tmp;
                data_buffer_tmp->next = p;
                p = NULL;
                return AKM_SUCCESS;
            }
            else
            {
                temp_p = temp_p->next;
            }
        }
        if (temp_p->next == NULL)
        {
            if (data_buffer_tmp->data_package.time_ns > akm_data_buffer_p->data_package.time_ns)
            {
                temp_p->next = data_buffer_tmp;
            }
            else
            {
                data_buffer_tmp->next = akm_data_buffer_p;
                akm_data_buffer_p = data_buffer_tmp;
            }
        }
    }
    return AKM_SUCCESS;
 }
static int16_t AKM_GetDataFromBuffer(struct AKM_SENSOR_DATA *data_package)
 {
    akm_data_filter_buffer *data_buffer_temp = NULL;
    memcpy (data_package,&(akm_data_buffer_p->data_package),sizeof(struct AKM_SENSOR_DATA));
    if (akm_buffer_count == AKM_DATA_FILTER_SIZE)
    {
        data_buffer_temp = akm_data_buffer_p->next;
        vPortFree(akm_data_buffer_p);
        akm_data_buffer_p = data_buffer_temp;
        akm_buffer_count -- ;
    }
    return AKM_SUCCESS;
 }
static int16_t AKM_DataFilter(struct AKM_SENSOR_DATA *data_package)
 {
     int16_t ret = -1;
     akm_data_filter_buffer *data_buffer_tmp =
         (akm_data_filter_buffer *) pvPortMalloc (sizeof(akm_data_filter_buffer));
     if(data_buffer_tmp == NULL)
     {
            printf("AKM_DataFilter maollc failed\n");
            return AKM_ERROR;
     }
     memcpy(&(data_buffer_tmp->data_package),data_package,
         sizeof(akm_data_filter_buffer));
     data_buffer_tmp->next = NULL;

     ret = AKM_InsertDataToBuffer(data_buffer_tmp);
     if (ret != AKM_SUCCESS)
     {
         return AKM_ERROR;
     }
    if (akm_buffer_count < AKM_DATA_FILTER_SIZE)
    {
        return AKM_SUCCESS;
    }
     ret = AKM_GetDataFromBuffer(data_package);
     if (ret != AKM_SUCCESS)
     {
         return AKM_ERROR;
     }
     return AKM_SUCCESS;
 }
#endif

 /**
  * @brief A simple implementation of string copy function.
  * This is really simple, so argument check is not done in the function.
  * Wrong parameter may destroy memory area.
  *
  * @param dst A pointer to destination buffer.
  * @param src A pointer to an original string.
  * @param dst_len The size of destination buffer.
  */
 static void string_copy(
     char          dst[],
     const char    src[],
     const int16_t dst_len)
 {
     int16_t i = 0;

     while (src[i] != '\0') {
         dst[i] = src[i];
         i++;

         if (i >= dst_len) {
             i--;
             break;
         }
     }

     dst[i] = '\0';
 }


 /**
  * @brief Set certification information to enable DOE function.
  * This function sets LICENSER and LICENSEE string, then
  * get hardware information from driver.
  *
  * @param info A pointer to #AKL_CERTIFICATION_INFO struct.
  *
  * @return When function succeeds, #AKM_SUCCESS is returned.
  */
 static int16_t AKM_SetCertificationInfo(struct AKL_CERTIFICATION_INFO *info)
 {
     string_copy((char *)info->a_licenser,
                 AKM_CUSTOM_LICENSER, AKL_CI_MAX_CHARSIZE + 1);
     string_copy((char *)info->a_licensee,
                 AKM_CUSTOM_LICENSEE, AKL_CI_MAX_CHARSIZE + 1);

 #if 0
     info->a_key[0] = (int16_t)dev_info.parameter[0];
     info->a_key[1] = (int16_t)dev_info.parameter[1];
     info->a_key[2] = (int16_t)dev_info.parameter[2];
     info->a_key[3] = (int16_t)dev_info.parameter[3];
     info->a_key[4] = (int16_t)dev_info.parameter[4];
 #else
     info->a_key[0] = 0;
     info->a_key[1] = 0x48;
     info->a_key[2] = 0;
     info->a_key[3] = 0;
     info->a_key[4] = 0;
 #endif

     info->a_key[5] = (int16_t)AKM_CUSTOM_MAG_AXIS_ORDER_X;
     info->a_key[6] = (int16_t)AKM_CUSTOM_MAG_AXIS_ORDER_Y;
     info->a_key[7] = (int16_t)AKM_CUSTOM_MAG_AXIS_ORDER_Z;
     info->a_key[8] = (int16_t)AKM_CUSTOM_MAG_AXIS_SIGN_X;
     info->a_key[9] = (int16_t)AKM_CUSTOM_MAG_AXIS_SIGN_Y;
     info->a_key[10] = (int16_t)AKM_CUSTOM_MAG_AXIS_SIGN_Z;

     return AKM_SUCCESS;
 }

 /*! This function load PDC parameter.
  * \param pdc A pointer to PDC parameter array which size should be
  * #AKL_PDC_SIZE. */
#ifdef AKM_ENABLE_PDC
 static void AKM_LoadPDCParameter(uint8_t pdc[AKL_PDC_SIZE])
 {
#if 1
     int16_t i;
     /* UNIT parameter for PDC */
     const uint8_t U_PDC[AKL_PDC_SIZE] = {
         32, 82, 186, 174, 49, 255, 255, 96, 214,
         55, 231, 85, 38, 206, 255, 242, 255, 255,
         127, 154, 191, 252, 255, 255, 9, 38, 255
     };

     /* Set unit parameter */
     for (i = 0; i < AKL_PDC_SIZE; i++) {
         pdc[i] = U_PDC[i];
     }
#else
     AKH_LoadPDC(pdc);
#endif
 }
#endif

 /*! Library initialize function.
  * This function allocate new memory area for #AKL_SCL_PRMS.
  * When this function succeeds, #AKL_SCL_PRMS parameter is ready to
  * start measurement. But please note that this function does
  * not restore previously saved data, for example, offset and its
  * accuracy etc.\n
  * When this function fails, \c prm is cleaned up internally,
  * so you don't need to \c 'free' outside of this function.
  * \return When function succeeds, #AKM_SUCCESS is returned.
  * \param prm A pointer of pointer to #AKL_SCL_PRMS struct. */
// int16_t library_init(struct AKL_SCL_PRMS **prm)
 int16_t AKM_LibraryInit(void)
 {
     struct   AKL_CERTIFICATION_INFO info;
     uint32_t                        prmSz;
     int16_t                         fret;
     struct AKL_SCL_PRMS *prm = (struct AKL_SCL_PRMS *) g_prm;
     AKL_DEVICE_TYPE device = AK09915;
     /* alloc library data parameter */
     prmSz = AKL_GetParameterSize((uint8_t)AKM_CUSTOM_NUM_FORM);
     ALOGE("akm library_init prmsize:%d\n",prmSz);

     if (prmSz == 0U) {
         fret = AKM_ERROR;
         goto EXIT_LIBRARY_INIT;
     }

     memset(prm, 0, (size_t)prmSz);

     /* certification */
     fret = AKM_SetCertificationInfo(&info);

     if (fret != AKM_SUCCESS) {
         goto EXIT_LIBRARY_INIT;
     }

     /* Initialize AKM library. */
     fret = AKL_Init(prm, &info, (uint8_t)AKM_CUSTOM_NUM_FORM, device);

     if (fret != AKM_SUCCESS) {
         goto EXIT_LIBRARY_INIT;
     }

     return AKM_SUCCESS;

 EXIT_LIBRARY_INIT:

     if (prm != NULL) {
         prm = NULL;
     }
     return fret;
 }

void AKM_ReloadContext(float *offset)
{
    g_offset[0] = offset[0];
    g_offset[1] = offset[1];
    g_offset[2] = offset[2];
}
static int16_t AKM_LoadParameter(struct AKL_NV_PRMS *nv_data)
{
    nv_data->magic = 0xdeadbeefU;
    nv_data->va_hsuc_ho.u.x = (int16)(g_offset[0] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hsuc_ho.u.y = (int16)(g_offset[1] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hsuc_ho.u.z = (int16)(g_offset[2] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hflucv_href.u.x = (int16)(g_offset[0] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hflucv_href.u.y = (int16)(g_offset[1] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hflucv_href.u.z = (int16)(g_offset[2] / CONVERT_AKSC_TO_MICROTESLA);
    nv_data->va_hsuc_hbase.u.x = 0;
    nv_data->va_hsuc_hbase.u.y = 0;
    nv_data->va_hsuc_hbase.u.z = 0;
    nv_data->a_hsuc_hdst = AKSC_HDST_L0;
    return 0;
}
 /*! This function restore previous parameter from non-volatile
  * area (for example, EEPROM on MCU, a file on filesystem).
  * If previous parameter is not restored correctly (may be it's
  * collapsed), a default parameter is set.
  * When this function succeeds, it is completely ready to start
  * measurement.
  * \return When function succeeds, #AKM_SUCCESS is returned.
  * \param prm A pointer to #AKL_SCL_PRMS struct. */
 int16_t AKM_LoadAndStart(void)
 {
     uint16_t nvSz;
     int16_t  fret = -1;
#ifdef AKM_ENABLE_PDC
     uint8_t pdc[AKL_PDC_SIZE];
#endif
     struct AKL_SCL_PRMS *prm = (struct AKL_SCL_PRMS *) g_prm;
     struct AKL_NV_PRMS *nv_data = (struct AKL_NV_PRMS *) g_nvdata;

     /* alloc nv data area */
     nvSz = AKL_GetNVdataSize((uint8_t)AKM_CUSTOM_NUM_FORM);
     ALOGE("akm library_init nvdata size:%d\n",nvSz);

     if (nvSz == 0U) {
         nv_data = NULL;
         fret = AKM_ERROR;
         goto LOAD_QUIT;
     }


     /* load data from storage */
     if (nv_data != NULL) {
         fret = AKM_LoadParameter(nv_data);

         if (fret != AKM_SUCCESS) {
             nv_data = NULL;
         }
     }

     /* AKL can accept NULL pointer for nv_data */
     fret = AKL_StartMeasurement(prm, (uint8_t *)nv_data);

     if (fret != AKM_SUCCESS) {
         goto LOAD_QUIT;
     }

#ifdef AKM_ENABLE_PDC
     /* Use customized PDC parameter */
     AKM_LoadPDCParameter(pdc);
     fret = AKL_SetPDC(prm, pdc, 0U);
#endif

 LOAD_QUIT:


     return fret;
 }

 /*! This function save parameter to non-volatile area for next
  * measurement. If this function fails, next measurement will
  * start with a default parameters.
  * \return When function succeeds, #AKM_SUCCESS is returned.
  * \param prm A pointer to #AKL_SCL_PRMS struct. */
 int16_t AKM_StopAndSave(void)
 {
     int16_t  fret;
     struct AKL_SCL_PRMS *prm = (struct AKL_SCL_PRMS *) g_prm;

     /* AKL can accept NULL pointer for nv_data */
     fret = AKL_StopMeasurement(prm, NULL);

     return fret;
 }

 int16_t AKM_SetData(struct AKM_SENSOR_DATA *data)
 {
    int16_t ret = -1;
     struct AKL_SCL_PRMS *prm = (struct AKL_SCL_PRMS *) g_prm;
     if (data == NULL)
     {
         ALOGE("akm %s,%d  data == NULL\n",__FILE__,__LINE__);
         return ret;
     }
     #ifdef CFG_FAST_CALIBRATION_SUPPORT
     if ((data->stype == AKM_ST_MAG) || (data->stype == AKM_ST_GYR))
     {
        ret = AKM_DataFilter(data);
     }
     #endif
    ret = AKL_SetVector(prm,data,1);
    if (data->stype == AKM_ST_FUSION_MAG)
    {
        AKL_CalcFusion(prm);
    }
    return ret;
 }
 int16_t AKM_GetData(int32_t result[6], int senor_type, int *accuracy)
 {
    int64_t time_stamp;
    int16_t ret = 1;
    struct AKL_SCL_PRMS *prm = (struct AKL_SCL_PRMS *) g_prm;
    switch(senor_type)
    {
        case AKM_VT_ORI:
        case AKM_VT_GRAVITY:
        case AKM_VT_LACC:
        case AKM_VT_QUAT:
#ifdef CFG_AKM_PG_SUPPORT
        case AKM_VT_GYR:
#endif
        case AKM_VT_MAG_DOEAG:
        case AKM_VT_MAG:

            break;
        default:
            break;
    }
    ret = AKL_GetVector(senor_type,prm,result,AKM_VT_MAG_SIZE,(int32_t *)accuracy,&time_stamp);
    if (ret != 0)
    {
        ALOGE("AKL_GetData ERROR:%d\n",ret);
    }
    return ret;
 }
