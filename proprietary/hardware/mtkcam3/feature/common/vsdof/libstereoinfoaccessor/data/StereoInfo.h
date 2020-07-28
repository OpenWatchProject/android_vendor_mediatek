#ifndef STEREO_INFO_H
#define STEREO_INFO_H

#include "types.h"
#include "BufferManager.h"

namespace stereo {

class Rect {
public:
    int left;
    int top;
    int right;
    int bottom;
    Rect() : left(0), top(0), right(0), bottom(0){}
    Rect(int l, int t, int r, int b) :
        left(l), top(t), right(r), bottom(b){}
};

// jpeg section
class Section {
public:
    // e.g. 0xffe1, exif
    int marker;
    // marker offset from start of file
    long offset;
    // app length, follow spec, include 2 length bytes
    int length;
    StereoString type;
    StereoBuffer_t buffer;

    Section():
        marker(0), offset(0), length(0), type("") {}
    Section(int markerV, long offsetV, int lengthV):
        marker(markerV), offset(offsetV), length(lengthV), type("") {}
};

}
#endif