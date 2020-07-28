/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG "CCAP"


#include "meta_ccap_interface.h"
#include "meta_ccap_para.h"

int intf_ccap_init()
{
    return META_CCAP_init()? 1:0;
}

void intf_ccap_deinit()
{
    META_CCAP_deinit();
}

void intf_ccap_op(const void* localBuf, void* peerBuf, void **out_cnf, unsigned short *out_cnf_len, void **out_peerBuf, unsigned short *out_peerBuf_len)
{
    META_CCAP_OP((FT_CCT_REQ*) localBuf, (char*) peerBuf, out_cnf, out_cnf_len, out_peerBuf, out_peerBuf_len);
}
void intf_ccap_adb_op()
{
    META_CCAP_ADB_OP();
}

void intf_ccap_atci_op(const void* req, void* cnf, const int cct_op_legacy)
{
    META_CCAP_ATCI_OP((FT_CCT_REQ*) req, (FT_CCT_CNF*) cnf, cct_op_legacy);
}

void intf_ccap_const(int *sizeReq, int *sizeCnf, int *opEnd, int *opLcdStart, int *opLcdStop)
{
    if(opEnd){ *opEnd = FT_CCT_OP_END; }
    if(opLcdStart){ *opLcdStart = FT_CCT_OP_SUBPREVIEW_LCD_START; }
    if(opLcdStop){ *opLcdStop = FT_CCT_OP_SUBPREVIEW_LCD_STOP; }
    if(sizeReq){ *sizeReq = sizeof(FT_CCT_REQ); }
    if(sizeCnf){ *sizeCnf = sizeof(FT_CCT_CNF); }
}

void intf_ccap_set_error(const void* _req, void* _cnf)
{
    const FT_CCT_REQ *req = (FT_CCT_REQ*)_req;
    FT_CCT_CNF *cnf = (FT_CCT_CNF*)_cnf;

    cnf->header.id = req->header.id +1;
    cnf->header.token = req->header.token;
    cnf->op= req->op;
    cnf->status = META_FAILED;
}
int intf_ccap_get_req_op(const void* _req)
{
    const FT_CCT_REQ *req = (FT_CCT_REQ*)_req;
    return (req == NULL)? 0:req->op;
}







