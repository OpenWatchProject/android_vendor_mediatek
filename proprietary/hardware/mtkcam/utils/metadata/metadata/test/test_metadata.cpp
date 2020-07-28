/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

#define LOG_TAG "TestMetadata"
#include <mtkcam/utils/metadata/IMetadata.h>

#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>
#include <utils/Vector.h>

#include <gtest/gtest.h>
#include <stdio.h>
#include <thread>

using namespace NSCam;
using namespace android;

/*******************************************************************************
*  helper function
********************************************************************************/
#ifdef MTKCAM_METADATA_V2
void verifyFlatten(const IMetadata &m)
{
    std::vector<MUINT8> buf,buf1;

    IMetadata m1;

    auto size = m.flattenSize();
    buf.resize(size);
    auto result = m.flatten(buf.data(), size);

    // flatten should the same as flattenSize
    EXPECT_EQ(size, (size_t) result);
    // unflatten should use same size
    EXPECT_EQ(size, (size_t) m1.unflatten(buf.data(), size));

    // flatten again
    EXPECT_EQ(size, m1.flattenSize());
    buf1.resize(size);
    EXPECT_EQ(size, (size_t) m1.flatten(buf1.data(), size));
    // two flatten buffer should the same
    EXPECT_TRUE( memcmp(buf.data(), buf1.data(), size)==0);
}
void verifyWhiteList(const IMetadata &m)
{
    auto cnt = m.count();
    auto WN = (size_t) std::rand() % cnt;
    if(cnt ==0) {WN =0;}
    auto BN = cnt - WN;

    //META_LOGD("cnt %d WN %d BN %d",(int)cnt,(int)WN, (int)BN);

    std::vector<IMetadata::Tag_t> B;
    std::vector<IMetadata::Tag_t> W;

    // collect all tag into Black list
    //META_LOGD("all tag:");
    for(int i=0;i<(int)cnt;i++) {
        auto tag = m.entryAt((size_t)i).tag();
        //META_LOGD("=>tag %X",(int)tag);
        B.push_back(tag);
    }

    // Random filter out WN items from Black list to White list
    //META_LOGD("WhiteList:");
    for(int i=0;i<(int)WN;i++) {
        auto pos = (size_t)(std::rand() % B.size());
        auto tag = B[pos];
        W.push_back(tag);
        //META_LOGD("=>W %X",(int)tag);
        B.erase(B.begin() + pos);
    }
    //META_LOGD("BlackList:");
    //for(auto t=B.cbegin(); t != B.end();t++) {
    //    META_LOGD("=>B %X", (int)*t);
    //}

    // sort the Whitelist
    std::sort(W.begin(), W.end());

    IMetadata wm, bm;
    // filter white list to wm
    {
        std::vector<MUINT8> buf;
        buf.resize( m.flattenSize(W.data(),WN));
        m.flatten(buf.data(), buf.size(), W.data(), WN);
        wm.unflatten(buf.data(), buf.size());

        //META_LOGD("wm cnt %d", (int)wm.count());
    }

    // filter black list to bm
    {
        std::vector<MUINT8> buf;
        buf.resize( m.flattenSize(B.data(),BN));
        m.flatten(buf.data(), buf.size(), B.data(), BN);

        bm.unflatten(buf.data(), buf.size());

        //META_LOGD("bm cnt %d", (int)bm.count());
    }

    // check white list all in wm
    {
        //META_LOGD("check white list all in wm ");
        for(auto t = W.cbegin(); t != W.cend(); t++) {
            auto entry = wm.entryFor(*t);
            //META_LOGD("..%X vs %X", (int)*t, (int)entry.tag());
            EXPECT_TRUE ( entry.tag() != IMetadata::IEntry::BAD_TAG);
        }
    }

    // check black list all not in wm
    {
        //META_LOGD("check black list all not in wm ");
        for(auto t = B.cbegin(); t != B.cend(); t++) {
            auto entry = wm.entryFor(*t);
            //META_LOGD("..%X vs %X (cmp%d)", (int)*t, (int)entry.tag(),(int)(entry.tag() == IMetadata::IEntry::BAD_TAG));
            EXPECT_TRUE ( entry.tag() == IMetadata::IEntry::BAD_TAG);
        }
    }

    // filter out all wm data, mm should not contains any whitelist item
    {
        IMetadata mm = m;
        // remove wm's tag
        auto wn = wm.count();
        for(size_t pos = 0;pos < wn;pos++) {
            auto tag = wm.entryAt(pos).tag();
            auto entry = mm.takeEntryFor(tag);
            EXPECT_TRUE ( entry.tag() != IMetadata::IEntry::BAD_TAG); // it should have data
        }

        // now mm should no white list item
        for(auto t = W.cbegin(); t != W.cend(); t++) {
            auto entry = mm.entryFor(*t);
            EXPECT_TRUE ( entry.tag() == IMetadata::IEntry::BAD_TAG); // it should no data
        }

        // now mm should all black list item
        for(auto t = B.cbegin(); t != B.cend(); t++) {
            auto entry = mm.entryFor(*t);
            //META_LOGD("..%X vs %X (cmp%d)", (int)*t, (int)entry.tag(),(int)(entry.tag() != IMetadata::IEntry::BAD_TAG));
            EXPECT_TRUE ( entry.tag() != IMetadata::IEntry::BAD_TAG); // it should have data
        }
    }
}
#endif
/*******************************************************************************
*  Main Function
********************************************************************************/
TEST(CHECK_LOG, EmptyMetadataContent) {
    META_LOGD("CHECK_LOG, EmptyMetadataContent begin===============");
    IMetadata m;
    IMetadata::IEntry e;

    e.push_back(m, Type2Type<IMetadata>()); // put empty metadata

    m.update(123,e); // put into metadata

    e = m.entryFor(123); // query it

    EXPECT_TRUE(e.tag() != IMetadata::IEntry::BAD_TAG); // it should exists

    m = e.itemAt(0,Type2Type<IMetadata>()); // it should not error log

    META_LOGD("CHECK_LOG, EmptyMetadataContent end===============");
}

