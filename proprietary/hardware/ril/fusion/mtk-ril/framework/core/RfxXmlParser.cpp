/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include "RfxXmlParser.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RfxXmlParser"

RfxXmlParser::~RfxXmlParser() {
}

void RfxXmlParser::parse(DefaultHandler* parsedData, string xmlData) {
    TiXmlDocument *myDoc = new TiXmlDocument();
    myDoc->Parse(xmlData.c_str());
    TiXmlElement *rootElement = myDoc->RootElement();
    parseSubElement(rootElement, parsedData);
    delete myDoc;
}

void RfxXmlParser::parseSubElement(TiXmlElement *subElement, DefaultHandler* parsedData) {
    if (NULL == subElement) {
        return;
    }
    string nodeName = "";
    string attributeName = "";
    string attributeValue = "";
    string nodeValue = "";

    TiXmlAttribute *subAttribute = subElement->FirstAttribute();
    //handle node without attribute.
    if (subAttribute == 0) {
        nodeName = subElement->Value();
        nodeValue = "";
        parsedData->startElement(nodeName, nodeValue, attributeName, attributeValue);
    }
    //
    while (subAttribute) {
        nodeName = subElement->Value();
        nodeValue = "";
        attributeName = subAttribute->Name();
        attributeValue = subAttribute->Value();
        parsedData->startElement(nodeName, nodeValue, attributeName, attributeValue);
        subAttribute = subAttribute->Next();
    }

    if (NULL != subElement->FirstChildElement()) {
        parseSubElement((TiXmlElement *)subElement->FirstChild(), parsedData);
    } else {
        if (0 != subElement->GetText()) {
            nodeValue = subElement->GetText();
        } else {
            nodeValue = "";
        }
        nodeName = subElement->Value();
        attributeName = "";
        attributeValue = "";
        parsedData->startElement(nodeName, nodeValue, attributeName, attributeValue);
    }

    nodeName = subElement->Value();
    parsedData->endElement(nodeName);

    if (subElement->NextSiblingElement()) {
        parseSubElement(subElement->NextSiblingElement(), parsedData);
    }
}
