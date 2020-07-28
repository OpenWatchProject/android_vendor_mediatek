#ifndef _UTIL_IMG_BOUNDARY_EXT_H_
#define _UTIL_IMG_BOUNDARY_EXT_H_

#include "utilSystem/utilMemOp.h"

typedef class UTIL_IMG_BOUND
{
public:
    MINT32 top;
    MINT32 bottom;
    MINT32 left;
    MINT32 right;

    // constructor
    UTIL_IMG_BOUND() : top(0), bottom(0), left(0), right(0) {}
    UTIL_IMG_BOUND(MINT32 _t, MINT32 _b, MINT32 _l, MINT32 _r) : top(_t), bottom(_b), left(_l), right(_r) {}
    UTIL_IMG_BOUND(const UTIL_IMG_BOUND &data)
    {
        top = data.top;
        bottom = data.bottom;
        left = data.left;
        right = data.right;
    }

} UTIL_IMG_BOUND, *P_UTIL_IMG_BOUND;

template <typename DATA_TYPE>
void utilImgBoundExt(DATA_TYPE *p_img_ext, DATA_TYPE *p_img, MINT32 width, MINT32 height, UTIL_IMG_BOUND img_bnd)
{
    // variables
    MINT32 width_ext = width + img_bnd.left + img_bnd.right;
    MINT32 top_bound_size = img_bnd.top;
    MINT32 bottom_bound_size = img_bnd.bottom;
    MINT32 left_bound_size = img_bnd.left;
    MINT32 right_bound_size = img_bnd.right;

    //==========================//
    // boundary extension for Y //
    //==========================//
    // row 0
    for (MINT32 j=0; j<left_bound_size; j++)
    {
        *p_img_ext++ = p_img[0];
    }
    vmemcpy(p_img_ext, p_img, width*sizeof(DATA_TYPE));
    p_img_ext += width;
    for (MINT32 j=0; j<right_bound_size; j++)
    {
        *p_img_ext++ = p_img[width-1];
    }
    p_img += width;
    // row 1 ~ top_bound_size
    for (MINT32 i=0; i<top_bound_size; i++)
    {
        vmemcpy(p_img_ext, p_img_ext - width_ext, width_ext*sizeof(DATA_TYPE));
        p_img_ext += width_ext;
    }
    // row top_bound_size+1 ~ top_bound_size+height-1
    for (MINT32 i=0; i<height-1; i++)
    {
        for (MINT32 j=0; j<left_bound_size; j++)
        {
            *p_img_ext++ = p_img[0];
        }
        vmemcpy(p_img_ext, p_img, width*sizeof(DATA_TYPE));
        p_img_ext += width;
        for (MINT32 j=0; j<right_bound_size; j++)
        {
            *p_img_ext++ = p_img[width-1];
        }
        p_img += width;
    }
    // row top_bound_size+height ~ top_bound_size+bottom_bound_size+height-1
    for (MINT32 i=0; i<bottom_bound_size; i++)
    {
        vmemcpy(p_img_ext, p_img_ext - width_ext, width_ext*sizeof(DATA_TYPE));
        p_img_ext += width_ext;
    }
}

#endif /* _UTIL_IMG_BOUNDARY_EXT_H_ */
