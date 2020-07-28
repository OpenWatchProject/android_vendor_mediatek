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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef FAST_LOGGER_H_
#define FAST_LOGGER_H_

#include <string.h>             //strlen
#include <stdarg.h>             //vs_list...
#include <android/log.h>
#include <utility>              //Pair
#include <utils/Mutex.h>
#include <cutils/properties.h>
#include <cctype>
#include <string>

// #define FAST_LOGGER_DEBUG    // Enable debug log.

// Coding usage:
// 0. Make sure you have include vendor/mediatek/proprietary/hardware/mtkcam/mtkcam.mk in your Android.mk
//
// 1. Include FastLogger.h
// 2. Declare a FastLogger instance, the LOG_TAG and log property must be set:
//     e.g. FastLogger logger(LOG_TAG, "debug.xxx.log");
// 3. Add macro:
//    #define FAST_LOGD(fmt, arg...)  logger.FastLogD("[%s]" fmt, __func__, ##arg)
//    #define FAST_LOGI(fmt, arg...)  logger.FastLogI("[%s]" fmt, __func__, ##arg)
//    #define FAST_LOGW(fmt, arg...)  logger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
//    #define FAST_LOGE(fmt, arg...)  logger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
// 4. Replace MY_LOGD/CAM_LOGD/ALOGD with FAST_LOGD
// 5. Remember to call logger.print() in the end
//
// Tricky usage example:
//    FastLogger(LOG_TAG, "debug.xxx.log")
//    .FastLogD("[%s] %s", __func__, "123")
//    .FastLogD("[%s] %s", __func__, "456")
//    .FastLogD("[%s] %s", __func__, "789")
//    .print();
//
// In runtime:
// You must enable the log property first, and run the process
#define FAST_LOGGER_TAG "FastLogger"

class FastLogger
{
public:
    /**
     * \brief Construct FastLogger. Default is single line mode.
     *
     * \param TAG Pass LOG_TAG of each module
     * \param LOG_PROPERTY The property to turn on log
     * \param LOG_VALUE The value to enable log, default is 1, must >= 1
     */
    FastLogger(const char *TAG, const char *LOG_PROPERTY, const int LOG_VALUE=1)
        : _LOG_PROPERTY(LOG_PROPERTY)
        , _LOG_VALUE(LOG_VALUE)
    {
        setSingleLineMode(true);
        _setTAG(TAG);
        updateLogStatus();
    }

    /**
     * \brief Default destructor of FastLogger
     * \details We'll print rest cached log if user does not call print() last time.
     */
    virtual ~FastLogger()
    {
        _flush(_logBuffer);

        if(_logBuffer) {
            delete [] _logBuffer;
            _logBuffer = NULL;
        }

        if(__lineBuffer) {
            delete [] __lineBuffer;
            __lineBuffer = NULL;
        }
    }

    virtual FastLogger &updateLogStatus()
    {
        __isLogEnabled = ( _TAG.length() > 0 &&
                           _checkLogProperty(_LOG_PROPERTY.c_str(), _LOG_VALUE) );
        return *this;
    }

    /**
     * \brief Set FastLogger to single line mode. We'll also trim whitespace in single line mode.
     * \details Single line mode will put semicolon(;) in the end of each line instead of new line(\n)
     *
     * \param isSingleLine true to switch to single line mode, false to default mode
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").setSingleLineMode().FastLogD("123").FastLogD("456").print();
     */
    virtual FastLogger &setSingleLineMode(bool isSingleLine=true)
    {
        android::Mutex::Autolock lock(__logLock);
        // _isTrimWhiteSpace = true;
        _endl = (isSingleLine) ? ';' : '\n';

        return *this;
    }

    /**
     * \brief Set to trim whitespace to reduce log size
     * \details We'll:
     *          1. Trim head whitespace
     *          2. Trim tail whitespace
     *          3. Replace multiple white space with single one
     *
     * \param trimWhiteSpace true to trim whitespace
     * \return [The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").setTrimWhitespace().FastLogD("123").FastLogD("456").print();
     */
    virtual FastLogger &setTrimWhitespace(bool trimWhiteSpace=true)
    {
        android::Mutex::Autolock lock(__logLock);
        _isTrimWhiteSpace = trimWhiteSpace;

        return *this;
    }

    #define VA_LOG_(LOG_LEVEL) \
        do { \
            va_list args; \
            va_start(args, fmt); \
            _log(LOG_LEVEL, fmt, args); \
            va_end(args); \
        } while(0)

    /**
     * \brief Add debug log to cache
     * \details Log will be cached until:
     *          1. print() is called
     *          2. Another FastLog API(e.g. FastLogE) is called(Log level changes)
     *          3. Log cache is full
     *          4. FastLogger instance is released
     *
     *          This API is thread safe
     *
     * \param fmt,... The format to print, like printf
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogD("123").FastLogD("456").print();
     */
    virtual FastLogger& FastLogD(const char *fmt, ...)
    {
        VA_LOG_(ANDROID_LOG_DEBUG);
        return *this;
    }

