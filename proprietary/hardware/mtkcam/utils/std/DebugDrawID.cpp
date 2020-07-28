#define LOG_TAG "MtkCam/DebugDrawID"

#include <mtkcam/utils/std/DebugDrawID.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <mtkcam/utils/std/Log.h>
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)

using namespace std;

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
    mNeedDraw = property_get_int32("debug.cam.drawid", 0) || property_get_int32("debug.cam.draw.en", 0);
    mDigit = property_get_int32("debug.cam.drawid.digit", digit);
    mDigit = mDigit ? mDigit : 1;
    mOffsetX = property_get_int32("debug.cam.drawid.x", x);
    mOffsetY = property_get_int32("debug.cam.drawid.y", y);
    mLinePixel = property_get_int32("debug.cam.drawid.size", linePixel);
    mLinePixel = mLinePixel ? mLinePixel : 1;
    mBG = property_get_int32("debug.cam.drawid.bg", bg);
    mFG = property_get_int32("debug.cam.drawid.fg", fg);

    if(mNeedDraw) {
        MY_LOGD("need(%d), digit(%d), offset(%d,%d), pixel(%d), bg(%d), fg(%d)", mNeedDraw, mDigit, mOffsetX, mOffsetY, mLinePixel, mBG, mFG);
	}
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

void DebugDrawID::draw(std::string str, unsigned offsetX, unsigned offsetY, char *buffer, unsigned width, unsigned height, unsigned stride, unsigned bufSize, unsigned linePixel, char bg, char fg) const
{
    const unsigned BPP = linePixel;
    const unsigned BG = bg, FG = fg;   // BG: backgroud color  FG: front color
    const int MASK_W = 6, MASK_H = 5;

    if( buffer == nullptr )
    {
        MY_LOGD("skip: no buffer");
        return;
    }
    else if((width == 0) || (height == 0))
    {
        MY_LOGD("skip: width(%d) height(%d) ", width, height);
        return;
    }

    unsigned int last_pixel = ((offsetY+MASK_H*BPP-1)*stride) + offsetX + MASK_W*BPP*str.length();
    if( last_pixel >= bufSize )
    {
        MY_LOGD("Skip: exceed buffer size(%d) : length(%d), buffer stride(%d), mask(%dx%d), offset(%d,%d), BPP(%d)", bufSize, str.length(), stride, MASK_W, MASK_H, offsetX, offsetY, BPP);
        return;
    }


    typedef enum
    {
        IDX_0 = 0,
        IDX_1,
        IDX_2,
        IDX_3,
        IDX_4,
        IDX_5,
        IDX_6,
        IDX_7,
        IDX_8,
        IDX_9,
        IDX_A,
        IDX_B,
        IDX_C,
        IDX_D,
        IDX_E,
        IDX_F,
        IDX_G,
        IDX_H,
        IDX_I,
        IDX_J,
        IDX_K,
        IDX_L,
        IDX_M,
        IDX_N,
        IDX_O,
        IDX_P,
        IDX_Q,
        IDX_R,
        IDX_S,
        IDX_T,
        IDX_U,
        IDX_V,
        IDX_W,
        IDX_X,
        IDX_Y,
        IDX_Z,
    } MASK_INDEX;

    const char mask[36][MASK_W*MASK_H] = {
        {0,0,1,1,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,1,1,0}, // 0
        {0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0}, // 1
        {0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,0,0,1,1,1,0}, // 2
        {0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0}, // 3
        {0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0}, // 4
        {0,0,1,1,1,0,0,0,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0}, // 5
        {0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,0,0,0,1,0,1,0,0,0,1,1,1,0}, // 6
        {0,0,1,1,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0}, // 7
        {0,0,1,1,1,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,1,0,0,0,1,1,1,0}, // 8
        {0,0,1,1,1,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0}, // 9
        {0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0}, // A
        {1,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,0,0}, // B
        {0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,1,1,0,0}, // C
        {1,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,0}, // D
        {1,1,1,1,1,0,1,0,0,0,0,0,1,1,1,1,0,0,1,0,0,0,0,0,1,1,1,1,1,0}, // E
        {1,1,1,1,1,0,1,0,0,0,0,0,1,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0}, // F
        {0,1,1,1,1,0,1,0,0,0,0,0,1,0,1,1,1,0,1,0,0,1,1,0,0,1,1,0,1,0}, // G
        {1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,1,0}, // H
        {1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0}, // I
        {1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0}, // J
        {1,0,0,0,1,0,1,0,0,1,0,0,1,1,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0}, // K
        {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0}, // L
        {1,0,0,0,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0}, // M
        {1,0,0,0,1,0,1,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0}, // N
        {0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,1,1,0,0}, // O
        {1,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0}, // P
        {0,1,1,1,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,0,1,1,1,1,0}, // Q
        {1,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0}, // R
        {0,1,1,1,1,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,0,1,1,1,1,0,0}, // S
        {1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0}, // T
        {1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,1,1,0,0}, // U
        {1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0}, // V
        {1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0}, // W
        {1,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,0,0,1,0}, // X
        {1,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0}, // Y
        {1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0}, // Z
    };


    for(unsigned int y=0; y<MASK_H*BPP; y++)
    {
        char *pImgY = buffer + ((offsetY+y)*stride) + offsetX;

        for( auto _c : str)
        {
            char c  = toupper(_c);
            unsigned int idx = -1;
            if('0'<=c && c<='9')
                idx = c - '0';
            else if ('A'<=c && c<='Z')
                idx = IDX_A + c - 'A';
            else
                continue;

            const char* ptr_mask2 = &(mask[idx][(y/BPP)*MASK_W]);

            for(unsigned int x=0; x<MASK_W; ++x)
            {
                if( ptr_mask2[x])
                    memset(pImgY, FG, BPP);
                else
                    memset(pImgY, BG, BPP);
                pImgY += BPP;

            }
        }
    }



}

