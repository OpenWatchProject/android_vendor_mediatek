#include "pngconf.h"

void png_read_filter_row_neon(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row, int filter);

void png_read_sub_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row);
void png_read_up_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row);
void png_read_avg_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row);
void png_read_paeth_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row);
void sub_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void sub_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row,png_const_bytep prev_row);
void sub_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void sub_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void sub_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void sub_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void up_filter_neon64(png_size_t rowbytes,png_bytep row, png_const_bytep prev_row);
void avg_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void avg_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void avg_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void avg_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void avg_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void avg_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);
void paeth_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row);

void png_read_filter_row_neon(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row, int filter) {

	switch (filter) {
	case 0:
		break;
	case 1:{
		png_read_sub_filter(rowbytes,pixel_depth,row,prev_row);
		break;
	}
	case 2:{
		png_read_up_filter(rowbytes,pixel_depth,row,prev_row);
		break;
	}
	case 3:{
		png_read_avg_filter(rowbytes,pixel_depth,row,prev_row);
		break;
	}
	case 4:{
		png_read_paeth_filter(rowbytes,pixel_depth,row,prev_row);
		break;
	}
	default:
		break;
	}
}

void png_read_sub_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row) {
	unsigned int bpp = (pixel_depth + 7) >> 3;//bpps;//(pixel_depth + 7) >> 3;

	switch (bpp) {
	case 1:
	{
		sub_filter_1bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 2:
	{
		sub_filter_2bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 3:
	{
		sub_filter_3bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 4:
	{
		sub_filter_4bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 6:
	{
		sub_filter_6bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 8:
	{
		sub_filter_8bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	default:
	break;
	}
}

void png_read_up_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row){
	up_filter_neon64(rowbytes, row, prev_row);
}

void png_read_avg_filter(png_size_t rowbytes, png_byte pixel_depth,	png_bytep row, png_const_bytep prev_row) {
	unsigned int bpp = (pixel_depth + 7) >> 3;
	switch (bpp) {
		case 1:
		{
			avg_filter_1bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		case 2:
		{
			avg_filter_2bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		case 3:
		{
			avg_filter_3bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		case 4:
		{
			avg_filter_4bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		case 6:
		{
			avg_filter_6bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		case 8:
		{
			avg_filter_8bpp_neon64(rowbytes, row, prev_row);
			break;
		}
		default:
		break;
     }
}

void png_read_paeth_filter(png_size_t rowbytes, png_byte pixel_depth, png_bytep row, png_const_bytep prev_row) {
	unsigned int bpp = (pixel_depth + 7) >> 3;
	switch (bpp) {
	case 1:
	{
		paeth_filter_1bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 2:
	{
		paeth_filter_2bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 3:
	{
		paeth_filter_3bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 4:
	{
		paeth_filter_4bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 6:
	{
		paeth_filter_6bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	case 8:
	{
		paeth_filter_8bpp_neon64(rowbytes, row, prev_row);
		break;
	}
	default:
	break;
	}
}

void sub_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
       //use x5,x6,v0,v2,v16,v17,v18,v19,v21,v22
        asm volatile(
	            "mov        x5, #1                                     \n\t"
                "lsr        x6,%[rowbytes],#4                         \n\t"                // r1 = floor(rowbytes/4)
                                               //    = iteration countfor loop16
                "cbz        x6, sub_filter_1bpp_16bytes_done          \n\t"//use this to debug.

                "movi       v21.8B, #0                                \n\t"
                "ld1        {v16.8B,v17.8B}, [%[row]]                 \n\t"             // load 16 pixels
                                               // d16 = a b c d e f g h
                                               // d17 = i j k l m n o p
                "mov        x5, #0                                    \n\t"
                "sub_filter_1bpp_16bytes:                             \n\t"

                "shl        d18, d16, #8                              \n\t"          // d18 = 0 a b c d e f g
                "add        v18.8B, v16.8B, v18.8B                    \n\t"              // d18 = a a+b b+c c+d d+e e+f f+g g+h

                "shl        d18, d18, #8                              \n\t"           // d18 = 0 a a+b b+c c+d d+e e+f f+g
                "add        v18.8B, v16.8B, v18.8B                    \n\t"              // d18 = a a+b a+b+c b+c+d c+d+e d+e+f e+f+g f+g+h

                "shl        d18, d18, #8                              \n\t"         // shift add continuously to propage the sum of previous
                "add        v18.8B,v16.8B, v18.8B                     \n\t"             // and current pixels

                "shl        d18, d18, #8                              \n\t"
                "add        v18.8B, v16.8B, v18.8B                    \n\t"

                "shl        d18, d18, #8                              \n\t"
                "add        v18.8B, v16.8B, v18.8B                    \n\t"

                "shl        d18, d18, #8                              \n\t"
                "add        v18.8B, v16.8B, v18.8B                    \n\t"

                "shl        d18, d18, #8                              \n\t"
                "add        v18.8B, v16.8B, v18.8B                    \n\t"              // maximum data size for shift is 64 bits i.e. doubleword.
                                               // after computing thh value of all the pixels in the double word
                                               // extract the last computed value which will be used by
                                               // the next set of pixels(i.e next doubleword)
                "ext        v22.8B, v18.8B, v21.8B, #7                \n\t"          // extract the updated value of d18[7] i.e a+b+c+d+e+f+h
                "add        v17.8B, v17.8B, v22.8B                    \n\t"             // d17 = a+b+c+d+e+f+g+h+i j k l m n o p

                "shl        d19, d17, #8                              \n\t"            // continue shift-add as the first half
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #8                              \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "st1        {v18.8B,v19.8B},[%[row]],#16              \n\t"             // store the result back

                "add        x5, x5, #16                               \n\t"                // add 16 to the loop counter(no of bytes completed)
                "subs       x6,x6,#1                                  \n\t"                  // decrement iteration count
                "beq        sub_filter_1bpp_16bytes_adjust            \n\t"


                "ext        v22.8B, v19.8B, v21.8B, #7                \n\t"            // more iterations to go
                                                // extract the last computed value
                "ld1        {v16.8B,v17.8B}, [%[row]]                 \n\t"            // load the next 16 bytes
                "add        v16.8B, v16.8B, v22.8B                    \n\t"           // set up the input by adding the previous pixel
                                                // value to the input
                "b sub_filter_1bpp_16bytes                            \n\t"

                "sub_filter_1bpp_16bytes_adjust:                      \n\t"

                "cmp        x5, %[rowbytes]                           \n\t"                   // no more pixels left .. exit
                "sub        %[row], %[row], #1                        \n\t"             // more pixels remaining
                                                // r2 points to the current pixel adjust it
                                                // so that it points to the prev pixel for the below loop
                "beq        sub_filter_DONE_1bpp                      \n\t"

                "sub_filter_1bpp_16bytes_done:                        \n\t"


                "ld1        {v0.B}[0],[%[row]],#1                     \n\t"               // load 1 byte (1 pixel) into D0[0]
                                                // increment row pointer
                "sub_filter_1bpp_loop:                                \n\t"
                "add        x5,x5,#1                                  \n\t"                // loop counter += bpp=1
                "cmp        x5,%[rowbytes]                            \n\t"

                "ld1        {v2.B}[0],[%[row]]                        \n\t"                 // load 1 byte (current pixel) into D2[0]

                "add        v0.8B,v0.8B,v2.8B                         \n\t"                   // vector add 1 byte of previous pixel with
                                                //            1 byte of current pixel
                "st1        {v0.B}[0],[%[row]],#1                     \n\t"                // store 1 byte (updated pixel) back
                                                //  into row pointer location and increment
                                                //  row pointer

                "bne        sub_filter_1bpp_loop                      \n\t"        // loop back until loop counter == rowbytes

                "sub_filter_DONE_1bpp:                                \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "x5", "x6", "v0", "v2", "v16", "v17", "v18", "v19", "v21", "v22"
        );
}
void sub_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row,png_const_bytep prev_row){
        asm volatile(
	            "mov        x5, #2                                     \n\t" 
                "lsr        x6,%[rowbytes],#4                         \n\t"             // r1 = floor(rowbytes/4)
	                                                //    = iteration count for loop16
                "cbz        x6,sub_filter_2bpp_16bytes_done           \n\t" //use this to debug.

                "movi       v21.8B, #0                                \n\t"
                "ld1        {v16.8B,v17.8B}, [%[row]]                 \n\t"           // load 16 bytes to q8
	                                                // d16 = a b c d e fgh
	                                                // d17 = i j k l m nop
                "mov        x5, #0                                    \n\t"
                "sub_filter_2bpp_16bytes:                             \n\t"

                "shl        d18, d16, #16                             \n\t"             //  each pixel is 2bytes .. shift by 16 bits to get previous pixel
                "add        v18.8B, v16.8B, v18.8B                    \n\t"              //  add to the current pixel

                "shl        d18, d18, #16                             \n\t"        // shift-add to propagate the computed sum as the case for 1bpp
                "add        v18.8B, v16.8B, v18.8B                    \n\t"

                "shl        d18, d18, #16                             \n\t"
                "add        v18.8B, v16.8B, v18.8B                    \n\t"


                "ext        v22.8B, v18.8B, v21.8B, #6                \n\t"           // extract the last computed value (i.e. last 2 bytes)
                "add        v17.8B, v17.8B, v22.8B                    \n\t"             // add the last computed pixel to the input

                "shl        d19, d17, #16                             \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #16                             \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"

                "shl        d19, d19, #16                             \n\t"
                "add        v19.8B, v17.8B, v19.8B                    \n\t"


                "st1        {v18.8B,v19.8B},[%[row]],#16              \n\t"           //16byte, store the result back


                "add        x5, x5, #16                               \n\t"         // add 16 to the loop counter(no of bytes completed)
                "subs       x6,x6,#1                                  \n\t"          // decrement iteration count
                "beq        sub_filter_2bpp_16bytes_adjust            \n\t"


                "ext        v22.8B, v19.8B, v21.8B, #6                \n\t"            // extract the last computed value
	                                                // add the last computed pixel to the input
                "ld1        {v16.8B,v17.8B}, [%[row]]                 \n\t"
                "add        v16.8B, v16.8B, v22.8B                    \n\t"

                "b sub_filter_2bpp_16bytes                            \n\t"


                "sub_filter_2bpp_16bytes_adjust:                      \n\t"

                "cmp        x5, %[rowbytes]                           \n\t"                   // no more pixels left .. exit
                "sub        %[row], %[row], #2                        \n\t"                 // more pixels remaining
	                                                // r2 points to the current pixel adjust it
	                                                // so that it pointsto the prev pixel for the below loop
                "beq        sub_filter_DONE_2bpp                      \n\t"

                "sub_filter_2bpp_16bytes_done:                        \n\t"

                "ld1        {v0.H}[0],[%[row]],#2                     \n\t"               // load 2 bytes (1 pixel) into D0[0]
	                                                // increment row pointer
                "sub_filter_2bpp_loop:                                \n\t"
                "add        x5,x5,#2                                  \n\t"                 // loop counter += bpp
                "cmp        x5,%[rowbytes]                            \n\t"

                "ld1        {v2.H}[0],[%[row]]                        \n\t"                 // load 2 bytes (current pixel) into D2[0]
                "add        v0.8B,v0.8B,v2.8B                         \n\t"                    // vector add 2 bytes of previous pixel with
	                                                //            2 bytes of current pixel
                "st1        {v0.H}[0],[%[row]], #2                    \n\t"             // store 2 bytes (updated pixel) back
	                                                //  into row pointerlocation and increment
	                                                //  row pointer

                "bne        sub_filter_2bpp_loop                      \n\t"      // loop back until loop counter == rowbytes
                "sub_filter_DONE_2bpp:                                \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "x5", "x6", "v0", "v2", "v16", "v17", "v18", "v19", "v21", "v22"
        );
}

void sub_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
        asm volatile(
                "movi       v3.8B,  #0                                \n\t"
                "cmp        %[rowbytes], #16                          \n\t"
	  	         "blt             sub_filter_3bpp_lessthan16 \n\t"
                "sub        %[rowbytes],  %[rowbytes],  #16           \n\t"
	  	         "sub_filter_3bpp_loop:     \n\t"
				          //over read?????????????
                "ld1        {v8.8B},    [%[row]],#8                   \n\t"
                "ld1        {v9.8B},    [%[row]]                      \n\t"
                "subs       %[row],%[row],#8                          \n\t"
				        // "mov         D9,v8.D[1]                            \n\t"
                "ext        v5.8B,  v8.8B, v9.8B, #3                  \n\t"
                "add        v0.8B,  v3.8B,  v8.8B                     \n\t"
                "ext        v6.8B,  v8.8B, v9.8B, #6                  \n\t"
                "add        v1.8B,  v0.8B,  v5.8B                     \n\t"
                "ext        v7.8B,  v9.8B, v9.8B, #1                  \n\t"
                "st1        {v0.S}[0],  [%[row]], #4                  \n\t"//x5
                "subs       %[row],%[row],#1                          \n\t"
                "add        v2.8B,  v1.8B,  v6.8B                     \n\t"
                "st1        {v1.S}[0],  [%[row]], #4                  \n\t" //x5
                "subs       %[row],%[row],#1                          \n\t"
                "add        v3.8B,  v2.8B,  v7.8B                     \n\t"
                "st1        {v2.S}[0],  [%[row]], #4                  \n\t"//x5
                "subs       %[row],%[row],#1                          \n\t"
				          //vst1.32         {d3[0]},  [r2], r12       //mark before
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"                // store 2 bytes (updated pixel) back
                "st1        {v3.B}[2],[%[row]],#1                     \n\t"              // store 1 byte (updated pixel) back
                "subs       %[rowbytes],  %[rowbytes],  #12           \n\t"
	  	         "bge             sub_filter_3bpp_loop                              \n\t"
                "add        %[rowbytes],  %[rowbytes],  #16           \n\t"
	  	        "sub_filter_3bpp_lessthan16:                                             \n\t"
                "cmp        %[rowbytes], #3                           \n\t"
	  	         "blt             sub_filter_3bpp_lessthan3                          \n\t"

                "sub        %[rowbytes],  %[rowbytes],  #3            \n\t"
	  	        "sub_filter_3bpp_loop_tail:                               \n\t"
				          //ld1         {v8.S}[0],    [%[row]]     // over read 1 byte, ????
                "ld1        {v8.H}[0],[%[row]],#2                     \n\t"//v8-128bit,
                "ld1        {v8.B}[2],[%[row]]                        \n\t"
                "subs       %[row], %[row], #2                        \n\t"
                "subs       %[rowbytes],  %[rowbytes],  #3            \n\t"
                "add        v3.8B,  v3.8B,  v8.8B                     \n\t"
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"              // store 2 bytes (updated pixel) back
	  	         " st1          {v3.B}[2],[%[row]],#1              \n\t"               // store 1 byte (updated pixel) back
	  	          "bge             sub_filter_3bpp_loop_tail                              \n\t"

	  	   	   	 "sub_filter_3bpp_lessthan3:                                               \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "v0", "v1", "v2", "v3","v5","v6","v7","v8","v9"
        );
}
 void sub_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
        asm volatile(
                "cmp        %[rowbytes], #16                          \n\t"
                "movi       v3.8B, #0                                 \n\t"
                "blt        sub_filter_4bbp_lessthan16                \n\t"//use this to debug
                "sub        %[rowbytes],  %[rowbytes],  #16           \n\t"
                "sub_filter_4bpp_loop:                                \n\t"
                "ld4r       {v4.2S,v5.2S,v6.2S,v7.2S}, [%[row]]       \n\t"
                "subs       %[rowbytes],  %[rowbytes],  #16           \n\t"
                "add        v0.8B,  v3.8B,  v4.8B                     \n\t"
                "add        v1.8B,  v0.8B,  v5.8B                     \n\t"
                "add        v2.8B,  v1.8B,  v6.8B                     \n\t"
                "add        v3.8B,  v2.8B,  v7.8B                     \n\t"
                "st4        {v0.S, v1.S, v2.S, v3.S}[0], [%[row]], #16\n\t"
                "bge        sub_filter_4bpp_loop                      \n\t"

                "add        %[rowbytes],  %[rowbytes],  #16           \n\t"

                "sub_filter_4bbp_lessthan16:                          \n\t"
                "cmp        %[rowbytes], #4                           \n\t"
                "blt        sub_filter_4bpp_lessthan4                 \n\t"

                "sub        %[rowbytes],  %[rowbytes],  #4            \n\t"

                "sub_filter_4bpp_loop_tail:                           \n\t"
                "ld1r       {v4.2S}, [%[row]]                         \n\t"
                "subs       %[rowbytes],  %[rowbytes],  #4            \n\t"
                "add        v3.8B,  v3.8B,  v4.8B                     \n\t"
                "st1        {v3.S}[0],[%[row]], #4                    \n\t"
                "bge        sub_filter_4bpp_loop_tail                 \n\t"
                "sub_filter_4bpp_lessthan4:                           \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7"
        );
}
void sub_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
        asm volatile(
                "ld1        {v0.S}[0],[%[row]],#4                     \n\t"
                "ld1        {v0.H}[2],[%[row]],#2                     \n\t" // load 8 bytes (1 pixel + 2 extra bytes) into D0
			                                                                // increment row pointer by bpp
                "mov        x5,#6                                     \n\t"
                "sub_filter_6bpp_loop:                                \n\t"
                "add        x5,x5,#6                                  \n\t"           // loop counter += bpp


			    //"ld1     {v1.8B},[%[row]]                              \n\t"            // load 8 bytes (1 pixel + 2 extra bytes) into D2
                "ld1        {v1.S}[0], [%[row]],#4                    \n\t"
                "ld1        {v1.H}[2], [%[row]]                       \n\t"
                "sub        %[row], %[row], #4                        \n\t"
                "add        v0.8B, v0.8B, v1.8B                       \n\t"               // vector add 6 bytes of previous pixel with
			                                               //            6 bytes of current pixel
                "st1        {v0.S}[0],[%[row]],#4                     \n\t"          // store 4 bytes (updated pixel) back
			                                               //  into row pointer location and increment
			                                               //  row pointer
                "st1        {v0.H}[2],[%[row]],#2                     \n\t"           // store 2 bytes (updated pixel) back
			                                               //  into row pointer location and increment
			                                               //  row pointer
    		    "cmp        x5,%[rowbytes]                             \n\t"
                "bne        sub_filter_6bpp_loop                      \n\t"      // loop back until loop counter == rowbytes

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "x5", "v0", "v1"
        );
}

void sub_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
        asm volatile(
                "ld1        {v0.8B},[%[row]],#8                       \n\t"
                "mov        x5,#8                                     \n\t"
                "1:                                                  \n\t"

                "add        x5,x5,#8                                  \n\t"                // loop counter += bpp

                "ld1        {v1.8B},[%[row]]                          \n\t"                 // load 8 bytes (current pixel) into D2
                "add        v0.8B,v0.8B,v1.8B                         \n\t"       // vector add 8 bytes of previous pixel with
		                                               //            8 bytesof current pixel
                "st1        {v0.8B},[%[row]],#8                       \n\t"               // store 8 bytes (updated pixel) back

                "cmp        x5,%[rowbytes]                            \n\t"
                "bne        1b                                        \n\t"

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row)
                :
                :"memory", "x5", "v0", "v1"
        );
}

void up_filter_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //v0,v1,v2,v3,x5
        asm volatile(
                "mov       x5,#0                                     \n\t"
                "lsr        x5,%[rowbytes],#5                         \n\t"               // r1 = floor(rowbytes/32)
	                                               //    = iteration countfor loop32
                "cbz        x5, up_filter_32bytes_proc_done           \n\t"


                "up_filter_32bytes_proc:                              \n\t"


                "ld1        {v0.16B, v1.16B},[%[prev_row]], #32       \n\t"                  // load 32 bytes from previous
                "ld1        {v2.16B, v3.16B},[%[row]]                 \n\t"                 // load 32 bytes from current row
                "prfm       PLDL1KEEP, [%[prev_row], #128]            \n\t"
                "prfm       PLDL1KEEP, [%[row], #128]                 \n\t"
                "subs       x5,x5,#1                                  \n\t"       // decrement iteration count

                "add        v0.16B,v2.16B,v0.16B                      \n\t"                   // vector add of 16 bytes
                "add        v1.16B,v3.16B,v1.16B                      \n\t"                   //
                "st1        {v0.16B, v1.16B},[%[row]], #32            \n\t"                //
	                                               //  and increment pointer
                "bne        up_filter_32bytes_proc                    \n\t"
                "and        %[rowbytes],%[rowbytes],#31               \n\t"                    // subtract 32 from rowbytes



                "up_filter_32bytes_proc_done:                         \n\t"

                "lsr        x5,%[rowbytes],#4                         \n\t"                    // r1 = floor(rowbytes/16)
	                                               //    = iteration countfor loop16
                "cbz        x5, up_filter_16bytes_proc_done           \n\t"

                "up_filter_16bytes_proc:                              \n\t"

                "ld1        {v0.16B}, [%[prev_row]], #16              \n\t"                // load 16 bytes from previous
	                                               //  row and incrementpointer
                "ld1        {v1.16B}, [%[row]]                        \n\t"               // load 16 bytes from current row
                "sub        %[rowbytes], %[rowbytes], #16             \n\t"                 // subtract 16 from rowbytes
                "add        v0.16B, v0.16B, v1.16B                    \n\t"                // vector add of 16 bytes
                "st1        {v0.16B}, [%[row]], #16                   \n\t"                 // store 16 bytes to current row
	                                               //  and increment pointer

                "up_filter_16bytes_proc_done:                         \n\t"

                "lsr        x5,%[rowbytes],#3                         \n\t"                // r1 = floor(rowbytes/8)
                "cbz        x5,up_filter_8bytes_proc_done             \n\t"

                "up_filter_8bytes_proc:                               \n\t"

                "ld1        {v0.8B}, [%[prev_row]], #8                \n\t"                  // load 8 bytes from previous
	                                               //  row and incrementpointer
                "ld1        {v1.8B}, [%[row]]                         \n\t"                // load 8 bytes from current row
                "sub        %[rowbytes], %[rowbytes], #8              \n\t"                     // subtract 8 from rowbytes
                "add        v0.8B, v0.8B, v1.8B                       \n\t"                   // vector add 8 bytes
                "st1        {v0.8B}, [%[row]], #8                     \n\t"                  // store 8 bytes to current row
	                                               //  and increment pointer

                "up_filter_8bytes_proc_done:                          \n\t"

                "lsr        x5, %[rowbytes] ,#2                       \n\t"                 // r1 = floor(rowbytes/4)
                "cbz        x5, up_filter_4bytes_proc_done            \n\t"

                "up_filter_4bytes_proc:                               \n\t"

                "ld1        {v0.S}[0], [%[prev_row]], #4              \n\t"             // load 4 bytes from previous row
	                                               //  and increment pointer
                "ld1        {v1.S}[0], [%[row]]                       \n\t"         // load 4 bytes from current row
                "sub        %[rowbytes], %[rowbytes], #4              \n\t"                  // subtract 4 from rowbytes
                "add        v0.8B, v0.8B, v1.8B                       \n\t"             // vector add 4 bytes
                "st1        {v0.S}[0], [%[row]], #4                   \n\t"             // store 4 bytes to current row
	                                               //  and increment pointer

                "up_filter_4bytes_proc_done:                          \n\t"

                "lsr        x5,%[rowbytes],#1                         \n\t"                // r1 = floor(rowbytes/2)
                "cbz        x5, up_filter_2bytes_proc_done            \n\t"

                "up_filter_2bytes_proc:                               \n\t"

                "ld1        {v0.H}[0], [%[prev_row]], #2              \n\t"            // load 2 bytes from previous row
	                                               //  and increment pointer
                "ld1        {v1.H}[0], [%[row]]                       \n\t"           // load 2 bytes from current row
                "sub        %[rowbytes], %[rowbytes], #2              \n\t"                    // subtract 2 from rowbytes
                "add        v0.8B, v0.8B, v1.8B                       \n\t"               // vector add 2 bytes
                "st1        {v0.H}[0], [%[row]], #2                   \n\t"              // store 2 bytes to current row
	                                               //  and increment pointer

                "up_filter_2bytes_proc_done:                          \n\t"

                "cmp        %[rowbytes],#0                            \n\t"
                "beq        up_filter_1byte_proc_done                 \n\t"

                "up_filter_1byte_proc:                                \n\t"      //use this to debug up.

                "ld1        {v0.B}[0],[%[prev_row]], #1               \n\t"               // load 1 byte from previous row
	                                               //  and increment pointer
                "ld1        {v1.B}[0],[%[row]]                        \n\t"           // load 1 byte from current row
                "add        v0.8B, v0.8B, v1.8B                       \n\t"               // vector add 1 byte
                "st1        {v0.B}[0], [%[row]], #1                   \n\t"          // store 1 byte to current row
	                                               //  and increment pointer
                "up_filter_1byte_proc_done:                           \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "x5", "v0", "v1","v2", "v3"
        );

}


void avg_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //use d0,d1,d2,d3, d4,d5,d6,d7, d16,d17,d18,d19
        asm volatile(
                "movi       v3.8B, #0                                 \n\t"
                "cmp        %[rowbytes], #4                           \n\t"
                "blt        avg_filter_1bpp_lessthan4                 \n\t" //use this to debug
                "sub        %[rowbytes],%[rowbytes], #4               \n\t"

                "avg_filter_1bpp_loop:                                \n\t"
                "ld4        {v4.B,v5.B,v6.B,v7.B}[0],    [%[row]]     \n\t"
                "ld4        {v16.B,v17.B,v18.B,v19.B}[0],[%[prev_row]],#4            \n\t"
                "subs       %[rowbytes],%[rowbytes], #4               \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"
                "uhadd      v1.8B,  v0.8B,  v17.8B                    \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
                "uhadd      v2.8B,  v1.8B,  v18.8B                    \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
                "uhadd      v3.8B,  v2.8B,  v19.8B                    \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
                "st4        {v0.B,v1.B,v2.B,v3.B}[0],[%[row]],#4      \n\t"
                "bgt        avg_filter_1bpp_loop                      \n\t"

                "add        %[rowbytes],%[rowbytes], #4               \n\t"
                "avg_filter_1bpp_lessthan4:                           \n\t"
                "cmp        %[rowbytes], #1                           \n\t"
                "blt        avg_filter_1bpp_lessthan1                 \n\t"
                "sub        %[rowbytes],%[rowbytes], #1               \n\t"

                "avg_filter_1bpp_loop_tail:                           \n\t"
                "ld1        {v4.B}[0], [%[row]]                       \n\t"
                "ld1        {v16.B}[0],[%[prev_row]], #1              \n\t"
                "subs       %[rowbytes],%[rowbytes], #1               \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "st1        {v3.B}[0],[%[row]],#1                     \n\t"
                "bge        avg_filter_1bpp_loop_tail                 \n\t"

                "avg_filter_1bpp_lessthan1:                           \n\t"

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory",  "v0", "v1", "v2", "v3","v4", "v5","v6", "v7","v16", "v17","v18", "v19"
        );

}
void avg_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	//use d0,d1,d2,d3, d4,d5,d6,d7,d16,d17,d18,d19
        asm volatile(
                "movi       v3.8B,  #0                                \n\t"
                "cmp        %[rowbytes], #8                           \n\t"
                "blt        avg_filter_2bpp_lessthan8                 \n\t"  //use this to debug.
                "sub        %[rowbytes], %[rowbytes], #8              \n\t"

                "avg_filter_2bpp_loop:                                \n\t"
                "ld4        {v4.H,v5.H,v6.H,v7.H}[0],    [%[row]]     \n\t"
                "ld4        {v16.H,v17.H,v18.H,v19.H}[0],[%[prev_row]],#8      \n\t"
                "subs       %[rowbytes], %[rowbytes], #8              \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"
                "uhadd      v1.8B,  v0.8B,  v17.8B                    \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
                "uhadd      v2.8B,  v1.8B,  v18.8B                    \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
                "uhadd      v3.8B,  v2.8B,  v19.8B                    \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
                "st4        {v0.H,v1.H,v2.H,v3.H}[0],[%[row]],#8      \n\t"
                "bgt        avg_filter_2bpp_loop                      \n\t"

                "add        %[rowbytes], %[rowbytes], #8              \n\t"
                "avg_filter_2bpp_lessthan8:                           \n\t"
                "cmp        %[rowbytes], #2                           \n\t"
                "blt        avg_filter_2bpp_lessthan2                 \n\t"
                "sub        %[rowbytes], %[rowbytes], #2              \n\t"

                "avg_filter_2bpp_loop_tail:                           \n\t"
                "ld1        {v4.H}[0], [%[row]]                       \n\t"
                "ld1        {v16.H}[0],[%[prev_row]],#2               \n\t"
                "subs       %[rowbytes], %[rowbytes], #2              \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"
                "bge        avg_filter_2bpp_loop_tail                 \n\t"

                "avg_filter_2bpp_lessthan2:                           \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "v0", "v1", "v2", "v3","v4", "v5","v6", "v7","v16", "v17","v18", "v19"
        );

}
void avg_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row) {
	     //old x29-->x5,x30->x6,use x5,x6, d0,d1,d2,d3,d5,d6,d7,q10,q11,d17,d18,d19,d21,d23,
        asm volatile(
                "movi       v3.8B,  #0                                \n\t"
                "mov        x6,  #12                                  \n\t"
                "cmp        %[rowbytes], #16                          \n\t"
                "blt        avg_filter_3bpp_lessthan16                \n\t"  //use this to debug.
                "sub        %[rowbytes], %[rowbytes], #16             \n\t"   // to avoid over read
                "avg_filter_3bpp_loop:                                \n\t"
	            // buffer over read?????????, if r0 = 3*5 = 15???
                "ld1        {v11.16B},    [%[row]]                    \n\t"
                "ld1        {v10.16B},    [%[prev_row]], x6           \n\t"
                "mov        D23,v11.D[1]                              \n\t"
                "mov        D21,v10.D[1]                              \n\t"
                "ext        v5.8B,  v11.8B, v23.8B, #3                \n\t"
                "uhadd      v0.8B,  v3.8B,  v10.8B                    \n\t"
                "ext        v17.8B, v10.8B, v21.8B, #3                \n\t"
                "add        v0.8B,  v0.8B,  v11.8B                    \n\t"
                "ext        v6.8B,  v11.8B, v23.8B, #6                \n\t"
                "uhadd      v1.8B,  v0.8B,  v17.8B                    \n\t"
                "ext        v18.8B, v10.8B, v21.8B, #6                \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
                "ext        v7.8B,  v23.8B, v23.8B, #1                \n\t"
	            //"st1         {v0.S}[0],  [%[row]], #3                     \n\t"      //#3bpp
                "st1        {v0.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v0.B}[2],  [%[row]], #1                  \n\t"

                "uhadd      v2.8B,  v1.8B,  v18.8B                    \n\t"
	            //"st1         {v1.S}[0],  [%[row]], #3                     \n\t"       //#3bpp
                "st1        {v1.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v1.B}[2],  [%[row]], #1                  \n\t"

                "ext        v19.8B, v21.8B, v21.8B, #1                \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
                "uhadd      v3.8B,  v2.8B,  v19.8B                    \n\t"
	            // "st1         {v2.S}[0],  [%[row]], #3                     \n\t"       //#3bpp
                "st1        {v2.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v2.B}[2],  [%[row]], #1                  \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
	            //vst1.32         {d3[0]},  [r2], r12  // over write 1 byte,NE
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"               // store 2 bytes (updated pixel x)
                "st1        {v3.B}[2],[%[row]],#1                     \n\t"             // store 1 byte (updated pixel x)
                "subs       %[rowbytes], %[rowbytes], #12             \n\t"
                "bge        avg_filter_3bpp_loop                      \n\t"
                "add        %[rowbytes], %[rowbytes], #16             \n\t"           // ??????????????????, to avoid over read

                "avg_filter_3bpp_lessthan16:                          \n\t"
                "cmp        %[rowbytes], #3                           \n\t"
                "blt        avg_filter_3bpp_lessthan3                 \n\t"
                "sub        %[rowbytes], %[rowbytes], #3              \n\t"

                "avg_filter_3bpp_loop_tail:                           \n\t"
	            //ld1         {v11.S}[0],[%[row]]     // may over read 1 byte,??????????????
                "ld1        {v11.H}[0],[%[row]],#2                    \n\t"
                "ld1        {v11.B}[2],[%[row]]                       \n\t"
                "sub        %[row],%[row],#2                          \n\t"
	            //ld1         {v10.S}[0],[%[prev_row]],#3         //#3bpp //may over read 1 byte, ??????????????????????
                "ld1        {v10.H}[0],[%[prev_row]],#2               \n\t"
                "ld1        {v10.B}[2],[%[prev_row]],#1               \n\t"

                "subs       %[rowbytes], %[rowbytes], #3              \n\t"

                "uhadd      v0.8B,  v3.8B,  v10.8B                    \n\t"
                "add        v3.8B,  v0.8B,  v11.8B                    \n\t"

                "st1        {v3.H}[0],[%[row]],#2                     \n\t"                // store 2 bytes (updated pixel x)
                "st1        {v3.B}[2],[%[row]],#1                     \n\t"                // store 1 byte (updated pixel x)
                "bge        avg_filter_3bpp_loop_tail                 \n\t"
                "avg_filter_3bpp_lessthan3:                           \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "x5","x6", "v0", "v1","v2", "v3","v5","v6","v7","v10","v11","v17","v18","v19","v21","v23"
        );

}
void avg_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row) {

	    //use d0,d1,d2,d3, d4,d5,d6,d7,d16,d17,d18,d19
        asm volatile(
                "movi       v3.8B,  #0                                \n\t"
                "cmp        %[rowbytes], #16                          \n\t"
                "blt        avg_filter_4bpp_lessthan16                \n\t"//use this to debug
                "sub        %[rowbytes], %[rowbytes], #16             \n\t"

                "avg_filter_4bpp_loop:                                \n\t"
               // "ld4r       {v4.2S, v5.2S, v6.2S, v7.2S},    [%[row]] \n\t"
        		"ld4       {v4.S, v5.S, v6.S, v7.S}[0],    [%[row]] \n\t"
                //"ld4r       {v16.2S, v17.2S, v18.2S, v19.2S},[%[prev_row]], #16          \n\t"
        		"ld4       {v16.S, v17.S, v18.S, v19.S}[0],[%[prev_row]], #16          \n\t"
                "subs       %[rowbytes], %[rowbytes], #16             \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"
                "uhadd      v1.8B,  v0.8B,  v17.8B                    \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
                "uhadd      v2.8B,  v1.8B,  v18.8B                    \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
                "uhadd      v3.8B,  v2.8B,  v19.8B                    \n\t"
                "add        v3.8B,   v3.8B,  v7.8B                    \n\t"
                "st4        {v0.S,v1.S,v2.S,v3.S}[0],[%[row]], #16    \n\t"
                "bgt        avg_filter_4bpp_loop                      \n\t"

                "add        %[rowbytes], %[rowbytes], #16             \n\t"
                "avg_filter_4bpp_lessthan16:                          \n\t"
                "cmp        %[rowbytes], #4                           \n\t"
                "blt        avg_filter_4bpp_lessthan4                 \n\t"
                "sub        %[rowbytes], %[rowbytes], #4              \n\t"

                "avg_filter_4bpp_loop_tail:                           \n\t"
                //"ld1r       {v4.2S}, [%[row]]                         \n\t"
        		"ld1       {v4.S}[0], [%[row]]                         \n\t"
                //"ld1r       {v16.2S},[%[prev_row]], #4                \n\t"
        		"ld1       {v16.S}[0],[%[prev_row]], #4                \n\t"
                "subs       %[rowbytes], %[rowbytes], #4              \n\t"
                "uhadd      v0.8B,  v3.8B,  v16.8B                    \n\t"
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "st1        {v3.S}[0], [%[row]], #4                   \n\t"
                "bge        avg_filter_4bpp_loop_tail                 \n\t"

                "avg_filter_4bpp_lessthan4:                           \n\t"

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory", "v0", "v1", "v2", "v3","v4", "v5","v6", "v7","v16", "v17","v18", "v19"
        );
}
void avg_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row) {
	//use x5, d0,d1,d2,q2-->d0,d1,d2,q3
        asm volatile(

                "mov        x5,#6                                     \n\t"                      //#6bpp
			 //     "ld1     {v0.8B},[%[row]]                               \n\t"                    // load 8 bytes (pixel x + 2 extra
                "ld1        {v0.S}[0],[%[row]],#4                     \n\t"
                "ld1        {v0.H}[2],[%[row]]                        \n\t"
                "sub        %[row],%[row],#4                          \n\t"
			                                                //  bytes) from curr row into d0
			      //"ld1     {v1.8B},[%[prev_row]],#6                       \n\t"                 // #6bpp,load 8 bytes (pixel b + 2 extra
                "ld1        {v1.S}[0],[%[prev_row]],#4                \n\t"
                "ld1        {v1.H}[2],[%[prev_row]],#2                \n\t"
			                                                //  bytes) from prev row into d1
			                                                // increment prev row pointer
                "usra       v0.8B,v1.8B,#1                            \n\t"                  // shift right pixel b by 1 and add
			                                                //  to pixel x
                "st1        {v0.S}[0],[%[row]],#4                     \n\t"                // store 4 bytes (updated pixel x)
			                                                // increment curr row pointer
			                                                // updated pixelxis now pixel a
                "st1        {v0.H}[2],[%[row]],#2                     \n\t"                 // store 2 bytes (updated pixel x)
			                                                // increment curr row pointer
			                                                // updated pixelxis now pixel a
                "cmp        x5,%[rowbytes]                            \n\t"
                "beq        avg_filter_6bpp_DONE                      \n\t"

                "avg_filter_6bpp_loop:                                \n\t"
                "add        x5,x5,#6                                  \n\t"             //#6bpp loop counter += bpp

			      //"ld1     {v2.8B},[%[row]]                               \n\t"                    // load 8 bytes (pixel x + 2 extra
			                                                //  bytes) from curr row into d2
                "ld1        {v2.S}[0],[%[row]], #4                    \n\t"
                "ld1        {v2.H}[2],[%[row]]                        \n\t"
                "sub        %[row],%[row],#4                          \n\t"
			      //"ld1     {v1.8B},[%[prev_row]],#6                       \n\t"               // #6bpp,load 8 bytes (pixel b + 2 extra
                "ld1        {v1.S}[0],[%[prev_row]],#4                \n\t"
                "ld1        {v1.H}[2],[%[prev_row]],#2                \n\t"
			                                                //  bytes) from prev row into d1
                "uaddl      v3.8H,v0.8B,v1.8B                         \n\t"                    // q2 = (pixel a + pixel b)
                "shrn       v1.8B,v3.8H,#1                            \n\t"                 // d1 = (a + b)/2//?????7/10
                "add        v0.8B,v2.8B,v1.8B                         \n\t"                    // d0 = x + ((a + b)/2)
                "st1        {v0.S}[0],[%[row]],#4                     \n\t"               // store 4 bytes (updated pixel x)
			                                                // increment curr row pointer
                "st1        {v0.H}[2],[%[row]],#2                     \n\t"               // store 2 bytes (updated pixel x)
			                                                // increment curr row pointer
                "cmp        x5,%[rowbytes]                            \n\t"
                "bne        avg_filter_6bpp_loop                      \n\t"
                "avg_filter_6bpp_DONE:                                \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory", "x5", "v0", "v1","v2", "v3"
        );
}
void avg_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //use x5, d0,d1,d2,q2-->d0,d1,d2,q3
        asm volatile(
                "mov        x5, #8                                    \n\t"
                "cmp        x5, %[rowbytes]                           \n\t"

                "ld1        {v0.8B},[%[row]]                         \n\t"                    // load 8 bytes (pixel x) from curr
			                                                //  row into d0
                "ld1        {v1.8B},[%[prev_row]],#8                  \n\t"                  // load 8 bytes (pixel b) from prev
			                                                //  row into d1
			                                                // increment prev row pointer
                "usra       v0.8B,v1.8B,#1                            \n\t"                  // shift right pixel b by 1 and add
			                                                //  to pixel x
                "st1        {v0.8B},[%[row]],#8                      \n\t"                     // store 8 bytes (updated pixel x)
			                                                // increment curr row pointer
			                                                // updated pixelxis now pixel a
                "beq        avg_filter_8bpp_loop_DONE                 \n\t"
                "avg_filter_8bpp_loop:                                \n\t"
                "add        x5,x5,#8                                  \n\t"                  // loop counter += bpp,#8bpp
                "cmp        x5,%[rowbytes]                            \n\t"


                "ld1        {v2.8B},[%[row]]                         \n\t"                    // load 8 bytes (pixel x) from curr
			                                                //  row into d2
                "ld1        {v1.8B},[%[prev_row]],#8                  \n\t"                   // load 8 bytes (pixel b) from prev
			                                                //  row into d1
                "uaddl      v3.8H,v0.8B,v1.8B                         \n\t"                   // q2 = (pixel a + pixel b)
                "shrn       v1.8B,v3.8H,#1                            \n\t"                  // d1 = (a + b)/2//????7/10
                "add        v0.8B,v2.8B,v1.8B                         \n\t"                    // d0 = x + ((a + b)/2)
                "st1        {v0.8B},[%[row]],#8                      \n\t"                  // store 8 bytes (updated pixel x)
			                                                // increment curr row pointer
                "bne        avg_filter_8bpp_loop                      \n\t"
                "avg_filter_8bpp_loop_DONE:                           \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory", "x5", "v0", "v1","v2", "v3"
        );
}

void paeth_filter_1bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //use d0,d1,d2,d3,d4,d5,d6,d7,d16,d17,d18,d19,d20,not forget paeth macro q12,q13,q14,q15 
        asm volatile(

                "cmp        %[rowbytes], #4                           \n\t"
                "movi       v3.8B,  #0                                \n\t"
                "movi       v20.8B, #0                                \n\t"
                "blt        paeth_filter_1bpp_lessthan4               \n\t" //use this to debug.
                "sub        %[rowbytes], %[rowbytes], #4              \n\t"

                "paeth_filter_1bpp_loop:                              \n\t"

                "ld4        {v4.B,v5.B,v6.B,v7.B}[0],    [%[row]]     \n\t"
                "ld4        {v16.B,v17.B,v18.B,v19.B}[0],[%[prev_row]],#4                                                  \n\t"
                "subs       %[rowbytes], %[rowbytes], #4              \n\t"

	             // "paeth           v0.8B,  v3.8B,  v16.8B, v20.8B              \n\t" //add mark
                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"        // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"         // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"         // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"

	            // "bl paeth           v1.8B,  v0.8B,  v17.8B, v16.8B           \n\t"
                "uaddl      v12.8H, v0.8B, v17.8B                     \n\t"        //a + b
                "uaddl      v15.8H, v16.8B, v16.8B                    \n\t"          // 2*c
                "uabdl      v13.8H, v17.8B, v16.8B                    \n\t"          // pa
                "uabdl      v14.8H, v0.8B, v16.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v1.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v17.8B, v16.8B                    \n\t"
                "bsl        v1.8B, v0.8B, v14.8B                      \n\t" //replace end
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"

	           //"bl paeth           v2.8B,  v1.8B,  v18.8B, v17.8B           \n\t"
                "uaddl      v12.8H, v1.8B, v18.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v17.8B, v17.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v18.8B, v17.8B                    \n\t"         // pa
                "uabdl      v14.8H, v1.8B, v17.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"        // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v2.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v18.8B, v17.8B                    \n\t"
                "bsl        v2.8B, v1.8B, v14.8B                      \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"

	             //"bl paeth           v3.8B,  v2.8B,  v19.8B, v18.8B           \n\t"
                "uaddl      v12.8H, v2.8B,v19.8B                      \n\t"     //a + b
                "uaddl      v15.8H, v18.8B, v18.8B                    \n\t"          // 2*c
                "uabdl      v13.8H,v19.8B, v18.8B                     \n\t"         // pa
                "uabdl      v14.8H, v2.8B, v18.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v3.8B, v12.8H                             \n\t"
                "bsl        v14.8B,v19.8B, v18.8B                     \n\t"
                "bsl        v3.8B, v2.8B, v14.8B                      \n\t"

                "mov        v20.8B, v19.8B                            \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
                "st4        {v0.B,v1.B,v2.B,v3.B}[0],[%[row]],#4      \n\t"
                "bge        paeth_filter_1bpp_loop                    \n\t"

                "add        %[rowbytes], %[rowbytes], #4              \n\t"
                "paeth_filter_1bpp_lessthan4:                         \n\t"
                "cmp        %[rowbytes], #1                           \n\t"
                "blt        paeth_filter_1bpp_lessthan1               \n\t"
                "sub        %[rowbytes], %[rowbytes], #1              \n\t"

                "paeth_filter_1bpp_loop_tail:                         \n\t"
                "ld1        {v4.B}[0], [%[row]]                       \n\t"
                "ld1        {v16.B}[0],[%[prev_row]],#1               \n\t"
                "subs       %[rowbytes], %[rowbytes], #1              \n\t"

	            //"paeth           v0.8B,  v3.8B,  v16.8B, v20.8B              \n\t"
                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"         //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"        // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"       // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"        // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"       // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"       // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"      // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "mov        v20.8B, v16.8B                            \n\t"
                "st1        {v3.B}[0],[%[row]],#1                     \n\t"
                "bge        paeth_filter_1bpp_loop_tail               \n\t"

                "paeth_filter_1bpp_lessthan1:                         \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory", "v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7", "v12", "v13", "v14", "v15", "v16", "v17","v18", "v19", "v20"
        );
}