    /**
     * \brief Add error log to cache
     * \details Log will be cached until:
     *          1. print() is called
     *          2. Another FastLog API(e.g. FastLogD) is called(Log level changes)
     *          3. Log cache is full
     *          4. FastLogger instance is released
     *
     *          This API is thread safe
     *
     * \param fmt,... The format to print, like printf
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogE("123").FastLogE("456").print();
     */
    virtual FastLogger& FastLogE(const char *fmt, ...)
    {
        VA_LOG_(ANDROID_LOG_ERROR);
        return *this;
    }

    /**
     * \brief Add warning log to cache
     * \details Log will be cached until:
     *          1. print() is called
     *          2. Another FastLog API(e.g. FastLogE) is called(Log level changes)
     *          3. Log cache is full
     *          4. FastLogger instance is released
     *
     *          This API is thread safe
     *
     * \param fmt,... The format to print, like printf
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogW("123").FastLogW("456").print();
     */
    virtual FastLogger& FastLogW(const char *fmt, ...)
    {
        VA_LOG_(ANDROID_LOG_WARN);
        return *this;
    }

    /**
     * \brief Add information log to cache
     * \details Log will be cached until:
     *          1. print() is called
     *          2. Another FastLog API(e.g. FastLogE) is called(Log level changes)
     *          3. Log cache is full
     *          4. FastLogger instance is released
     *
     *          This API is thread safe
     *
     * \param fmt,... The format to print, like printf
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogI("123").FastLogI("456").print();
     */
    virtual FastLogger& FastLogI(const char *fmt, ...)
    {
        VA_LOG_(ANDROID_LOG_INFO);
        return *this;
    }

    /**
     * \brief Add verbose log to cache
     * \details Log will be cached until:
     *          1. print() is called
     *          2. Another FastLog API(e.g. FastLogE) is called(Log level changes)
     *          3. Log cache is full
     *          4. FastLogger instance is released
     *
     *          This API is thread safe
     *
     * \param fmt,... The format to print, like printf
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogV("123").FastLogV("456").print();
     */
    virtual FastLogger& FastLogV(const char *fmt, ...)
    {
        VA_LOG_(ANDROID_LOG_VERBOSE);
        return *this;
    }

    #undef VA_LOG_

    /**
     * \brief Print cached log
     * \details This API is thread safe
     *
     * \return The reference of FastLogger.
     *         User can use this reference to begin next log.
     *         e.g. FastLogger(LOG_TAG, "log.property").FastLogV("123").FastLogV("456").print().FastLogE("789");
     */
    virtual FastLogger& print()
    {
        android::Mutex::Autolock lock(__logLock);
        _flush(_logBuffer);

        return *this;
    }

public:
    //The same size defined in system/core/liblog/logd_write_kern.c and logd_write.c:
    //#define LOG_BUF_SIZE 1024
    const static size_t MAX_LOG_SIZE = 1024;

protected:
    virtual bool _validateTAG(const char *TAG)
    {
        if(NULL == TAG ||
           0 == strlen(TAG))
        {
            __android_log_write(ANDROID_LOG_ERROR, FAST_LOGGER_TAG, "Invalid TAG");
            return false;
        }

        return true;
    }

    virtual bool _setTAG(const char *TAG)
    {
        android::Mutex::Autolock lock(__logLock);

        if( _validateTAG(TAG) ) {
            _TAG = std::string(TAG);
            return true;
        }

        return false;
    }

    virtual bool _checkLogProperty(const char* LOG_PROPERTY, const int LOG_VALUE)
    {
        android::Mutex::Autolock lock(__logLock);

        if( !_validateTAG(_TAG.c_str()) ) {
            return false;
        }

        if( LOG_VALUE <= 0 ||
            property_get_int32(LOG_PROPERTY, 0) != LOG_VALUE )
        {
            #ifdef FAST_LOGGER_DEBUG
            __android_log_write(ANDROID_LOG_DEBUG, _TAG.c_str(), "Log property not enabled, logger disabled");
            #endif

            if(_logBuffer) {
                delete [] _logBuffer;
                _logBuffer       = NULL;
                __currentLogSize = 0;
            }

            if(__lineBuffer) {
                delete [] __lineBuffer;
                __lineBuffer = NULL;
            }

            return false;
        } else {
            if(NULL == _logBuffer) {
                _logBuffer = new char[MAX_LOG_SIZE];
                ::memset(_logBuffer, 0, MAX_LOG_SIZE);
                __currentLogSize = 0;
            }

            if(NULL == __lineBuffer) {
                __lineBuffer = new char[MAX_LOG_SIZE];
            }
        }

        return true;
    }

