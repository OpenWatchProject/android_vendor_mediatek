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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_REFLECTION_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_REFLECTION_H_

#include <iostream>
#include <iomanip>
#include <type_traits>

/******************************************************************************
 * Variadic Macro
 ******************************************************************************/
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, N, ...) N
#define COUNT_VARARGS(...) _GET_NTH_ARG(__VA_ARGS__, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define _FOREACH_0(_call, i, ...)
#define _FOREACH_1(_call, i, x)         _call(i, x)
#define _FOREACH_2(_call, i, x, ...)    _call(i, x) _FOREACH_1(_call, i + 1, __VA_ARGS__)
#define _FOREACH_3(_call, i, x, ...)    _call(i, x) _FOREACH_2(_call, i + 1, __VA_ARGS__)
#define _FOREACH_4(_call, i, x, ...)    _call(i, x) _FOREACH_3(_call, i + 1, __VA_ARGS__)
#define _FOREACH_5(_call, i, x, ...)    _call(i, x) _FOREACH_4(_call, i + 1, __VA_ARGS__)
#define _FOREACH_6(_call, i, x, ...)    _call(i, x) _FOREACH_5(_call, i + 1, __VA_ARGS__)
#define _FOREACH_7(_call, i, x, ...)    _call(i, x) _FOREACH_6(_call, i + 1, __VA_ARGS__)
#define _FOREACH_8(_call, i, x, ...)    _call(i, x) _FOREACH_7(_call, i + 1, __VA_ARGS__)
#define _FOREACH_9(_call, i, x, ...)    _call(i, x) _FOREACH_8(_call, i + 1, __VA_ARGS__)
#define _FOREACH_10(_call, i, x, ...)   _call(i, x) _FOREACH_9(_call, i + 1, __VA_ARGS__)
#define _FOREACH_11(_call, i, x, ...)   _call(i, x) _FOREACH_10(_call, i + 1, __VA_ARGS__)
#define _FOREACH_12(_call, i, x, ...)   _call(i, x) _FOREACH_11(_call, i + 1, __VA_ARGS__)
#define _FOREACH_13(_call, i, x, ...)   _call(i, x) _FOREACH_12(_call, i + 1, __VA_ARGS__)
#define _FOREACH_14(_call, i, x, ...)   _call(i, x) _FOREACH_13(_call, i + 1, __VA_ARGS__)
#define _FOREACH_15(_call, i, x, ...)   _call(i, x) _FOREACH_14(_call, i + 1, __VA_ARGS__)
#define _FOREACH_16(_call, i, x, ...)   _call(i, x) _FOREACH_15(_call, i + 1, __VA_ARGS__)
#define _FOREACH_17(_call, i, x, ...)   _call(i, x) _FOREACH_16(_call, i + 1, __VA_ARGS__)
#define _FOREACH_18(_call, i, x, ...)   _call(i, x) _FOREACH_17(_call, i + 1, __VA_ARGS__)
#define _FOREACH_19(_call, i, x, ...)   _call(i, x) _FOREACH_18(_call, i + 1, __VA_ARGS__)
#define _FOREACH_20(_call, i, x, ...)   _call(i, x) _FOREACH_19(_call, i + 1, __VA_ARGS__)
#define _FOREACH_21(_call, i, x, ...)   _call(i, x) _FOREACH_20(_call, i + 1, __VA_ARGS__)
#define _FOREACH_22(_call, i, x, ...)   _call(i, x) _FOREACH_21(_call, i + 1, __VA_ARGS__)
#define _FOREACH_23(_call, i, x, ...)   _call(i, x) _FOREACH_22(_call, i + 1, __VA_ARGS__)
#define _FOREACH_24(_call, i, x, ...)   _call(i, x) _FOREACH_23(_call, i + 1, __VA_ARGS__)
#define _FOREACH_25(_call, i, x, ...)   _call(i, x) _FOREACH_24(_call, i + 1, __VA_ARGS__)
#define _FOREACH_26(_call, i, x, ...)   _call(i, x) _FOREACH_25(_call, i + 1, __VA_ARGS__)
#define _FOREACH_27(_call, i, x, ...)   _call(i, x) _FOREACH_26(_call, i + 1, __VA_ARGS__)
#define _FOREACH_28(_call, i, x, ...)   _call(i, x) _FOREACH_27(_call, i + 1, __VA_ARGS__)

