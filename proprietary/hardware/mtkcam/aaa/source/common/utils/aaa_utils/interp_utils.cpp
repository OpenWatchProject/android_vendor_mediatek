#include "interp_utils.h"

template <class TX, class TY>
TY interpLine(TX x1, TY y1, TX x2, TY y2, TX x)
{
    return y1 + (y2 - y1) / (x2 - x1) * (x - x1);
}
template int interpLine<int, int>(int x1, int y1, int x2, int y2, int x);
template float interpLine<int, float>(int x1, float y1, int x2, float y2, int x);
template float interpLine<float, float>(float x1, float y1, float x2, float y2, float x);

template <class TX, class TY>
TY interpTable(int n, TX *xNode, TY *yNode, TX x)
{
    int i;
    TY y = yNode[0];
    TX xStart = xNode[0];
    TX xEnd;
    TY yStart = yNode[0];
    TY yEnd;

    if (x <= xNode[0]) {
        /* less than xMin */
        x = xNode[0];
        y = yNode[0];
    } else if (x >= xNode[n - 1]) {
        /* larger than xMax */
        x = xNode[n - 1];
        y = yNode[n - 1];
    } else {
        /* in the middle of the table */
        for (i = 1; i < n; i++) {
            xEnd = xNode[i];
            yEnd = yNode[i];
            if (x <= xEnd) {
                y = interpLine(xStart, yStart, xEnd, yEnd, x);
                break;
            }
            xStart = xEnd;
            yStart = yEnd;
        }
    }

    return y;
}
template int interpTable<int, int>(int n, int *xNode, int *yNode, int x);
template float interpTable<int, float>(int n, int *xNode, float *yNode, int x);
template float interpTable<float, float>(int n, float *xNode, float *yNode, float x);