TEST(IEntry, TYPE_MUINT8) {
    META_LOGD("IEntry, TYPE_MUINT8 begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back((MUINT8)i, Type2Type< MUINT8 >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MUINT8) i,e1.itemAt(i, Type2Type< MUINT8 >()));
            e2.replaceItemAt(i, 100+i, Type2Type< MUINT8 >());
        }
        EXPECT_EQ((MUINT8) 0,e1.itemAt(5, Type2Type< MUINT8 >())); // out of boundary should be 0

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MUINT8)i,e1.itemAt(0, Type2Type< MUINT8 >()));
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++)
            EXPECT_EQ((MUINT8) (100+i),e2.itemAt(i, Type2Type< MUINT8 >()));
    }
    META_LOGD("IEntry, TYPE_MUINT8 end===============");
}
TEST(IEntry, TYPE_MINT32) {
    META_LOGD("IEntry, TYPE_MINT32 begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back((MINT32)i, Type2Type< MINT32 >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MINT32) i,e1.itemAt(i, Type2Type< MINT32 >()));
            e2.replaceItemAt(i, 100+i, Type2Type< MINT32 >());
        }
        EXPECT_EQ((MINT32) 0,e1.itemAt(5, Type2Type< MINT32 >())); // out of boundary should be 0

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MINT32)i,e1.itemAt(0, Type2Type< MINT32 >()));
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++)
            EXPECT_EQ((MINT32) (100+i),e2.itemAt(i, Type2Type< MINT32 >()));
    }
    META_LOGD("IEntry, TYPE_MINT32 end===============");
}
TEST(IEntry, TYPE_FLOAT) {
    META_LOGD("IEntry, TYPE_FLOAT begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back((MFLOAT)i, Type2Type< MFLOAT >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MFLOAT) i,e1.itemAt(i, Type2Type< MFLOAT >()));
            e2.replaceItemAt(i, 100+i, Type2Type< MFLOAT >());
        }
        EXPECT_EQ((MFLOAT) 0,e1.itemAt(5, Type2Type< MFLOAT >())); // out of boundary should be 0

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MFLOAT)i,e1.itemAt(0, Type2Type< MFLOAT >()));
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++)
            EXPECT_EQ((MFLOAT) (100+i),e2.itemAt(i, Type2Type< MFLOAT >()));
    }
    META_LOGD("IEntry, TYPE_FLOAT end===============");
}
TEST(IEntry, TYPE_MINT64) {
    META_LOGD("IEntry, TYPE_MINT64 begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back((MINT64)i, Type2Type< MINT64 >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MINT64) i,e1.itemAt(i, Type2Type< MINT64 >()));
            e2.replaceItemAt(i, 100+i, Type2Type< MINT64 >());
        }
        EXPECT_EQ((MINT64) 0,e1.itemAt(5, Type2Type< MINT64 >())); // out of boundary should be 0

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MINT64)i,e1.itemAt(0, Type2Type< MINT64 >()));
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++)
            EXPECT_EQ((MINT64) (100+i),e2.itemAt(i, Type2Type< MINT64 >()));
    }
    META_LOGD("IEntry, TYPE_MINT64 end===============");
}
TEST(IEntry, TYPE_MDOUBLE) {
    META_LOGD("IEntry, TYPE_MDOUBLE begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back((MDOUBLE)i, Type2Type< MDOUBLE >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MDOUBLE) i,e1.itemAt(i, Type2Type< MDOUBLE >()));
            e2.replaceItemAt(i, 100+i, Type2Type< MDOUBLE >());
        }
        EXPECT_EQ((MDOUBLE) 0,e1.itemAt(5, Type2Type< MDOUBLE >())); // out of boundary should be 0

        for(int i=0;i<5;i++) {
            EXPECT_EQ((MDOUBLE)i,e1.itemAt(0, Type2Type< MDOUBLE >()));
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++)
            EXPECT_EQ((MDOUBLE) (100+i),e2.itemAt(i, Type2Type< MDOUBLE >()));
    }
    META_LOGD("IEntry, TYPE_MDOUBLE end===============");
}
TEST(IEntry, TYPE_MRotional) {
    META_LOGD("IEntry, TYPE_Rotational begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back(MRational((MINT32)i,(MINT32)i), Type2Type< MRational >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MRational >()).numerator);
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MRational >()).denominator);
            e2.replaceItemAt(i, MRational((MINT32)(i+100),(MINT32)(i+200)), Type2Type< MRational >());
        }
        EXPECT_EQ(0,(int)e1.itemAt(5, Type2Type< MRational >()).numerator); // out of boundary error handle

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MRational >()).numerator);
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MRational >()).denominator);
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++) {
            EXPECT_EQ(100+i,(int)e2.itemAt(i, Type2Type< MRational >()).numerator);
            EXPECT_EQ(200+i,(int)e2.itemAt(i, Type2Type< MRational >()).denominator);
        }
    }
    META_LOGD("IEntry, TYPE_Rotational end===============");
}
TEST(IEntry, TYPE_MPoint) {
    META_LOGD("IEntry, TYPE_MPoint begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back(MPoint((int)i,(int)i), Type2Type< MPoint >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MPoint >()).x);
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MPoint >()).y);
            e2.replaceItemAt(i, MPoint((int)(i+100),(int)(i+200)), Type2Type< MPoint >());
        }
        EXPECT_EQ(0,(int)e1.itemAt(5, Type2Type< MPoint >()).x); // out of boundary error handle

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MPoint >()).x);
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MPoint >()).y);
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++) {
            EXPECT_EQ(100+i,(int)e2.itemAt(i, Type2Type< MPoint >()).x);
            EXPECT_EQ(200+i,(int)e2.itemAt(i, Type2Type< MPoint >()).y);
        }
    }
    META_LOGD("IEntry, TYPE_MPoint end===============");
}
TEST(IEntry, TYPE_MSize) {
    META_LOGD("IEntry, TYPE_MSize begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back(MSize((int)i,(int)i), Type2Type< MSize >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MSize >()).w);
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MSize >()).h);
            e2.replaceItemAt(i, MSize((int)(i+100),(int)(i+200)), Type2Type< MSize >());
        }
        EXPECT_EQ(0,(int)e1.itemAt(5, Type2Type< MSize >()).w); // out of boundary error handle

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MSize >()).w);
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MSize >()).h);
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++) {
            EXPECT_EQ(100+i,(int)e2.itemAt(i, Type2Type< MSize >()).w);
            EXPECT_EQ(200+i,(int)e2.itemAt(i, Type2Type< MSize >()).h);
        }
    }
    META_LOGD("IEntry, TYPE_MSize end===============");
}
TEST(IEntry, TYPE_MRect) {
    META_LOGD("IEntry, TYPE_MRect begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<2;j++) {
        for(int i=0;i<5;i++) {
            e1.push_back(MRect(MPoint((int)i,(int)i+1),MSize((int)i+2,(int)i+3)), Type2Type< MRect >());
            EXPECT_FALSE(e1.isEmpty());
            EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int) e1.itemAt((int)i,Type2Type< MRect >()).p.x);
            EXPECT_EQ(i+1,(int) e1.itemAt((int)i,Type2Type< MRect >()).p.y);
            EXPECT_EQ(i+2,(int) e1.itemAt((int)i,Type2Type< MRect >()).s.w);
            EXPECT_EQ(i+3,(int) e1.itemAt((int)i,Type2Type< MRect >()).s.h);
            e2.replaceItemAt(i, MRect(MPoint((int)(i+100),(int)(i+200)),MSize((int)(i+300),(int)(i+400))), Type2Type< MRect >());
        }
        EXPECT_EQ(0,(int)e1.itemAt(5, Type2Type< MRect >()).p.x); // out of boundary error handle
        EXPECT_EQ(0,(int)e1.itemAt(5, Type2Type< MRect >()).s.w); // out of boundary error handle

        for(int i=0;i<5;i++) {
            EXPECT_EQ(i,(int)e1.itemAt(0, Type2Type< MRect >()).p.x);
            EXPECT_EQ(i+1,(int)e1.itemAt(0, Type2Type< MRect >()).p.y);
            EXPECT_EQ(i+2,(int)e1.itemAt(0, Type2Type< MRect >()).s.w);
            EXPECT_EQ(i+3,(int)e1.itemAt(0, Type2Type< MRect >()).s.h);
            e1.removeAt(0);
        }
        EXPECT_TRUE(e1.isEmpty());
        EXPECT_EQ((MUINT)5,e2.count());

        for(int i=0;i<5;i++) {
            EXPECT_EQ(100+i,(int)e2.itemAt(i, Type2Type< MRect >()).p.x);
            EXPECT_EQ(200+i,(int)e2.itemAt(i, Type2Type< MRect >()).p.y);
            EXPECT_EQ(300+i,(int)e2.itemAt(i, Type2Type< MRect >()).s.w);
            EXPECT_EQ(400+i,(int)e2.itemAt(i, Type2Type< MRect >()).s.h);
        }
    }
    META_LOGD("IEntry, TYPE_MRect end===============");
}
TEST(IEntry, TYPE_Memory) {
    META_LOGD("IEntry, TYPE_Memory begin===============");
    IMetadata::IEntry e1;
    IMetadata::IEntry e2;
    EXPECT_TRUE(e1.isEmpty());

    for(int j=0;j<5;j++) {
        for(int i=0;i<5;i++) {
            IMetadata::Memory mem;
            for(int d=0;d<8192;d++) {
                mem.resize(d+1);
                MUINT8 *c = mem.editArray();
                c[d] = d+i;
            }

            e1.push_back(mem, Type2Type< IMetadata::Memory >());
            EXPECT_FALSE(e1.isEmpty());
            //EXPECT_EQ((int)(i+1),(int)e1.count());
        }

        e2 = e1;

        for(int i=0;i<5;i++) {
            IMetadata::Memory &&mem = e1.itemAt(i, Type2Type< IMetadata::Memory >());
            EXPECT_EQ(8192, (int)mem.size());
            MUINT8* c = mem.editArray();
            EXPECT_NE(nullptr, c);

            for(int d =0;d<8192;d++) {
                EXPECT_EQ((MUINT8) (d+i),(MUINT8)c[d]);
            }
            //replace wrong type, should not increase count
            {
                auto oldCount = e2.count();
                e2.replaceItemAt(i, 100+i, Type2Type< MUINT8 >());
                EXPECT_EQ((int)oldCount,(int)e2.count());
            }

            //insert wrong type, should not increase count
            {
                auto oldCount = e2.count();
                e2.push_back(10.5, Type2Type<MFLOAT> ());
                EXPECT_EQ((int)oldCount,(int)e2.count());
            }

            //Compile error : insert correct type, with wrong object
            //{
            //    auto oldCount = e2.count();
            //    e2.push_back(10.5, Type2Type<IMetadata::Memory> ());
            //    EXPECT_EQ(oldCount,(int)e2.count());
            //}
        }
        {
            auto v  = e1.itemAt(5, Type2Type< IMetadata::Memory >()); // out of boundary should be 0
            EXPECT_EQ((int) 0,(int)v.size()); // out of boundary should be 0
        }

        for(int i=0;i<5;i++) {
            auto mem = e1.itemAt(0, Type2Type< IMetadata::Memory>());
            e1.removeAt(0);
            EXPECT_EQ((MUINT8) i, (MUINT8)mem.editArray()[0]);
        }

        EXPECT_TRUE(e1.isEmpty());
        EXPECT_FALSE(e2.isEmpty());

        {
            auto oldCount = e2.count();
            for(int i=0;i<5;i++) {
                auto mem = e2.itemAt(i, Type2Type<IMetadata::Memory>());
                mem.editArray()[i] = (MUINT8)(100+i);
                e2.replaceItemAt(i,mem, Type2Type<IMetadata::Memory>());
                EXPECT_EQ((int) oldCount, (int)e2.count()); // replace should not change count
            }
            e1 = e2;
            for(int i=0;i<5;i++) {
                auto mem = e1.itemAt(i, Type2Type<IMetadata::Memory>());
                EXPECT_EQ((MUINT8)(100+i),(MUINT8)(mem.editArray()[i]));
            }
            e1.clear();
            EXPECT_TRUE(e1.isEmpty());
        }
    }
    META_LOGD("IEntry, TYPE_Memory end===============");
}
TEST(IEntry, Case1) {
    //construction
    IMetadata::IEntry entryA(MTK_SCALER_CROP_REGION);

    //copy construction
    IMetadata::IEntry entryB(entryA);

    //assignmenet operator
    IMetadata::IEntry entryC = entryA;

    //add value
    MRect regionA(MPoint(2,3), MSize(4,5));
    MRect regionB(MPoint(6,7), MSize(8,9));
    MRect regionC(MPoint(10,11), MSize(12,13));

    entryA.push_back(regionA, Type2Type< MRect >());
    entryB.push_back(regionB, Type2Type< MRect >());
    entryC.push_back(regionC, Type2Type< MRect >());

    //copy constructor
    IMetadata::IEntry entryD(entryA);

    //read value
    EXPECT_EQ(entryA.itemAt(0, Type2Type< MRect >()), regionA);
    EXPECT_EQ(entryD.itemAt(0, Type2Type< MRect >()), entryA.itemAt(0, Type2Type< MRect >()));
    EXPECT_EQ(entryB.itemAt(0, Type2Type< MRect >()), regionB);
    EXPECT_EQ(entryC.itemAt(0, Type2Type< MRect >()), regionC);

    //edit value
    MRect regionD(MPoint(20,21), MSize(22,23));
    entryA.replaceItemAt(0, regionD, Type2Type< MRect >());
    EXPECT_EQ(entryA.itemAt(0, Type2Type< MRect >()), regionD);

    //clear
    entryA.clear();
    EXPECT_EQ((int)entryA.count(), 0);


    //type int64
    IMetadata::IEntry sensorExp(MTK_SENSOR_EXPOSURE_TIME);
    sensorExp.push_back(1, Type2Type< MINT64>()); //int_64
    EXPECT_EQ((int)sensorExp.count(), 1);
    EXPECT_EQ(sensorExp.itemAt(0, Type2Type< MINT64 >()), 1);
}