#define FOR_EACH(_macro, ...) \
        _GET_NTH_ARG(__VA_ARGS__, \
        _FOREACH_28, _FOREACH_27, _FOREACH_26, _FOREACH_25, \
        _FOREACH_24, _FOREACH_23, _FOREACH_22, _FOREACH_21, \
        _FOREACH_20, _FOREACH_19, _FOREACH_18, _FOREACH_17, \
        _FOREACH_16, _FOREACH_15, _FOREACH_14, _FOREACH_13, \
        _FOREACH_12, _FOREACH_11, _FOREACH_10, _FOREACH_9, \
        _FOREACH_8, _FOREACH_7, _FOREACH_6, _FOREACH_5, \
        _FOREACH_4, _FOREACH_3, _FOREACH_2, _FOREACH_1) \
        (_macro, 0, ##__VA_ARGS__);

/******************************************************************************
 *
 ******************************************************************************/
#define _STR(s) #s
#define STRINGIZE(s) _STR(s)
#define REM(...) __VA_ARGS__
#define EAT(...)

#define TYPEOF(x) DETAIL_TYPEOF(DETAIL_TYPEOF_PROBE x,)
#define DETAIL_TYPEOF(...) DETAIL_TYPEOF_HEAD(__VA_ARGS__)
#define DETAIL_TYPEOF_HEAD(x, ...) REM x
#define DETAIL_TYPEOF_PROBE(...) (__VA_ARGS__),
#define STRIP(x) EAT x
#define PAIR(x) REM x
#define FIELDS(...) \
static const int fields_n = COUNT_VARARGS(__VA_ARGS__); \
friend struct Reflector; \
template<int N, class T> \
struct Field {}; \
FOR_EACH(FIELD_EACH, __VA_ARGS__)

#define FIELD_EACH(i, x) \
PAIR(x); \
template<class T> \
struct Field<i, T> \
{ \
    T& t; \
    Field(T& t) : t(t) {} \
    TYPEOF(x) get() { return t.STRIP(x); } \
    const char* name() { return STRINGIZE(STRIP(x)); } \
};

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

/******************************************************************************
 * Reflection
 ******************************************************************************/
struct Reflector {
    template<int N, class T>
    static typename T::template Field<N, T> GetField(T& x) {
        return typename T::template Field<N, T>(x);
    }

    template<class T>
    struct Fields {
        static const int n = T::fields_n;
    };

    template <int N>
    struct int_{ };

    template<int I, class T, class Visitor>
    static void Iterate(T& t, Visitor& v, int_<I>) {
        Iterate(t, v, int_<I - 1>());
        v(Reflector::GetField<I>(t));
    }

    template<class T, class Visitor>
    static void Iterate(T& t, Visitor& v, int_<0>) {
        v(Reflector::GetField<0>(t));
    }

    template<class T, class Visitor>
    static void ForEach(T& t, Visitor&& v) {
        Iterate(t, v, int_<Fields<T>::n - 1>());
    }

};

/******************************************************************************
 *
 ******************************************************************************/
struct FieldPrinter
{
    FieldPrinter(std::ostream& s) : os(s) {}

    template<typename Field>
    void operator()(Field&& f) {

        typedef typename std::conditional<
                std::is_same<MUINT8, decltype(f.get())>::value,
                MINT32, decltype(f.get())>::type ConvertedFieldType;

        os << "    ."
           << std::left << std::setfill(' ') << std::setw(14)
           << f.name()
           << ": ";

        if (std::is_same<MUINT64, decltype(f.get())>::value) {
            os.setf(std::ios_base::hex,  std::ios_base::basefield);
            os.setf(std::ios_base::internal, std::ios_base::adjustfield);
            os.setf(std::ios_base::showbase);
            os << std::setfill('0')
               << std::setw(18)
               << f.get()
               << std::setw(0);
            os.setf(std::ios_base::dec , std::ios_base::basefield);
            os.unsetf(std::ios_base::showbase);
        } else if (std::is_class<decltype(f.get())>::value) {
            os << f.get();
        } else {
            os << (ConvertedFieldType)f.get();
        }
        os << std::endl;
    }

    std::ostream& os;
};


/******************************************************************************
*
******************************************************************************/
};  //namespace NSPipelinePlugin
};  //namespace NSCam
#endif //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_REFLECTION_H_

