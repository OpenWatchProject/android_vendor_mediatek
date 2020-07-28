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

#include <stdio.h>
#include "meta_adc.h"

// ---------------------------------------------------------------------------

int main(int argc, const char** args)
{
   	printf("Meta Test ADC AP test : START\n");

	if (false == Meta_AUXADC_Init())
    {
        printf("Meta_AUXADC_Init() fail\n");
        return -1;
    }


	ADC_REQ *req;
	char peer_buff[100];
	unsigned short peer_len = 8;
	int i;

	for (i=3;i>0;i--)
	{
		req = (ADC_REQ*)malloc(sizeof(ADC_REQ));
		req->header.token = 1;
		req->header.id = 1;
		req->type = ADC_OP_GET_CAR_TUNE_VALUE;
		req->req.dummy = 0;

		Meta_ADC_OP(req, peer_buff, peer_len);
		free(req);

		req = (ADC_REQ*)malloc(sizeof(ADC_REQ));
		req->header.token = 5;
		req->header.id = 5;
		req->type = ADC_OP_SET_CAR_TUNE_VALUE;
		req->req.dummy = 1000 + 100 * i;

		Meta_ADC_OP(req, peer_buff, peer_len);
		free(req);
	}
	/*
	if (argc > peer_len)
		if (peer_buff[0] != peer_buff[1])
			if (*argv != NULL)
				peer_len = 9;
	*/

	if (false == Meta_AUXADC_Deinit())
    {
        printf("Meta_AUXADC_Deinit() fail\n");
		
        return -1;
    }

	printf("Meta Test ADC AP test : END\n");
	
    return 0;
}
