#define LOG_TAG "MtkCam/DebugDrawID"

#include <utils/std/DebugDrawID.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <utils/std/Log.h>
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)

DebugDrawID* DebugDrawID::createInstance()
{
    return new DebugDrawID();
}

void DebugDrawID::destroyInstance()
{
    delete this;
}

DebugDrawID::DebugDrawID(unsigned digit, unsigned x, unsigned y, unsigned linePixel, char bg, char fg)
{
    mNeedDraw = property_get_int32("debug.cam.drawid", 0);
    mDigit = property_get_int32("debug.cam.drawid.digit", digit);
    mDigit = mDigit ? mDigit : 1;
    mOffsetX = property_get_int32("debug.cam.drawid.x", x);
    mOffsetY = property_get_int32("debug.cam.drawid.y", y);
    mLinePixel = property_get_int32("debug.cam.drawid.size", linePixel);
    mLinePixel = mLinePixel ? mLinePixel : 1;
    mBG = property_get_int32("debug.cam.drawid.bg", bg);
    mFG = property_get_int32("debug.cam.drawid.fg", fg);

    MY_LOGD("need(%d), digit(%d), offset(%d,%d), pixel(%d), bg(%d), fg(%d)", mNeedDraw, mDigit, mOffsetX, mOffsetY, mLinePixel, mBG, mFG);
}

bool DebugDrawID::needDraw() const
{
    return mNeedDraw;
}

void DebugDrawID::draw(unsigned num, char *buffer, unsigned width, unsigned height, unsigned stride, unsigned bufSize) const
{
    if(mNeedDraw) {
        MY_LOGD("MagicNumber:(%d)", num);
    }
    draw(num, mDigit, mOffsetX, mOffsetY, buffer, width, height, stride, bufSize, mLinePixel, mBG, mFG);
}

void DebugDrawID::draw(unsigned num, unsigned digit, unsigned offsetX, unsigned offsetY, char *buffer, unsigned width, unsigned height, unsigned stride, unsigned bufSize) const
{
    draw(num, digit, offsetX, offsetY, buffer, width, height, stride, bufSize, mLinePixel, mBG, mFG);
}

void DebugDrawID::draw(unsigned num, unsigned digit, unsigned offsetX, unsigned offsetY, char *buffer, unsigned width, unsigned height, unsigned stride, unsigned bufSize, unsigned linePixel, char bg, char fg)
{
    // x 0 1 2 x
    // x 3 4 5 x
    // x 6 7 8 x
    // x 9 a b x
    // x c d e x
    const int MASK_W = 5, MASK_H = 5;
    const char mask[10][MASK_W*MASK_H] = {
    //  x 0 1 2 x x 3 4 5 x x 6 7 8 x x 9 a b x x c d e x
      { 0,1,1,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,1,1,0 }, // 0
      { 0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0 }, // 1
      { 0,1,1,1,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,1,1,1,0 }, // 2
      { 0,1,1,1,0,0,0,0,1,0,0,1,1,1,0,0,0,0,1,0,0,1,1,1,0 }, // 3
      { 0,1,0,1,0,0,1,0,1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,0 }, // 4
      { 0,1,1,1,0,0,1,0,0,0,0,1,1,1,0,0,0,0,1,0,0,1,1,1,0 }, // 5
      { 0,1,0,0,0,0,1,0,0,0,0,1,1,1,0,0,1,0,1,0,0,1,1,1,0 }, // 6
      { 0,1,1,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0 }, // 7
      { 0,1,1,1,0,0,1,0,1,0,0,1,1,1,0,0,1,0,1,0,0,1,1,1,0 }, // 8
      { 0,1,1,1,0,0,1,0,1,0,0,1,1,1,0,0,0,0,1,0,0,1,1,1,0 }, // 9
    };

    const unsigned BPP = linePixel;
    const unsigned BG = bg, FG = fg;   // BG: backgroud color  FG: front color
    const unsigned BMP_WIDTH = digit * MASK_W * BPP;
    const unsigned BMP_HEIGHT = MASK_H * BPP;
    char bmp[MASK_H][BMP_WIDTH];
    unsigned numArray[digit];

    if( buffer == NULL )
    {
        MY_LOGD("Skip: No buffer");
        return;
    } else if((width == 0) || (height == 0))
    {
        MY_LOGD("Skip: width(%d) height(%d) ", width, height);
        return;
    }

    unsigned max = ((offsetY+BMP_HEIGHT-1)*stride) + offsetX + BMP_WIDTH;
    if( max >= bufSize )
    {
        MY_LOGD("Skip: exceed bufSize(%d): digit(%d) stride(%d) (%dx%d)@(%d,%d) BPP(%d)", bufSize, digit, stride, MASK_W, MASK_H, offsetX, offsetY, BPP);
        return;
    }


    unsigned temp = num;
    for( unsigned i = 0; i < digit; ++i )
    {
        numArray[digit-1-i] = temp%10;
        temp /= 10;
    }

    memset(bmp[0], BG, sizeof(bmp));
    for( unsigned y = 0; y < MASK_H; ++y )
    {
        char *ptr = bmp[y];
        unsigned offset = y*MASK_W;
        for( unsigned d = 0; d < digit; ++d )
        {
            for( unsigned x = 0; x < MASK_W; ++x )
            {
                if( mask[numArray[d]][offset+x] )
                {
                    memset(ptr, FG, BPP);
                }
                ptr += BPP;
            }
        }
    }

    unsigned offset = (offsetY*stride) + offsetX;
    for( unsigned y = 0; y < MASK_H; ++y )
    {
        for( unsigned i = 0; i < BPP; ++i )
        {
            memcpy(buffer+offset, bmp[y], BMP_WIDTH);
            offset += stride;
        }
    }
}
