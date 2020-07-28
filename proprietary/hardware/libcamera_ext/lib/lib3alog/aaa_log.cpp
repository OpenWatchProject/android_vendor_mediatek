#include <aaa_log.h>
#include <string>

bool clog3A::bEn = 0;
clog3A::_init clog3A::_initializer;

void blog_print(BLog &blog, const char *tag, const char *fmt, ...)
{
    Mutex::Autolock _l(blog.mLock);
    char* buf = blog.mBuf;
    size_t& totLen = blog.mLen;
    //
    va_list arg;
    va_start(arg, fmt);
    size_t const freeLen = BUF_REAL_SIZE - totLen;
    size_t len = vsnprintf(buf + totLen, freeLen, fmt, arg);
    va_end(arg);
    //
    totLen += len;
    if (totLen > BUF_REAL_SIZE) {
        __android_log_print(ANDROID_LOG_ERROR, tag,
            "too many log, may lost information %zu > %d",
            totLen, BUF_REAL_SIZE);
        totLen = BUF_REAL_SIZE;
    }
    //
    while(totLen > BUF_THRESHOLD){
        __android_log_print(blog.mLevel, tag, "%.*s", BUF_THRESHOLD, buf);
        totLen -= BUF_THRESHOLD;
        memmove(buf, buf + BUF_THRESHOLD, totLen);
    }
    buf[totLen] = '\0';
}

void blog_flush(BLog &blog, const char *tag)
{
    Mutex::Autolock _l(blog.mLock);
    if(blog.mLen != 0) {
        __android_log_print(blog.mLevel, tag, "%s", blog.mBuf);
        blog.mLen = 0;
    }
}
int blog_isUserLoad()
{
    return BLOG_USER_LOAD;
}


