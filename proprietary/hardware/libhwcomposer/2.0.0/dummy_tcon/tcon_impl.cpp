#include "tcon_impl.h"

#include <string.h>

dummySwTcon::dummySwTcon(void)
    : m_width(0)
    , m_height(0)
    , m_src_ptr(NULL)
    , m_src_size(0)
    , m_count(0)
    , m_line_size(0)
{
}

void dummySwTcon::requestBufSize(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height)
{
    m_width = src_width;
    m_height = src_height;
    m_rate = src_rate;

    *dst_width = src_width;
    *dst_height = src_height;
}

void dummySwTcon::init(struct ProxyBuffer* buf, uint32_t num)
{
    if (m_count)
    {
        unsigned int line_height = m_height / m_count;
        m_line_size = line_height * m_width * 4;
    }
}

void dummySwTcon::release(struct ProxyBuffer* buf, uint32_t num)
{
}

unsigned char dummySwTcon::setSourceBuffer(int32_t mode, void *ptr, size_t size)
{
    m_src_ptr = ptr;
    m_src_size = size;

    return 0;
}

unsigned char dummySwTcon::getCookedBuffer(void *ptr, size_t size)
{
    unsigned char res = true;
    static int cnt = 0;
    size_t s = 0;
    if (size < m_src_size)
    {
        s = size;
    }
    else
    {
        s = m_src_size;
    }

    char *dst = (char*)ptr;
    memcpy(dst, m_src_ptr, s);
    if (m_count)
    {
        memset(dst + (m_line_size) * cnt, 0, m_line_size);
    }
    cnt++;
    usleep(20000);

    if (cnt >= m_count)
    {
        cnt = 0;
        res = false;
    }

    return res;
}

void dummySwTcon::reset(void)
{
}