TEST(IMetadata, Case1) {
    IMetadata m1;
    IMetadata m2;
    IMetadata m3;
    IMetadata &m4 = m1;

    m1 = m2;
    m1 +=m3;
    EXPECT_TRUE(m1.isEmpty());
    EXPECT_TRUE(m2.isEmpty());
    EXPECT_TRUE(m3.isEmpty());
    EXPECT_TRUE(m4.isEmpty());
    EXPECT_EQ((MUINT)0, m1.count());
}

TEST(IMetadata, Case2)
{
    META_LOGD("IMetadata, Case 2 begin===============");
    IMetadata m1,m2,empty;
    //insert data
    for(int tag=0;tag<10;tag+=2) {
        IMetadata::IEntry e1;
        IMetadata::IEntry e2;

        for(int i=0;i<tag;i++) {
            e1.push_back(MRational((MINT32)(i+tag),(MINT32)(i+tag+1)), Type2Type< MRational >());
        }
        m1.update(tag,e1); // tag = 0 will test dummy entry inseration

        IMetadata::Memory mem;
        mem.resize(tag);
        for(int i=0;i<tag;i++)
            mem.editArray()[i] = (MUINT8)(i);

        e2.push_back(mem, Type2Type< IMetadata::Memory >());
        m1.update(tag+1,e2);
        EXPECT_EQ(tag+2,(int)m1.count());
    }

    //query and verify
    for(int tag=0;tag<10;tag+=2) {
        IMetadata::IEntry e;
        e = m1.entryFor(tag);
        for(int i=0;i<(int)e.count();i++) {
            auto r = e.itemAt(i,Type2Type<MRational>());
            EXPECT_EQ((int)(i+tag), (int)r.numerator);
            EXPECT_EQ((int)(i+tag+1), (int)r.denominator);
        }

        e = m1.entryFor(tag+1);
        for(int i=0;i<(int)e.count();i++) {
            auto mem = e.itemAt(i,Type2Type<IMetadata::Memory>());

            for(int i=0;i<tag;i++) {
                EXPECT_EQ((MUINT8)i, (MUINT8) mem.editArray()[i]);
            }
        }
    }

    m2 += m1; //clone
    m2 += empty;
    m2 += m1; //replace with same tag
    m2 += m1; //replace with same tag
    m2 += m2;
    EXPECT_EQ(m1.count(), m2.count());

    EXPECT_FALSE(m1.isEmpty());
    META_LOGD("IMetadata, Case 2 end===============");
}
TEST(IMetadata, Case3)
{
    META_LOGD("IMetadata, Case 3 begin===============");

    IMetadata m;
    m.dump();
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            IMetadata::IEntry e1(j+i*10); // create new entry
            for(int k=0;k<3;k++) {
                e1.push_back(j+i*10+k*100, Type2Type<MINT32>());
            }
            m.update(j+i,e1); // put entry into metadata
        }
        for(int j=0;j<3;j++) {
            IMetadata::IEntry e2; //create new entry
            for(int k=0;k<3;k++) {
                e2.push_back(m, Type2Type<IMetadata>());  //put metadata into entry e2
            }
            m.update(j+i*10,e2); // put entry into metadat
        }
    }
    //m.dump();
    META_LOGD("IMetadata, Case 3 end===============");
}
TEST(IMetadata, Case4)
{
    META_LOGD("IMetadata, Case 4 begin===============");
    MRect region1(MPoint(2,3),MSize(4,5));
    MRect region2(MPoint(20,30),MSize(40,50));

    META_LOGD("test 1: construction");
    IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
    entry1.push_back(region1, Type2Type< MRect >());

    IMetadata metadata1;
    metadata1.update(MTK_SCALER_CROP_REGION, entry1);

    META_LOGD("add more tags..");
    IMetadata::IEntry sensorFps(MTK_SENSOR_INFO_FRAME_RATE);
    sensorFps.push_back(33, Type2Type< MINT32 >());
    metadata1.update(MTK_SENSOR_INFO_FRAME_RATE, sensorFps);

    META_LOGD("test 2: copy construction");
    IMetadata metadata2(metadata1);
    EXPECT_EQ(metadata2.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region1);

    META_LOGD("test 3: assignment operator");
    IMetadata metadata3 = metadata1;
    EXPECT_EQ(metadata3.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region1);


    META_LOGD("test 4: query (read-only)");
    const IMetadata::IEntry entry4 = metadata1.entryFor(MTK_SCALER_CROP_REGION);
    EXPECT_EQ(entry4.itemAt(0, Type2Type< MRect >()), region1);

    META_LOGD("test 5-1: query and revise through takeEntryFor, and update by metadata");
    {
        IMetadata::IEntry entry5 = metadata1.takeEntryFor(MTK_SCALER_CROP_REGION);
        EXPECT_EQ(entry5.itemAt(0, Type2Type< MRect >()), region1);
        entry5.replaceItemAt(0, region2, Type2Type< MRect >());
        metadata1.update(MTK_SCALER_CROP_REGION, entry5);

        const IMetadata::IEntry entry_test = metadata1.entryFor(MTK_SCALER_CROP_REGION);
        EXPECT_EQ(entry_test.itemAt(0, Type2Type< MRect >()), region2);
    }

    META_LOGD("test 6: set and get nested metadata");
    {
        //create
        IMetadata::IEntry sensorFps(MTK_SENSOR_INFO_FRAME_RATE);
        sensorFps.push_back(33, Type2Type< MINT32 >());

        IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
        MRect rect;
        rect.p.x = 10;
        rect.p.y = 20;
        rect.s.h = 600;
        rect.s.w = 800;
        sensorRegion.push_back(rect, Type2Type< MRect >());

        IMetadata metadataUnion;
        metadataUnion.update(MTK_SENSOR_INFO_FRAME_RATE, sensorFps);
        metadataUnion.update(sensorRegion.tag(), sensorRegion);

        IMetadata::IEntry entryUnion(MTK_SENSOR_INFO_PACKAGE);
        entryUnion.push_back(metadataUnion, Type2Type< IMetadata >());

        IMetadata metadataOverall;
        metadataOverall.update(MTK_SENSOR_INFO_PACKAGE, entryUnion);

        //read
        const IMetadata::IEntry entry = metadataOverall.entryFor(MTK_SENSOR_INFO_PACKAGE);
        const IMetadata metadataUnion_getback = entry.itemAt(0, Type2Type< IMetadata >());

        const IMetadata::IEntry entryB = metadataUnion_getback.entryFor(MTK_SENSOR_INFO_FRAME_RATE);
        const IMetadata::IEntry entryC = metadataUnion_getback.entryFor(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);

        EXPECT_EQ(entryB.itemAt(0, Type2Type< MINT32 >()), 33);
        EXPECT_EQ(entryC.itemAt(0, Type2Type< MRect >()).p.x, 10);
        EXPECT_EQ(entryC.itemAt(0, Type2Type< MRect >()).p.y, 20);
        EXPECT_EQ(entryC.itemAt(0, Type2Type< MRect >()).s.w, 800);
        EXPECT_EQ(entryC.itemAt(0, Type2Type< MRect >()).s.h, 600);
    }


    META_LOGD("test 7: remove");
    {
        IMetadata metadata7(metadata1);
        const IMetadata::IEntry entry = metadata7.entryFor(MTK_SCALER_CROP_REGION);
        MRect region = entry.itemAt(0, Type2Type< MRect >());

        metadata1.remove(MTK_SCALER_CROP_REGION);

        EXPECT_EQ(metadata7.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region);
        EXPECT_EQ(metadata1.entryFor(MTK_SCALER_CROP_REGION).tag(), IMetadata::IEntry::BAD_TAG);
    }

    META_LOGD("test 8: operator+");
    {
        // const values
        MINT32 const sensorFps = 33;
        MRect const activeArray(MPoint(10, 20), MSize(600, 600));
        MRect const cropRegion(MPoint(5, 7), MSize(99, 123));
        //
        IMetadata metadata8_0;
        {
            //create metadata with 2 entry
            IMetadata::IEntry entryFps(MTK_SENSOR_INFO_FRAME_RATE);
            entryFps.push_back(sensorFps, Type2Type< MINT32 >());

            IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
            sensorRegion.push_back(activeArray, Type2Type< MRect >());

            metadata8_0.update(MTK_SENSOR_INFO_FRAME_RATE, entryFps);
            metadata8_0.update(sensorRegion.tag(), sensorRegion);
        }
        IMetadata metadata8_1;
        {
            IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
            entry1.push_back(cropRegion, Type2Type< MRect >());
            metadata8_1.update(MTK_SCALER_CROP_REGION, entry1);
        }

        IMetadata metadataresult = metadata8_0 + metadata8_1;
        {
            // check
            IMetadata::IEntry entry = metadataresult.entryFor(MTK_SENSOR_INFO_FRAME_RATE);
            EXPECT_NE( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                EXPECT_EQ(
                        entry.itemAt(0, Type2Type< MINT32 >()),
                        sensorFps);
            }
            entry = metadataresult.entryFor(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
            EXPECT_NE( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                EXPECT_EQ(
                        entry.itemAt(0, Type2Type< MRect >()),
                        activeArray);
            }
            entry = metadataresult.entryFor(MTK_SCALER_CROP_REGION);
            EXPECT_NE( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                EXPECT_EQ(
                        entry.itemAt(0, Type2Type< MRect >()),
                        cropRegion);
            }
        }
    }
    META_LOGD("IMetadata, Case 4 end===============");
}
TEST(IMetadata, Case5)
{
    META_LOGD("IMetadata, Case 5 begin===============");
    // either add itemAt function with an index as input parameter,
    // or search through all enums.
    IMetadata unionMetadata;
    IMetadata::Tag_t mTag = IMetadata::IEntry::BAD_TAG;

    META_LOGD("create metadataA");
    //
    IMetadata metadataA = []() -> IMetadata {
        IMetadata::IEntry entryA(MTK_SCALER_CROP_REGION);
        MRect region(MPoint(2,3), MSize(4,5));
        entryA.push_back(region, Type2Type< MRect >());

        IMetadata metadata;
        metadata.update(MTK_SCALER_CROP_REGION, entryA);
        return metadata;
    }();

    META_LOGD("union += metadataA");
    for (size_t i = 0; i < metadataA.count(); i++)
    {
        mTag = metadataA.entryAt(i).tag();
        unionMetadata.update(metadataA.entryAt(i).tag(), metadataA.entryAt(i));
    }
    EXPECT_EQ(unionMetadata.count(), metadataA.count());

    META_LOGD("create metadataB");
    //
    IMetadata metadataB = []()->IMetadata {
        IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
        MRect rect;
        rect.p.x = 10;
        rect.p.y = 20;
        rect.s.h = 600;
        rect.s.w = 800;
        sensorRegion.push_back(rect, Type2Type< MRect >());

        IMetadata metadata;
        metadata.update(sensorRegion.tag(), sensorRegion);
        return metadata;
    }();

    META_LOGD("union += metadataB");
    for (size_t i = 0; i < metadataB.count(); i++)
    {
        unionMetadata.update(metadataB.entryAt(i).tag(), metadataB.entryAt(i));
    }

    EXPECT_EQ(unionMetadata.count(), metadataA.count() + metadataB.count());
    EXPECT_NE(mTag, IMetadata::IEntry::BAD_TAG);

    META_LOGD("check same tag in union and metadataA");
    if (mTag != IMetadata::IEntry::BAD_TAG) {
        IMetadata::IEntry EntryA = unionMetadata.entryFor(mTag);
        IMetadata::IEntry EntryB = metadataA.entryFor(mTag);
        EXPECT_EQ(EntryA.itemAt(0, Type2Type< MRect >()), EntryB.itemAt(0, Type2Type< MRect >()));
    }
    META_LOGD("IMetadata, Case 5 end===============");
}
TEST(IMetadata, Case6)
{
    META_LOGD("IMetadata, Case 6 begin===============");

    MUINT8 val = 2;
    size_t size = 10;

    IMetadata::Memory mMemory;
    mMemory.resize(size);

    memset(mMemory.editArray(), val, sizeof(MUINT8)*size);

    IMetadata::Memory mMemory2;
    mMemory2 = mMemory;

    mMemory.clear();

    EXPECT_EQ(mMemory2.size(), size);
    EXPECT_EQ(mMemory2.itemAt(0), val);
    META_LOGD("IMetadata, Case 6 end===============");
}
TEST(IMetadata, Case7)
{
    META_LOGD("IMetadata, Case 7 begin===============");

    MUINT8 val = 2;
    size_t size = 10;

    IMetadata::Memory mMemory;
    mMemory.resize(size);
    memset(mMemory.editArray(), val, sizeof(MUINT8)*size);

    const int MEMORY_TAG = 1;
    IMetadata::IEntry entry(MEMORY_TAG);
    entry.push_back(mMemory, Type2Type<IMetadata::Memory>());

    IMetadata meta;
    meta.update(MEMORY_TAG, entry);

    //read value
    IMetadata::Memory readOut = meta.entryFor(MEMORY_TAG).itemAt(0, Type2Type<IMetadata::Memory>());
    for(size_t i = 0; i < readOut.size(); i++) {
        EXPECT_EQ(readOut.itemAt(i), val);
    }
    META_LOGD("IMetadata, Case 7 end===============");
}
TEST(IMetadata, Case8)
{
    META_LOGD("IMetadata, Case 8 begin===============");

    typedef NSCam::IMetadata::Memory Memory;

    // elements we are going to be checked
    MUINT8   val_MUINT8 = 32;
    MINT32   val_MINT32 = 10011;
    MDOUBLE  val_MDOUBLE = 3.1415926;
    Memory   val_Memory1;
    Memory   val_Memory2;

    // prepare random memory data chunk
    val_Memory1.resize(1280);
    val_Memory2.resize(110011);
    auto random_memory_content = [](Memory *pMemory)
    {
        uint8_t *p = pMemory->editArray();
        for (size_t i = 0; i <pMemory->size(); i++) {
            *(p++) = static_cast<uint8_t>(std::rand());
        }
    };

    META_LOGD("random memory...");
    random_memory_content(&val_Memory1);
    random_memory_content(&val_Memory2);

    // prepare source metadata
    IMetadata sourceMeta;
    IMetadata resultMeta;
    std::unique_ptr<char[]> chunk(new char[120000]);

    // construct sourceMeta
    META_LOGD("add entry 0");
    IMetadata::setEntry<MUINT8> (&sourceMeta, 0, val_MUINT8);
    META_LOGD("add entry 1");
    IMetadata::setEntry<MINT32> (&sourceMeta, 1, val_MINT32);
    META_LOGD("add entry 2");
    IMetadata::setEntry<MDOUBLE>(&sourceMeta, 2, val_MDOUBLE);
    META_LOGD("add entry 3");
    IMetadata::setEntry<Memory> (&sourceMeta, 3, val_Memory1);
    META_LOGD("add entry 4");
    IMetadata::setEntry<Memory> (&sourceMeta, 4, val_Memory2);

    // flatten & unflatten
    META_LOGD("flatten...");
    sourceMeta.  flatten(chunk.get(), 120000);
    META_LOGD("flatten done");

    META_LOGD("unflatten...");
    resultMeta.unflatten(chunk.get(), 120000);
    META_LOGD("unflatten done.");

    // get result
    MUINT8   result_MUINT8 = 0;
    MINT32   result_MINT32 = 0;
    MDOUBLE  result_MDOUBLE = 0.0f;
    Memory   result_Memory1;
    Memory   result_Memory2;
    bool ret = true;
    //
    META_LOGD("get entry 0");
    ret &= IMetadata::getEntry<MUINT8> (&resultMeta, 0, result_MUINT8);
    META_LOGD("get entry 1");
    ret &= IMetadata::getEntry<MINT32> (&resultMeta, 1, result_MINT32);
    META_LOGD("get entry 2");
    ret &= IMetadata::getEntry<MDOUBLE>(&resultMeta, 2, result_MDOUBLE);
    META_LOGD("get entry 3");
    ret &= IMetadata::getEntry<Memory> (&resultMeta, 3, result_Memory1);
    META_LOGD("get entry 4");
    ret &= IMetadata::getEntry<Memory> (&resultMeta, 4, result_Memory2);

    META_LOGD("check result...(%d)", ret);
    // check if they're equivalent
    META_LOGD("check result 0");
    EXPECT_EQ(result_MUINT8,   val_MUINT8);
    META_LOGD("check result 1");
    EXPECT_EQ(result_MINT32,   val_MINT32);
    META_LOGD("check result 2");
    EXPECT_EQ(result_MDOUBLE,  val_MDOUBLE);

    // memory chunk, check size first
    META_LOGD("check memory chunks sizes...");
    EXPECT_EQ(result_Memory1.size(), val_Memory1.size());
    EXPECT_EQ(result_Memory2.size(), val_Memory2.size());

    // memory compare
    META_LOGD("check memory 1 ...");
    EXPECT_EQ(
            ::memcmp(val_Memory1.array(), result_Memory1.array(), val_Memory1.size()),
            0);

    META_LOGD("check memory 2... ");
    EXPECT_EQ(
            ::memcmp(val_Memory2.array(), result_Memory2.array(), val_Memory2.size()),
            0);

    META_LOGD("%s: [-]", __FUNCTION__);
    META_LOGD("IMetadata, Case 8 end===============");
}
TEST(IMetadata, Case9)
{
    META_LOGD("IMetadata, Case 9 begin===============");

    IMetadata::IEntry e(5),e1,e2,e3;
    IMetadata meta;
    META_LOGD("Next should show error : tag mismatch update tag=4 but entry.tag=5");
    meta.update(4, e);
    e1 = meta.takeEntryFor(4);
    e2 = e1;
    e3 = e2;
    meta.clear();
    META_LOGD("Next should show error : tag mismatch update tag=1 but entry.tag=4");
    meta.update(1,e3);
    META_LOGD("IMetadata, Case 9 end===============");
}
#ifdef MTKCAM_METADATA_V2

TEST(IMetadata, Case10)
{
    META_LOGD("IMetadata, Case 10 begin===============");

    IMetadata::IEntry e;
    MUINT8 array [] = {1,3,5,7,9};
    e.push_back(array,5,Type2Type<MUINT8>());

    auto data = (MUINT8*)e.data();
    int  count = (int)e.count();
    for(auto i=0;i< count;i++)
        META_LOGD("%d %d",i, (int)data[i]);

    META_LOGD("IMetadata, Case 10 end===============");
}
TEST(IMetadata, Case11)
{
    META_LOGD("IMetadata, Case 11 begin===============");

    IMetadata::IEntry e;
    IMetadata m;

    std::string S = "Hello World";
    e.push_back((MUINT8*)S.c_str(), S.size()+1, Type2Type<MUINT8>());

    m.update(123,e);

    std::string T((char*)m.entryFor(123).data());

    EXPECT_TRUE(S == T);

    META_LOGD("IMetadata, Case 11 end===============");
}
TEST(IMetadata, Case12)
{
    META_LOGD("IMetadata, Case 12 begin===============");

    IMetadata::IEntry e;
    IMetadata m;

    auto size = 100;
    MINT64 array[size];

    MINT64 sum = 0;
    for(auto &item: array) {
        item = (MINT64)std::rand() %10000;
        sum += item;
    }

    for(auto &item: array) {
        META_LOGD("%d",(int) item);
    }

    e.push_back(array, size, Type2Type<MINT64>());
    m.update(100,e);
    e.push_back(array, size, Type2Type<MINT64>());
    m.update(200,e);
    e.push_back(array, size, Type2Type<MINT64>());
    m.update(300,e);

    for(auto &item: array)
        item = (MINT64) 1;

    e.replaceItemAt(size, array, size, Type2Type<MINT64>()); // replace middle part to 1 x size
    m.update(400,e);

    {
        auto e = m.entryFor(100);
        auto c = e.count();
        MINT64 d[c];

        e.itemAt(0,d,c,Type2Type<MINT64>());

        MINT64 s = 0;
        for(auto &item: d)
            s += item;

        EXPECT_EQ(s,sum);
    }
    {
        auto e = m.entryFor(200);
        auto c = e.count();
        MINT64 d[c];

        e.itemAt(0,d,c,Type2Type<MINT64>());

        MINT64 s = 0;
        for(auto &item: d)
            s += item;

        EXPECT_EQ(s,sum*2);
    }
    {
        auto e = m.entryFor(300);
        auto c = e.count();
        MINT64 d[c];

        e.itemAt(0,d,c,Type2Type<MINT64>());

        MINT64 s = 0;
        for(auto &item: d)
            s += item;

        EXPECT_EQ(s,sum*3);
    }
    {
        auto e = m.entryFor(400);
        auto c = e.count();
        MINT64 d[c];

        e.itemAt(0,d,c,Type2Type<MINT64>());

        MINT64 s = 0;
        for(auto &item: d)
            s += item;

        EXPECT_EQ(s,sum*2 + size);
    }

    META_LOGD("IMetadata, Case 12 end===============");
}
/******************************************************************************
 * case13  test nest metadata flatten/unflatten
 ******************************************************************************/
TEST(IMetadata, Case13)
{
    META_LOGD("IMetadata, Case 13 begin===============");


    // test null entry to mata flatten
    {
        IMetadata m;
        IMetadata::IEntry e;
        m.update(0x1,e);
        verifyFlatten(m);
    }

    // test null meta to meta flatten
    {
        IMetadata m1,m2;
        IMetadata::IEntry e;

        e.push_back(m1, Type2Type<IMetadata>());
        m2.update(0x1,e);

        verifyFlatten(m2);
    }
    // test nest null meta to meta flatten
    {
        IMetadata m;
        IMetadata::IEntry e;

        auto tag = 0;
        for(int i=0;i<10;i++) {
            e.push_back(m, Type2Type<IMetadata>());
            m.update(tag++,e);
            verifyFlatten(m);
        }
    }
    // test nest null meta & null mem to meta flatten
    {
        IMetadata m,m1;
        IMetadata::Memory a;
        IMetadata::IEntry e,e1;

        auto tag = 0;
        for(int i=0;i<10;i++) {

            e1.push_back(a, Type2Type<IMetadata::Memory>());
            m1.update(tag++,e1); // put null mem to m1

            e.push_back(m1, Type2Type<IMetadata>());
            m.update(tag++,e); // put m1 into m

            verifyFlatten(m);
        }
    }
    // test nest random data to check flatten
    {
        IMetadata m;
        IMetadata::Memory a;
        IMetadata::IEntry e;
        MUINT8      v8;
        MRational   vR;

        auto tag = 0;
        for(int i=0;i<10;i++) {
            e.clear();
            e.push_back(m, Type2Type<IMetadata>());
            m.update(tag++,e);

            e.clear();
            e.push_back(a, Type2Type<IMetadata::Memory>());
            m.update(tag++,e);

            e.clear();
            e.push_back(v8, Type2Type<MUINT8>());
            m.update(tag++,e);

            e.clear();
            e.push_back(vR, Type2Type<MRational>());
            m.update(tag++,e);

            verifyFlatten(m);

            // random v8
            v8 = (MUINT8)std::rand();

            // random Rational
            vR = MRational((int)std::rand(),(int)std::rand());

            // random Memory
            auto size = std::rand() % 1000;
            a.clear();
            if(size >0 ) {
                a.resize(size);
                auto p = a.editArray();
                for(int i=0;i<(int)size;i++)
                    p[i] = (MUINT8)std::rand();
            }
        }
    }

    META_LOGD("IMetadata, Case 13 end===============");
}

/******************************************************************************
 * case14  test whitelist
 ******************************************************************************/
TEST(IMetadata, Case14)
{
    META_LOGD("IMetadata, Case 14 begin===============");

    IMetadata::IEntry e;
    IMetadata m;

    std::string S = "Hello World";
    e.push_back((MUINT8*)S.c_str(), S.size()+1, Type2Type<MUINT8>());

    for(int i=0;i<100;i++)
        m.update(i,e);

    auto fullSize = m.flattenSize();

    IMetadata::Tag_t OW[50];
    for(int i=0;i<50;i++)
        OW[i] = i*2+1;

    IMetadata::Tag_t EW[50];
    for(int i=0;i<50;i++)
        EW[i] = i*2;

    auto oddSize  = m.flattenSize(OW,50);
    auto evenSize = m.flattenSize(EW,50);

    EXPECT_TRUE(fullSize > oddSize);
    EXPECT_TRUE(fullSize > evenSize);

    IMetadata om,em;
    {
        std::vector<MUINT8> buf;
        buf.resize(oddSize);

        auto result = m.flatten(buf.data(), buf.size(), OW,50);

        result = om.unflatten(buf.data(),result);

        EXPECT_EQ(result, (decltype(result)) oddSize);
    }

    EXPECT_EQ((int)om.count(), 50);

    {
        std::vector<MUINT8> buf;
        buf.resize(evenSize);

        auto result = m.flatten(buf.data(), buf.size(), EW,50);

        result = em.unflatten(buf.data(),result);

        EXPECT_EQ(result, (decltype(result)) evenSize);
    }
    EXPECT_EQ((int)em.count(), 50);

    {
        std::vector<MUINT8> buf,buf1;
        IMetadata oem = om+em;

        buf.resize( oem.flattenSize());
        oem.flatten(buf.data(), buf.size());

        buf1.resize( m.flattenSize());
        m.flatten(buf1.data(), buf1.size());

        EXPECT_EQ( buf, buf1);
    }

    verifyWhiteList(m);

    META_LOGD("IMetadata, Case 14 end===============");
}
TEST(IMetadata, Case15)
{
    META_LOGD("IMetadata, Case 15 begin===============");

    IMetadata m;
    IMetadata m1;
    IMetadata m2;
    IMetadata m3;

    IMetadata::IEntry e,ee;
    IMetadata::Memory mem1,mem2;
    {
        mem1.resize(10);
        uint8_t *p = mem1.editArray();
        for (size_t i = 0; i <mem1.size(); i++) {
            *(p++) = static_cast<uint8_t>(std::rand());
        }
    }
    {
        mem2.resize(20);
        uint8_t *p = mem2.editArray();
        for (size_t i = 0; i <mem2.size(); i++) {
            *(p++) = static_cast<uint8_t>(std::rand());
        }
    }

    IMetadata::setEntry<MINT32>(&m1, 0x0 , 1800 );
    IMetadata::setEntry<MINT32>(&m1, 0x1 , 5580 );
    IMetadata::setEntry<MINT32>(&m1, 0x4 , 0 );
    IMetadata::setEntry<MINT32>(&m1, 0x5 , 255 );
    IMetadata::setEntry<MINT32>(&m1, 0x6 , 0 );
    IMetadata::setEntry<MINT32>(&m1, 0x7 , 0 );
    IMetadata::setEntry<MINT32>(&m1, 0x8 , 0 );
    IMetadata::setEntry<MINT32>(&m1, 0x9 , 2 );
    IMetadata::setEntry<MINT32>(&m1, 0xa , 0 );
    IMetadata::setEntry<MINT32>(&m1, 0xb , 19999 );
    IMetadata::setEntry<MINT32>(&m1, 0xc , 1482 );
    IMetadata::setEntry<MINT32>(&m1, 0xe , -252579322 );
    IMetadata::setEntry<IMetadata::Memory>(&m1, 0xf ,  mem1);
    IMetadata::setEntry<MINT32>(&m1, 0x10 , -5042977 );
    IMetadata::setEntry<MINT32>(&m1, 0x12 , -185207049 );
    IMetadata::setEntry<IMetadata::Memory>(&m1, 0x13 , mem2);
    IMetadata::setEntry<IMetadata>(&m, 0xc0040000 , m1);

    m.dump(0);

    std::vector<MUINT8> buf;
    auto size = m.flattenSize();
    buf.resize(size);
    META_LOGD("flattenSize %d",(int)size);

    auto result = m.flatten(buf.data(), size);
    EXPECT_EQ(size, (size_t) result);

    result = m2.unflatten(buf.data(), size);
    EXPECT_EQ(size, (size_t) result);

    e = m2.entryAt(0);
    EXPECT_EQ((int)e.tag(), (int)0xc0040000);

    m3 = e.itemAt(0, Type2Type<IMetadata>());

    EXPECT_EQ((int)m3.count(), 16);

    META_LOGD("IMetadata, Case 15 end===============");
}
#if 0
TEST(IMetadata, Case99_StressRandomize)
{
    META_LOGD("IMetadata, Case 99 begin===============");
    const int MAX_TAG = 10;
    const int MAX_ENTRY = 10;
    const int MAX_METADATA = 10;
    const int MAX_MEMORY = 10;
    const int MAX_THREAD = 1;
    const int MAX_LOOP = 500000;
    //const int MAX_ENTRY_SIZE = 1024*1024*32; //32MB

    auto randEntryOP = [] (IMetadata &m, IMetadata::IEntry &e) {
        auto randMemory = []() -> IMetadata::Memory {
            auto size = static_cast<size_t>(std::rand()%(33)); //33 bytes
            META_LOGD("randMemory size %d",(int)size);
            IMetadata::Memory memory;
            memory.resize(size);
            uint8_t *p = memory.editArray();
            for (size_t i = 0; i <memory.size(); i++) {
                *(p++) = static_cast<uint8_t>(std::rand());
            }
            return memory;
        };
        auto randMUINT8 = []() -> MUINT8 {
            META_LOGD("randMUINT8");
            return static_cast<MUINT8>(std::rand());
        };
        auto randMINT32 = []() -> MINT32 {
            META_LOGD("randMINT32");
            return static_cast<MINT32>(std::rand());
        };
        auto randMINT64 = []() -> MINT64 {
            META_LOGD("randMINT64");
            return static_cast<MINT64>(std::rand());
        };
        auto randMFLOAT = []() -> MFLOAT {
            META_LOGD("randMFLOAT");
            return static_cast<MFLOAT>(std::rand());
        };
        auto randMDOUBLE = []() -> MDOUBLE {
            META_LOGD("randMDOUBLE");
            return static_cast<MDOUBLE>(std::rand());
        };
        auto randMRational = []() -> MRational {
            META_LOGD("randMRational");
            return MRational((int)std::rand(),(int)std::rand());
        };
        auto randMPoint = []() -> MPoint {
            META_LOGD("randMPoint");
            return MPoint((int)std::rand(),(int)std::rand());
        };
        auto randMSize = []() -> MSize {
            META_LOGD("randMSize");
            return MSize((int)std::rand(),(int)std::rand());
        };
        auto randMRect = []() -> MRect {
            META_LOGD("randMRect");
            return MRect(MPoint((int)std::rand(),(int)std::rand()),MSize((int)std::rand(),(int)std::rand()));
        };
        //MUINT32 tag = static_cast<MUINT32>(std::rand());

        auto type = e.type();

        if(type == -1 || (std::rand()%10)==0) { // no type or 10% chance to use wrong type
            type = std::rand()%NUM_MTYPES;
        }

        //if(type == TYPE_Memory) type = TYPE_MUINT8;

        const char *entryOPName[] = {
            "push_back",//0
            "replaceItemAt",//1
            "itemAt",//2
            "tag",//3
            "type",//4
            "data",//5
            "isEmpty",//6
            "count",//7
            "clear",//8
            "removeAt",//9
            "push_back array",//10
            "replaceItemAt array", //11
            "itemAt array", //12
        };

        auto op = std::rand()% (sizeof(entryOPName) / sizeof(entryOPName[0]));

        META_LOGD("randEntry OP(%d)= %s",(int)op, entryOPName[op]);
        switch(op) { // push_back/replace/delete
            case 0:
                {
                    switch(type) {
                        case TYPE_MUINT8: e.push_back(randMUINT8(),Type2Type<MUINT8>());break;
                        case TYPE_MINT32: e.push_back(randMINT32(),Type2Type<MINT32>());break;
                        case TYPE_MFLOAT: e.push_back(randMFLOAT(),Type2Type<MFLOAT>());break;
                        case TYPE_MINT64: e.push_back(randMINT64(),Type2Type<MINT64>());break;
                        case TYPE_MDOUBLE: e.push_back(randMDOUBLE(),Type2Type<MDOUBLE>());break;
                        case TYPE_MRational: e.push_back(randMRational(),Type2Type<MRational>());break;
                        case TYPE_MPoint: e.push_back(randMPoint(),Type2Type<MPoint>());break;
                        case TYPE_MSize: e.push_back(randMSize(),Type2Type<MSize>());break;
                        case TYPE_MRect: e.push_back(randMRect(),Type2Type<MRect>());break;
                        case TYPE_Memory: e.push_back(randMemory(),Type2Type<IMetadata::Memory>());break;
                        case TYPE_IMetadata: {
                            //auto size = (int)m.flattenSize();
                            e.push_back(m,Type2Type<IMetadata>());
                            break;
                        }
                    };
                    break;
                }
            case 1:
                {
                    auto idx = (MUINT)(std::rand() % e.count());
                    META_LOGD("replace at type = %x, idx = %d",type, (int)idx);

                    switch(type) {
                        case TYPE_MUINT8: e.replaceItemAt(idx,randMUINT8(),Type2Type<MUINT8>());break;
                        case TYPE_MINT32: e.replaceItemAt(idx,randMINT32(),Type2Type<MINT32>());break;
                        case TYPE_MFLOAT: e.replaceItemAt(idx,randMFLOAT(),Type2Type<MFLOAT>());break;
                        case TYPE_MINT64: e.replaceItemAt(idx,randMINT64(),Type2Type<MINT64>());break;
                        case TYPE_MDOUBLE: e.replaceItemAt(idx,randMDOUBLE(),Type2Type<MDOUBLE>());break;
                        case TYPE_MRational: e.replaceItemAt(idx,randMRational(),Type2Type<MRational>());break;
                        case TYPE_MPoint: e.replaceItemAt(idx,randMPoint(),Type2Type<MPoint>());break;
                        case TYPE_MSize: e.replaceItemAt(idx,randMSize(),Type2Type<MSize>());break;
                        case TYPE_MRect: e.replaceItemAt(idx,randMRect(),Type2Type<MRect>());break;
                        case TYPE_Memory: e.replaceItemAt(idx,randMemory(),Type2Type<IMetadata::Memory>());break;
                        case TYPE_IMetadata: e.replaceItemAt(idx,m,Type2Type<IMetadata>()); break;
                    };
                    break;
                }
            case 2:
                {
                    auto idx = (MUINT)(std::rand() % e.count());

                    switch(type) {
                        case TYPE_MUINT8: e.itemAt(idx,Type2Type<MUINT8>());break;
                        case TYPE_MINT32: e.itemAt(idx,Type2Type<MINT32>());break;
                        case TYPE_MFLOAT: e.itemAt(idx,Type2Type<MFLOAT>());break;
                        case TYPE_MINT64: e.itemAt(idx,Type2Type<MINT64>());break;
                        case TYPE_MDOUBLE: e.itemAt(idx,Type2Type<MDOUBLE>());break;
                        case TYPE_MRational: e.itemAt(idx,Type2Type<MRational>());break;
                        case TYPE_MPoint: e.itemAt(idx,Type2Type<MPoint>());break;
                        case TYPE_MSize: e.itemAt(idx,Type2Type<MSize>());break;
                        case TYPE_MRect: e.itemAt(idx,Type2Type<MRect>());break;
                        case TYPE_Memory: e.itemAt(idx,Type2Type<IMetadata::Memory>());break;
                        case TYPE_IMetadata: e.itemAt(idx,Type2Type<IMetadata>()); break;
                    };
                    break;
                }
            case 3:
                {
                    e.tag();
                    break;
                }
            case 4:
                {
                    e.type();
                    break;
                }
            case 5:
                {
                    e.data();
                    break;
                }
            case 6:
                {
                    e.isEmpty();
                    break;
                }
            case 7:
                {
                    e.count();
                    break;
                }
            case 8:
                {
                    e.clear();
                    break;
                }
            case 9:
                {
                    auto idx = (MUINT)(std::rand() % e.count()); // count = 0 will cause invalid index value
                    e.removeAt(idx);// when removeAt, previous count() number may not the same as current numbers
                    break;
                }
            case 10:
                {
                    auto num = (std::rand() %10) + 1;
                    switch(type){
                        #define CASE_PUSH_BACK_ARRAY(_T) \
                        case TYPE_##_T: \
                            { \
                                _T array[num]; \
                                for(auto &item: array) item = rand##_T(); \
                                e.push_back(array, num ,Type2Type<_T>()); \
                                break; \
                            }
                        CASE_PUSH_BACK_ARRAY(MUINT8)
                        CASE_PUSH_BACK_ARRAY(MINT32)
                        CASE_PUSH_BACK_ARRAY(MFLOAT)
                        CASE_PUSH_BACK_ARRAY(MINT64)
                        CASE_PUSH_BACK_ARRAY(MDOUBLE)
                        CASE_PUSH_BACK_ARRAY(MRational)
                        CASE_PUSH_BACK_ARRAY(MPoint)
                        CASE_PUSH_BACK_ARRAY(MSize)
                        CASE_PUSH_BACK_ARRAY(MRect)
                    }
                    break;
                }
            case 11:
                {
                    auto count = e.count();
                    if(count>0) {
                        auto index =(MUINT)(std::rand() % count);
                        auto num = (std::rand() % (count - index)) + 1;

                        switch(type){
                            #define CASE_REPLACE_ARRAY(_T) \
                            case TYPE_##_T: \
                                { \
                                    _T array[num]; \
                                    for(auto &item: array) item = rand##_T(); \
                                    e.replaceItemAt(index, array, num ,Type2Type<_T>()); \
                                    break; \
                                }
                            CASE_REPLACE_ARRAY(MUINT8)
                            CASE_REPLACE_ARRAY(MINT32)
                            CASE_REPLACE_ARRAY(MFLOAT)
                            CASE_REPLACE_ARRAY(MINT64)
                            CASE_REPLACE_ARRAY(MDOUBLE)
                            CASE_REPLACE_ARRAY(MRational)
                            CASE_REPLACE_ARRAY(MPoint)
                            CASE_REPLACE_ARRAY(MSize)
                            CASE_REPLACE_ARRAY(MRect)
                        }
                    }
                    break;
                }
            case 12:
                {
                    auto count = e.count();
                    if(count>0) {
                        auto index =(MUINT)(std::rand() % count);
                        auto num = (std::rand() % (count - index)) + 1;
                        auto realType = e.type();

                        switch(type){
                            #define CASE_ITEM_ARRAY(_T) \
                            case TYPE_##_T: \
                                { \
                                    _T array[num]; \
                                    if(realType == type) { \
                                        EXPECT_EQ(MTRUE,e.itemAt(index, array, num ,Type2Type<_T>())); \
                                        for(int i=0;i<(int)num;i++) {\
                                            _T result = e.itemAt(index+i, Type2Type<_T>()); \
                                            EXPECT_EQ(0,  memcmp(&array[i], &result, sizeof(_T))); \
                                        } \
                                    }\
                                    else \
                                        EXPECT_EQ(MFALSE,e.itemAt(index, array, num ,Type2Type<_T>())); \
                                    break; \
                                }
                            CASE_ITEM_ARRAY(MUINT8)
                            CASE_ITEM_ARRAY(MINT32)
                            CASE_ITEM_ARRAY(MFLOAT)
                            CASE_ITEM_ARRAY(MINT64)
                            CASE_ITEM_ARRAY(MDOUBLE)
                            CASE_ITEM_ARRAY(MRational)
                            CASE_ITEM_ARRAY(MPoint)
                            CASE_ITEM_ARRAY(MSize)
                            CASE_ITEM_ARRAY(MRect)
                        }
                    }
                    break;
                }
        }
    };
    auto randMetadataOP = [] (IMetadata &m1,IMetadata &m2,IMetadata::IEntry &e) {
        const char* metadataOPName[] = {
            "m1.update(tag,e)",//0
            "m1.remove",//1
            "m1=m2",//2
            "m1=IMetadata(m2)",//3
            "m1+=m2",//4
            "m1=m1+m2",//5
            "isEmpty",//6
            "count",//7
            "clear",//8
            "remove",//9
            "sort",//10
            "entryFor",//11
            "entryAt",//12
            "takeEntryFor",//13
            "m2 = unflatten(m1.flatten)",//14
            "verifyFlatten(m1)", // 15
            //"unflatten(wrong random content)", // remove
            "verifyWhiteList(m1)", // 16
            "m1.update(e)",//17
        };

        auto max_op = (sizeof(metadataOPName) / sizeof(metadataOPName[0]));
        auto op = std::rand()% max_op;
        META_LOGD("randMetadata OP(%d)= %s",(int)op, metadataOPName[op]);
        switch(op) {
            case 0:
                {
                    auto tag = (int)(std::rand() % MAX_TAG);
                    m1.update(tag,e);
                    break;
                }
            case 1:
                {
                    auto tag = (int)(std::rand() % MAX_TAG);
                    m1.remove(tag);
                    break;
                }
            case 2:
                {
                    m1 = m2;
                    break;
                }
            case 3:
                {
                    m1 = IMetadata(m2);
                    break;
                }
            case 4:
                {
                    m1 += m2;
                    break;
                }
            case 5:
                {
                    m1 = m1 + m2;
                    break;
                }
            case 6:
                {
                    m1.isEmpty();
                    break;
                }
            case 7:
                {
                    m1.count();
                    break;
                }
            case 8:
                {
                    m1.clear();
                    break;
                }
            case 9:
                {
                    auto tag = (int)(std::rand() % MAX_TAG);
                    m1.remove(tag);
                    break;
                }
            case 10:
                {
                    m1.sort();
                    break;
                }
            case 11:
                {
                    auto tag = (int)(std::rand() % MAX_TAG);
                    e = m1.entryFor(tag);
                    break;
                }
            case 12:
                {
                    IMetadata tmp = m1; // copy m1 to local to avoid multi thread issue
                    if(tmp.count()>0) {
                        auto index = (MUINT)(std::rand() % tmp.count());
                        e = tmp.entryAt(index);
                    }
                    break;
                }
            case 13:
                {
                    auto tag = (int)(std::rand() % MAX_TAG);
                    e = m1.takeEntryFor(tag);
                    break;
                }
            case 14:
                {
                    std::vector<MUINT8> b;
                    IMetadata tmp = m1; // copy m1 to local to avoid multi thread issue
                    auto size = tmp.flattenSize();
                    b.resize(size);
                    tmp.flatten(b.data(),size); // flatten tmp to buffer
                    m2.unflatten(b.data(),size); // unflatten buffer to m2
                    break;
                }
            case 15:
                {
                    verifyFlatten(m1);
                    break;
                }
                /*
                 *  remove test case due to
                 *  support random modify need check everywhere
                 *  It cause lots unnecessary code inseration !!
                 *
             case 16:
                {
                    std::vector<MUINT8> buf,modBuf;
                    auto size = m1.flattenSize();
                    buf.resize(size);

                    META_LOGD("size %d",(int)size);

                    auto result = m1.flatten(buf.data(), size);
                    EXPECT_EQ(size, (size_t) result);

                    modBuf = buf;

                    auto newSize = size;
                    //auto newSize = (size_t) (std::rand() %(size*2));
                    //if(newSize > size*2) // avoid size = 0
                    //    newSize = size*2;
                    //modBuf.resize(newSize); //change size

                    if(newSize>0) {
                        auto mod = (int)std::rand()%newSize;

                        META_LOGD("mod %d",(int)mod);
                        for(int i = 0;i<mod;i++) {
                            auto pos = (size_t) (std::rand() %newSize);
                            modBuf[pos] = (MUINT8)((int)modBuf[pos] + (int)(std::rand()%20) - 10);
                        }
                    }

                    IMetadata m;
                    META_LOGD("b %d", (int)newSize);
                    result = m.unflatten(modBuf.data(),newSize); // it should not NE
                    META_LOGD("e %d", (int) result);
                    break;
                 }
                */
            case 16:
                {
                    verifyWhiteList(m1);
                    break;
                }
            case 17:
                {
                    auto tag = e.tag();
                    auto result = m1.update(e);
                    if(tag != IMetadata::IEntry::BAD_TAG)
                        EXPECT_TRUE(OK == result);
                    else
                        EXPECT_TRUE(OK != result);
                    break;
                }
        }
    };

    std::vector<IMetadata::IEntry> E;
    E.resize(MAX_ENTRY);

    std::vector<IMetadata> M;
    M.resize(MAX_METADATA);

    std::vector<IMetadata::Memory> A; // name from Array
    A.resize(MAX_MEMORY);

    auto threadTester = [&] (int id,int count) {
        std::vector<MUINT8>    B; //flatten buf for metadata. owned by self
        META_LOGD("treadTester%d use global Metadata(%d) and Entry(%d) with loop (%d)", id, (int)M.size(),(int)E.size(),count);
        for(int i=0;i<count;i++) {
            auto eIdx = (int)(std::rand() % E.size());
            auto &e = E[eIdx];
            auto mIdx = (int)(std::rand() % M.size());
            auto &m = M[mIdx];
            auto mIdx1 = (int)(std::rand() % M.size());
            auto &m1 = M[mIdx1];
            auto mIdx2 = (int)(std::rand() % M.size());
            auto &m2 = M[mIdx2];

            META_LOGD("Stat:Content %zu obj, Storage %zu obj",
                    IMetadata::Content::mStatNum, IMetadata::Storage::mStatNum);
            META_LOGD("T%d Entry[%d](tag:%x, type:%x, cnt:%d) M[%d](cnt:%d) M1[%d](cnt:%d) M2[%d](cnt:%d)",
                    id,
                    eIdx,e.tag(),e.type(),e.count(),
                    mIdx,m.count(),
                    mIdx1,m1.count(),
                    mIdx2,m2.count());

            randEntryOP(m,e);
            randMetadataOP(m1,m2,e);
        }
        META_LOGD("treadTester%d Finish running ", (int)id);
    };

    std::vector<std::thread> workers;
    for(int id=0;id<MAX_THREAD;id++) {
        workers.push_back(std::thread(threadTester,id,MAX_LOOP));
    }
    std::for_each(workers.begin(), workers.end(), [] (std::thread &t) {t.join();});

    META_LOGD("IMetadata, Case 99 end===============");
}
#endif
#endif
int main(int argc, char **argv)
{
    NSCam::Utils::ULog::ULogInitializer ulogInit; // The constructor will init ULog
    std::srand(0);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