void paeth_filter_2bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){

	            //use d0,d1,d2,d3,d4,d5,d6,d7,d16,d17,d18,d19,d20
        asm volatile(

                "cmp        %[rowbytes], #8                           \n\t"
                "movi       v3.8B,  #0                                \n\t"
                "movi       v20.8B, #0                                \n\t"
                "blt        paeth_filter_2bpp_lessthan8               \n\t"//use this to debug.
                "sub        %[rowbytes], %[rowbytes], #8              \n\t"

                "paeth_filter_2bpp_loop:                              \n\t"

                "ld4        {v4.H,v5.H,v6.H,v7.H}[0],    [%[row]]     \n\t"
                "ld4        {v16.H,v17.H,v18.H,v19.H}[0],[%[prev_row]],#8                                                  \n\t"
                "subs       %[rowbytes], %[rowbytes], #8              \n\t"

		   //  "paeth           v0.8B,  v3.8B,  v16.8B, v20.8B            \n\t"
                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"        // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"         // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"         // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"//replace end
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"

		 //    "paeth           v1.8B,  v0.8B,  v17.8B, v16.8B            \n\t"
                "uaddl      v12.8H, v0.8B, v17.8B                     \n\t"        //a + b
                "uaddl      v15.8H, v16.8B, v16.8B                    \n\t"          // 2*c
                "uabdl      v13.8H, v17.8B, v16.8B                    \n\t"          // pa
                "uabdl      v14.8H, v0.8B, v16.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v1.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v17.8B, v16.8B                    \n\t"
                "bsl        v1.8B, v0.8B, v14.8B                      \n\t" //replace end
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"

	            //   "paeth           v2.8B,  v1.8B,  v18.8B, v17.8B            \n\t"

                "uaddl      v12.8H, v1.8B, v18.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v17.8B, v17.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v18.8B, v17.8B                    \n\t"         // pa
                "uabdl      v14.8H, v1.8B, v17.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"        // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v2.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v18.8B, v17.8B                    \n\t"
                "bsl        v2.8B, v1.8B, v14.8B                      \n\t"//replace end
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
	            // "paeth           v3.8B,  v2.8B,  v19.8B, v18.8B            \n\t"
                "uaddl      v12.8H, v2.8B,v19.8B                      \n\t"     //a + b
                "uaddl      v15.8H, v18.8B, v18.8B                    \n\t"          // 2*c
                "uabdl      v13.8H,v19.8B, v18.8B                     \n\t"         // pa
                "uabdl      v14.8H, v2.8B, v18.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v3.8B, v12.8H                             \n\t"
                "bsl        v14.8B,v19.8B, v18.8B                     \n\t"
                "bsl        v3.8B, v2.8B, v14.8B                      \n\t"

                "mov        v20.8B, v19.8B                            \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
                "st4        {v0.H,v1.H,v2.H,v3.H}[0],[%[row]],#8      \n\t"
                "bge        paeth_filter_2bpp_loop                    \n\t"

                "add        %[rowbytes], %[rowbytes], #8              \n\t"
                "paeth_filter_2bpp_lessthan8:                         \n\t"
                "cmp        %[rowbytes], #2                           \n\t"
                "blt        paeth_filter_2bpp_lessthan2               \n\t"
                "sub        %[rowbytes], %[rowbytes], #2              \n\t"

                "paeth_filter_2bpp_loop_tail:                         \n\t"
                "ld1        {v4.H}[0], [%[row]]                       \n\t"
                "ld1        {v16.H}[0],[%[prev_row]],#2               \n\t"
                "subs       %[rowbytes], %[rowbytes], #2              \n\t"
	            //  "paeth        v0.8B,  v3.8B,  v16.8B, v20.8B               \n\t"

                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"         //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"        // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"       // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"        // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"       // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"       // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"      // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"      //replace end
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "mov        v20.8B, v16.8B                            \n\t"
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"
                "bge        paeth_filter_2bpp_loop_tail               \n\t"

                "paeth_filter_2bpp_lessthan2:                         \n\t"

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7", "v12", "v13", "v14", "v15", "v16", "v17","v18", "v19", "v20"
        );
}
void paeth_filter_3bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	            //use x5,d0,d1,d2,d3,d4,d5,d6,d7,d17,d18,d19,q10(d20,d11),q11(d22,d23),d21,d23
        asm volatile(
                "movi       v3.8B,  #0                                \n\t"
                "movi       v4.8B,  #0                                \n\t"
                "mov        x5,  #12                                  \n\t"
                "cmp        %[rowbytes], #16                          \n\t"
                "blt        paeth_filter_3bbp_lessthan16              \n\t" //use this to debug.
                "sub        %[rowbytes], %[rowbytes], #16             \n\t"
                "paeth_filter_3bpp_loop:                              \n\t"
                "ld1        {v11.16B},    [%[row]]                    \n\t"
                "ld1        {v10.16B},    [%[prev_row]], x5           \n\t"
                "mov        D23,v11.D[1]                              \n\t"
                "mov        D21,v10.D[1]                              \n\t"
	            // "paeth           v0.8B,  v3.8B,  v10.8B, v4.8B             \n\t"
                "uaddl      v12.8H,  v3.8B,v10.8B                     \n\t"       //a + b
                "uaddl      v15.8H, v4.8B, v4.8B                      \n\t"        // 2*c
                "uabdl      v13.8H,v10.8B, v4.8B                      \n\t"      // pa
                "uabdl      v14.8H,  v3.8B, v4.8B                     \n\t"       // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"       // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"       // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"       // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"     // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"         // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B,v10.8B, v4.8B                      \n\t"
                "bsl        v0.8B,  v3.8B, v14.8B                     \n\t"     //replace end
                "ext        v5.8B,  v11.8B, v23.8B, #3                \n\t"
                "add        v0.8B,  v0.8B,  v11.8B                    \n\t"
                "ext        v17.8B, v10.8B, v21.8B, #3                \n\t"
	            // "paeth           v1.8B,  v0.8B,  v17.8B, v10.8B            \n\t"
                "uaddl      v12.8H, v0.8B, v17.8B                     \n\t"    //a + b
                "uaddl      v15.8H, v10.8B, v10.8B                    \n\t"    // 2*c
                "uabdl      v13.8H, v17.8B, v10.8B                    \n\t"    // pa
                "uabdl      v14.8H, v0.8B, v10.8B                     \n\t"    // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"     // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"     // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"     // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"    // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"        // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v1.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v17.8B, v10.8B                    \n\t"
                "bsl        v1.8B, v0.8B, v14.8B                      \n\t" //replace end
	            // "st1             {v0.S}[0],  [%[row]], #3                  \n\t"
                "st1        {v0.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v0.B}[2],  [%[row]], #1                  \n\t"
                "ext        v6.8B,  v11.8B, v23.8B, #6                \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
                "ext        v18.8B, v10.8B, v21.8B, #6                \n\t"
	            //  "paeth           v2.8B,  v1.8B,  v18.8B, v17.8B            \n\t"
                "uaddl      v12.8H, v1.8B, v18.8B                     \n\t"   //a + b
                "uaddl      v15.8H, v17.8B, v17.8B                    \n\t"      // 2*c
                "uabdl      v13.8H, v18.8B, v17.8B                    \n\t"       // pa
                "uabdl      v14.8H, v1.8B, v17.8B                     \n\t"      // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"       // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"        // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"        // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"       // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v2.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v18.8B, v17.8B                    \n\t"
                "bsl        v2.8B, v1.8B, v14.8B                      \n\t"  //replace end
                "ext        v7.8B,  v23.8B, v23.8B, #1                \n\t"
	            //"st1             {v1.S}[0],  [%[row]], #3                  \n\t"
                "st1        {v1.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v1.B}[2],  [%[row]], #1                  \n\t"
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
                "ext        v19.8B, v21.8B, v21.8B, #1                \n\t"
	            //"paeth           v3.8B,  v2.8B,  v19.8B, v18.8B            \n\t"
                "uaddl      v12.8H, v2.8B, v19.8B                     \n\t"         //a + b
                "uaddl      v15.8H, v18.8B, v18.8B                    \n\t"        // 2*c
                "uabdl      v13.8H, v19.8B, v18.8B                    \n\t"      // pa
                "uabdl      v14.8H, v2.8B, v18.8B                     \n\t"     // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"      // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"      // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"      // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"    // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"         // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v3.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v19.8B, v18.8B                    \n\t"
                "bsl        v3.8B, v2.8B, v14.8B                      \n\t"    //replace end
	            //"st1             {v2.S}[0],  [%[row]], #3                  \n\t"
                "st1        {v2.H}[0],  [%[row]], #2                  \n\t"
                "st1        {v2.B}[2],  [%[row]], #1                  \n\t"
                "mov        v4.8B,  v19.8B                            \n\t"
                "add        v3.8B,  v3.8B,  v7.8B                     \n\t"
	            //"vst1.32         {d3[0]},  [r2], r12                       \n\t"
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"               // store 2 bytes (updated pixel x)
                "st1        {v3.B}[2],[%[row]],#1                     \n\t"                // store 1 byte (updated pixel x)
                "subs       %[rowbytes], %[rowbytes], #12             \n\t"
                "bge        paeth_filter_3bpp_loop                    \n\t"
                "add        %[rowbytes], %[rowbytes], #16             \n\t"
                "paeth_filter_3bbp_lessthan16:                        \n\t"
                "cmp        %[rowbytes], #3                           \n\t"
                "blt        paeth_filter_3bpp_lessthan3               \n\t"
                "sub        %[rowbytes], %[rowbytes], #3              \n\t"

                "paeth_filter_3bpp_loop_tail:                         \n\t"
                "ld1        {v11.S}[0],    [%[row]]                   \n\t"     // read 1 more byte???????????NE
	            // "ld1            {v10.S}[0],    [%[prev_row]], #3           \n\t"
                "ld1        {v10.H}[0],    [%[prev_row]], #2          \n\t"
                "ld1        {v10.B}[2],    [%[prev_row]], #1          \n\t"
                "subs       %[rowbytes], %[rowbytes], #3              \n\t"
	            //"paeth           v0.8B,  v3.8B,  v10.8B, v4.8B             \n\t"
                "uaddl      v12.8H, v3.8B, v10.8B                     \n\t"    //a + b
                "uaddl      v15.8H, v4.8B, v4.8B                      \n\t"   // 2*c
                "uabdl      v13.8H, v10.8B, v4.8B                     \n\t"    // pa
                "uabdl      v14.8H, v3.8B, v4.8B                      \n\t"   // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"   // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"   // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"    // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"   // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"   // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v10.8B, v4.8B                     \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"   //replace end
                "add        v3.8B,  v0.8B,  v11.8B                    \n\t"
                "mov        v4.8B,  v10.8B                            \n\t"
                "st1        {v3.H}[0],[%[row]],#2                     \n\t"             // store 2 bytes (updated pixel x)
                "st1        {v3.B}[2],[%[row]],#1                     \n\t"              // store 1 byte (updated pixel x)
                "bge        paeth_filter_3bpp_loop_tail               \n\t"
                "paeth_filter_3bpp_lessthan3:                         \n\t"

                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "x5","v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7",  "v12", "v13", "v14", "v15", "v17","v18", "v19", "v21","v23","v10","v11"
        );
}
void paeth_filter_4bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //use d0,d1,d2,d3,d4,d5,d6,d7,d16,d17,d18,d19,d20
        asm volatile(
                "cmp        %[rowbytes], #16                          \n\t"
                "movi       v3.8B,  #0                                \n\t"
                "movi       v20.8B, #0                                \n\t"
                "blt        paeth_filter_4bpp_lessthan16              \n\t"//use this to debug.
                "sub        %[rowbytes], %[rowbytes], #16             \n\t"

                "paeth_filter_4bpp_loop:                              \n\t"

                "ld4r       {v4.2S,v5.2S,v6.2S,v7.2S},    [%[row]]    \n\t"
                "ld4r       {v16.2S,v17.2S,v18.2S,v19.2S},[%[prev_row]],#16                                                  \n\t"
                "subs       %[rowbytes], %[rowbytes], #16             \n\t"
			    // "paeth           v0.8B,  v3.8B,  v16.8B, v20.8B          \n\t"//??much bits???
                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"        // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"         // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"         // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t" //replace end
                "add        v0.8B,  v0.8B,  v4.8B                     \n\t"
			    // "paeth           v1.8B,  v0.8B,  v17.8B, v16.8B          \n\t"
                "uaddl      v12.8H, v0.8B, v17.8B                     \n\t"        //a + b
                "uaddl      v15.8H, v16.8B, v16.8B                    \n\t"          // 2*c
                "uabdl      v13.8H, v17.8B, v16.8B                    \n\t"          // pa
                "uabdl      v14.8H, v0.8B, v16.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v1.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v17.8B, v16.8B                    \n\t"
                "bsl        v1.8B, v0.8B, v14.8B                      \n\t"
                "add        v1.8B,  v1.8B,  v5.8B                     \n\t"
			    // "paeth           v2.8B,  v1.8B,  v18.8B, v17.8B          \n\t"
                "uaddl      v12.8H, v1.8B, v18.8B                     \n\t"      //a + b
                "uaddl      v15.8H, v17.8B, v17.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v18.8B, v17.8B                    \n\t"         // pa
                "uabdl      v14.8H, v1.8B, v17.8B                     \n\t"        // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"         // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"         // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"        // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"       // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v2.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v18.8B, v17.8B                    \n\t"
                "bsl        v2.8B, v1.8B, v14.8B                      \n\t"//replace end
                "add        v2.8B,  v2.8B,  v6.8B                     \n\t"
			    // "paeth           v3.8B,  v2.8B,  v19.8B, v18.8B          \n\t"
                "uaddl      v12.8H, v2.8B,v19.8B                      \n\t"     //a + b
                "uaddl      v15.8H, v18.8B, v18.8B                    \n\t"          // 2*c
                "uabdl      v13.8H,v19.8B, v18.8B                     \n\t"         // pa
                "uabdl      v14.8H, v2.8B, v18.8B                     \n\t"         // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"          // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"          // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"          // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"         // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v3.8B, v12.8H                             \n\t"
                "bsl        v14.8B,v19.8B, v18.8B                     \n\t"
                "bsl        v3.8B, v2.8B, v14.8B                      \n\t" //replace end
                "mov        v20.8B, v19.8B                            \n\t"
                "add        v3.8B,   v3.8B,  v7.8B                    \n\t"
                "st4        {v0.S, v1.S, v2.S, v3.S}[0],[%[row]], #16 \n\t"
                "bge        paeth_filter_4bpp_loop                    \n\t"

                "add        %[rowbytes], %[rowbytes], #16             \n\t"
                "paeth_filter_4bpp_lessthan16:                        \n\t"
                "cmp        %[rowbytes], #4                           \n\t"
                "blt        paeth_filter_4bpp_lessthan4               \n\t"
                "sub        %[rowbytes], %[rowbytes], #4              \n\t"

                "paeth_filter_4bpp_loop_tail:                         \n\t"
                "ld1        {v4.S}[0], [%[row]]                       \n\t"
                "ld1        {v16.S}[0],[%[prev_row]], #4              \n\t"
                "subs       %[rowbytes], %[rowbytes], #4              \n\t"
	            //	     "paeth           v0.8B,  v3.8B,  v16.8B, v20.8B          \n\t"//??
                "uaddl      v12.8H, v3.8B, v16.8B                     \n\t"         //a + b
                "uaddl      v15.8H, v20.8B, v20.8B                    \n\t"         // 2*c
                "uabdl      v13.8H, v16.8B, v20.8B                    \n\t"        // pa
                "uabdl      v14.8H, v3.8B, v20.8B                     \n\t"       // pb
                "uabd       v15.8H, v12.8H, v15.8H                    \n\t"        // pc
                "cmhs       v12.8H, v14.8H , v13.8H                   \n\t"       // pa <= pb
                "cmhs       v13.8H, v15.8H , v13.8H                   \n\t"       // pa <= pc
                "cmhs       v14.8H, v15.8H , v14.8H                   \n\t"      // pb <= pc
                "and        v12.16B, v12.16B, v13.16B                 \n\t"          // pa <= pb && pa <= pc
                "xtn        v14.8B, v14.8H                            \n\t"
                "xtn        v0.8B, v12.8H                             \n\t"
                "bsl        v14.8B, v16.8B, v20.8B                    \n\t"
                "bsl        v0.8B, v3.8B, v14.8B                      \n\t"      //replace end
                "add        v3.8B,  v0.8B,  v4.8B                     \n\t"
                "mov        v20.8B, v16.8B                            \n\t"//??
                "st1        {v3.S}[0], [%[row]], #4                   \n\t"
                "bge        paeth_filter_4bpp_loop_tail               \n\t"

                "paeth_filter_4bpp_lessthan4:                         \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7", "v12", "v13", "v14", "v15", "v16", "v17","v18", "v19", "v20"
        );
}
void paeth_filter_6bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	    //use "x5","d0", "d1->v8","d2->v1","d3->v9","q2", "q3" ,"q4", "q5","q6", "q7",,no paeth macro
        asm volatile(
                "mov        x5, #6                                    \n\t"
                "cmp        x5, %[rowbytes]                           \n\t"

                "ld1        {v0.8B},[%[row]]                          \n\t"                 // load 8 bytes (pixel x + 2 extra
			                                                //  bytes) from curr row into d0
			     //"ld1     {v8.8B},[%[prev_row]],#6   %[prev_row]          \n\t"                // load 8 bytes (pixel b + 2 extra
                "ld1        {v8.S}[0],[%[prev_row]],#4                \n\t"
                "ld1        {v8.H}[2],[%[prev_row]],#2                \n\t"
			                                                //  bytes) from prev row into d1
			                                                // increment prev row pointer
                "add        v1.8B,v0.8B,v8.8B                         \n\t"                     // d2 = x + b = updated pixel x
                "st1        {v1.S}[0],[%[row]],#4                     \n\t"                 // store 4 bytes (updated pixel x)
			                                                // increment curr row pointer
                "st1        {v1.H}[2],[%[row]],#2                     \n\t"                 // store 2 bytes (updated pixel x)
			                                                // increment curr row pointer
                "beq        paeth_filter_DONE                         \n\t"

                "paeth_filter_6bpp_loop:                              \n\t"
                "add        x5,x5,#6                                  \n\t"                // loop counter += bpp
                "cmp        x5,%[rowbytes]                            \n\t"


			     //;; d1[0] = c (b in the previous loop iteration)
			     //;; d2[0] = a (x in the previous loop iteration)
			   //  "ld1     {v9.8B},[%[prev_row]],#6                        \n\t"              // load 8 bytes (pixel b + 2 extra
                "ld1        {v9.S}[0],[%[prev_row]],#4                \n\t"
                "ld1        {v9.H}[2],[%[prev_row]],#2                \n\t"
			                                                //  bytes) from prev row into d3
                "ld1        {v0.8B},[%[row]]                          \n\t"                 // load 8 bytes (pixel x + 2 extra
			                                                //  bytes) from curr row into d0
                "ushll      v4.8H,v8.8B,#1                            \n\t"                  // q4 = c<<1 = 2c///????ask
                "uabdl      v3.8H,v1.8B,v8.8B                         \n\t"                   // q3 = pb = abs(a - c)
                "uabdl      v2.8H,v9.8B,v8.8B                         \n\t"                     // q2 = pa = abs(b - c)
                "uaddl      v5.8H,v1.8B,v9.8B                         \n\t"                    // q5 = a + b
                "uabd       v4.8H,v5.8H,v4.8H                         \n\t"                    // q4 = pc = abs(a + b - 2c)

                "cmge       v5.8H,v3.8H,v2.8H                         \n\t"                  // q5 = (pa <= pb)
                "cmge       v6.8H,v4.8H,v2.8H                         \n\t"                  // q6 = (pa <= pc)
                "and        v5.16B ,v5.16B ,v6.16B                    \n\t"                     // q5 = ((pa <= pb) && (pa <= pc))//??7/10
                "cmge       v7.8H,v4.8H, v3.8H                        \n\t"                  // q7 = (pb <= pc)
                "shrn       v5.8B,v5.8H,#8                            \n\t"                // d10 = ((pa <= pb) && (pa <= pc))//??shrn2??7/10
                "shrn       v7.8B,v7.8H,#8                            \n\t"                // d14 = (pb <= pc)

                "and        v1.8B,v1.8B,v5.8B                         \n\t"                   // d2 = a where 1, 0 where 0
                "bsl        v7.8B,v9.8B,v8.8B                         \n\t"                   // d14 = b where 1, c where 0
                "mvn        v5.8B,v5.8B                               \n\t"               // invert d10
                "and        v7.8B,v7.8B,v5.8B                         \n\t"                 // d14 = b/c where 1, 0 where 0
                "add        v1.8B,v1.8B,v7.8B                         \n\t"                   // d2 = p = a/b/c where appropriate
                "add        v1.8B,v1.8B,v0.8B                         \n\t"                  // d2 = x + p (updated pixel x)
                "mov        v8.8B,v9.8B                               \n\t"                 // d1 = b (c for next iteration)//16B??/710
                "st1        {v1.S}[0],[%[row]],#4                     \n\t"               // store 4 bytes (updated pixel x)
			                                                // increment curr row pointer
                "st1        {v1.H}[2],[%[row]],#2                     \n\t"                 // store 2 bytes (updated pixel x)
			                                                // increment curr row pointer
                "bne        paeth_filter_6bpp_loop                    \n\t"
                "paeth_filter_DONE:                                   \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r" (prev_row)
                :
                :"memory", "x5","v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7","v8","v9"
        );

}


