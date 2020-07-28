/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _CCU_CMD_PARAM_INFO_H_
#define _CCU_CMD_PARAM_INFO_H_

/*******************************************************************************
* ARM linux kernel 64 -> CCU 32 compatible
* Make all pointers as 32bit MVA,
* avoiding pointer type compatible problem in kernel driver
********************************************************************************/
namespace NSCcuIf {

//**********************************************************************************
// CcuPtrDescriptor: describes pointed data info. of ptr member in original struct
//**********************************************************************************
struct CcuPtrDescriptor
{
    const bool valid; //true: valid entry, false: means end of ptr description list
    const size_t offset_in_orig_struct; //ptr. member byte size offset in original struct
    const size_t offset_in_compat_struct; //ptr. member byte size offset in compat. struct
    const size_t size; //data chunk size of datas to which ptr member points
};

//**********************************************************************************
// CcuCtrlParamDescriptor: describes ccu_io_control param info.
//**********************************************************************************
struct CcuCtrlParamDescriptor
{
    const bool need_input; //if need input data
    const bool need_output; //if need output data
    const size_t input_non_ptr_total_size; //input data size of non-ptr member part
    const size_t output_non_ptr_total_size; //output data size, have only non-ptr member
    const struct CcuPtrDescriptor *intput_ptr_descs; //list of CcuPtrDescriptor to describe info needed to handle ptr members
    const size_t buffer_size; //total buffer size need for content of ptr member
    const int ptr_count; //ptr count
    const int ring_count; //ring count
    const char *msg_literal;
};

struct CcuCtrlCmdSetInfo
{
	uint32_t msgIdMin;
	uint32_t msgIdMax;
};

class CcuCmdParamInfo
{
public:
    static CcuCtrlParamDescriptor* getParamDescriptor(enum ccu_feature_type featureType, MUINT32 msgId);
    static struct CcuCtrlCmdSetInfo* getCmdSetInfo(enum ccu_feature_type featureType);
};

}; //namespace NSCcuIf

#endif