    virtual void _log(const int LOG_LEVEL, const char *fmt, va_list args)
    {
        android::Mutex::Autolock lock(__logLock);

        if(!__isLogEnabled) {
            return;
        }

        if(LOG_LEVEL != __lastLogLevel) {
            _flush(_logBuffer);
        }

        __lastLogLevel = LOG_LEVEL;

        //1. Use printf to generate string
        size_t lineSize = vsnprintf(__lineBuffer, MAX_LOG_SIZE, fmt, args);
        if(0 == lineSize) {
            #ifdef FAST_LOGGER_DEBUG
                __android_log_write(ANDROID_LOG_ERROR, _TAG.c_str(), "[Error] Log failed");
            #endif
            return;
        }

        lineSize = WhitespaceTrimmer::trimWhitespace(__lineBuffer, WhitespaceTrimmer::TRIM_TAIL);

        if(_isTrimWhiteSpace &&
           LOG_LEVEL < ANDROID_LOG_WARN)
        {
            lineSize = WhitespaceTrimmer::trimWhitespace(__lineBuffer, WhitespaceTrimmer::TRIM_ALL);
        }

        //2. Save result to stream, and flush if needed
        if(lineSize < MAX_LOG_SIZE-1) {    //we will append endl, so the size must < MAX-1
            //append endl
            if(_endl != __lineBuffer[lineSize-1]) {
                __lineBuffer[lineSize]   = (LOG_LEVEL < ANDROID_LOG_WARN) ? _endl : '\n';
                __lineBuffer[lineSize+1] = 0;
                ++lineSize;
            }

            if(__currentLogSize + lineSize >= MAX_LOG_SIZE) {
                _flush(_logBuffer);
            }

            ::memcpy(_logBuffer+__currentLogSize, __lineBuffer, lineSize);
            __currentLogSize += lineSize;
            _logBuffer[__currentLogSize] = 0;
        } else {
            #ifdef FAST_LOGGER_DEBUG
            __android_log_print(ANDROID_LOG_WARN, _TAG.c_str(),
                                "Line size %d >= limit %d, some log will miss", lineSize, MAX_LOG_SIZE);
            #endif

            _flush(_logBuffer);
            _flush(__lineBuffer);
        }
    }

    virtual void _flush(const char *OUTPUT)
    {
        const size_t OUTPUT_SIZE = (OUTPUT == _logBuffer) ? __currentLogSize : strlen(OUTPUT);
        if(NULL == OUTPUT ||
           OUTPUT_SIZE <= 0)
        {
            return;
        }

        #ifdef FAST_LOGGER_DEBUG
            __android_log_write(ANDROID_LOG_DEBUG, FAST_LOGGER_TAG, "========= Print Log Begin =========");
        #endif

        __android_log_write(__lastLogLevel, _TAG.c_str(), OUTPUT);

        #ifdef FAST_LOGGER_DEBUG
            __android_log_write(ANDROID_LOG_DEBUG, FAST_LOGGER_TAG, "========= Print Log End =========");
        #endif

        if(OUTPUT == _logBuffer) {
            __currentLogSize = 0;
        }
    }

protected:
    class WhitespaceTrimmer
    {
    public:
        enum ENUM_TRIM_METHOD {
            TRIM_LEAD = 1,
            TRIM_TAIL = 2,
            TRIM_ALL  = 4,
        };

    public:
        static size_t trimWhitespace(char *text, ENUM_TRIM_METHOD method=TRIM_ALL)
        {
            if(NULL == text) {
                return 0;
            }

            size_t len = 0;
            char *src  = text;
            char *dst  = text;

            //Trim tail whitespace
            if((TRIM_TAIL & method) ||
               (TRIM_ALL  & method))
            {
                int tailPos = strlen(src) - 1;
                char *tail = src + tailPos;
                while( tailPos >= 0 && isSpace(*tail) ) {
                    *tail = 0;
                    --tailPos;
                    --tail;
                }

                len = tailPos + 1;
            }

            //Trim leading whitespace
            if(TRIM_LEAD & method) {
                while( isSpace(*src) ) {
                    ++src;
                }

                if(!(TRIM_ALL & method)) {
                    if(src == dst) {
                        len = strlen(src);
                    } else {
                        while(*src) {
                            *(dst+len) = *src;
                            ++len;
                        }
                    }
                }
            }

            if(TRIM_ALL & method) {
                // Replace multiple white space with single one
                while(*src) {
                    *(dst+len) = *src;
                    ++len;

                    if( !isSpace(*src) ) {
                        ++src;
                    } else {
                        ++src;  //trim from next whitespace
                        while(isSpace(*src)) {
                            ++src;
                        }
                    }
                }
            }

            *(dst+len) = 0;

            return len;
        }

        static inline bool isSpace(char c)
        {
            return ::isspace(c);
            // return (' '!=c && '\t'!=c) ? false : true;
        }
    };


protected:
    char *                  _logBuffer        = NULL;
    std::string             _TAG;
    const std::string       _LOG_PROPERTY;
    const int               _LOG_VALUE;
    char                    _endl             = '\n';
    bool                    _isTrimWhiteSpace = false;

private:
    mutable android::Mutex  __logLock;
    bool                    __isLogEnabled   = true;

    char                    *__lineBuffer    = NULL;
    size_t                  __currentLogSize = 0;
    int                     __lastLogLevel   = ANDROID_LOG_DEBUG;
};

#endif