void paeth_filter_8bpp_neon64(png_size_t rowbytes, png_bytep row, png_const_bytep prev_row){
	     //use "x5","d0", "d1","q2", "q3" ,"q4", "q5(+d10)","q6", "q7(+d14)", "d8","d9",no paeth macro
        asm volatile(
                "mov        x5, #8                                    \n\t"
                "cmp        x5, %[rowbytes]                           \n\t"

                "ld1        {v0.8B},[%[row]]                          \n\t"                      // load 8 bytes (pixel x) from curr
			                                                //  row into d0
                "ld1        {v8.8B},[%[prev_row]],#8                  \n\t"                    // load 8 bytes (pixel b) from prev
			                                                //  row into d1
			                                                // increment prev row pointer
                "add        v1.8B,v0.8B,v8.8B                         \n\t"                       // d2 = x + b = updated pixel x
                "st1        {v1.8B},[%[row]],#8                       \n\t"                      // store 8 bytes (updated pixel x)
			                                                // increment curr row pointer
                "beq        paeth_filter_DONE                         \n\t"

                "paeth_filter_8bpp_loop:                              \n\t"
                "add        x5,x5,#8                                  \n\t"                     // loop counter += bpp
                "cmp        x5,%[rowbytes]                            \n\t"


			    //;; d1[0] = c (b in the previous loop iteration)
			    //;; d2[0] = a (x in the previous loop iteration)
                "ld1        {v9.8B},[%[prev_row]],#8                  \n\t"                     // load 8 bytes (pixel b) from prev
			                                                //  row into d3
                "ld1        {v0.8B},[%[row]]                          \n\t"                     // load 8 bytes (pixel x) from curr
			                                                //  row into d0
                "ushll      v4.8H,v8.8B,#1                            \n\t"                        // q4 = c<<1 = 2c
                "uabdl      v3.8H,v1.8B,v8.8B                         \n\t"                        // q3 = pb = abs(a - c)
                "uabdl      v2.8H,v9.8B,v8.8B                         \n\t"                        // q2 = pa = abs(b - c)
                "uaddl      v5.8H,v1.8B,v9.8B                         \n\t"                        // q5 = a + b
                "uabd       v4.8H,v5.8H,v4.8H                         \n\t"                        // q4 = pc = abs(a + b - 2c)

                "cmge       v5.8H,v3.8H,v2.8H                         \n\t"                        // q5 = (pa <= pb)
                "cmge       v6.8H,v4.8H,v2.8H                         \n\t"                       // q6 = (pa <= pc)
                "and        v5.16B,v5.16B,v6.16B                      \n\t"                         // q5 = ((pa <= pb) && (pa <= pc))//q5 before is 16s/8H
                "cmge       v7.8H,v4.8H,v3.8H                         \n\t"                      // q7 = (pb <= pc)
                "shrn       v5.8B,v5.8H,#8                            \n\t"                      // d10 = ((pa <= pb) && (pa <= pc))//??shrn2??7/10
                "shrn       v7.8B,v7.8H,#8                            \n\t"                      // d14 = (pb <= pc)

                "and        v1.8B,v1.8B,v5.8B                         \n\t"                      // d2 = a where 1, 0 where 0//??7/10
                "bsl        v7.8B,v9.8B,v8.8B                         \n\t"                      // d14 = b where 1, c where 0
                "mvn        v5.8B,v5.8B                               \n\t"                  // invert d10
                "and        v7.8B,v7.8B,v5.8B                         \n\t"                     // d14 = b/c where 1, 0 where 0
                "add        v1.8B,v1.8B,v7.8B                         \n\t"                     // d2 = p = a/b/c where appropriate
                "add        v1.8B,v1.8B,v0.8B                         \n\t"                                      // d2 = x + p (updated pixel x)
                "mov        v8.8B,v9.8B                               \n\t"                      // d1 = b (c for next iteration)
                "st1        {v1.8B},[%[row]],#8                       \n\t"                     // store 8 bytes (updated pixel x)
			                                                // increment curr row pointer
                "bne        paeth_filter_8bpp_loop                    \n\t"
                :[rowbytes] "+r" (rowbytes), [row] "+r" (row), [prev_row] "+r"(prev_row)
                :
                :"memory", "x5","v0", "v1","v2", "v3" ,"v4", "v5","v6", "v7","v8","v9"
        );
}
