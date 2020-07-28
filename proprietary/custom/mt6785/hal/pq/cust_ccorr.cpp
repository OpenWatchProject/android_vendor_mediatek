#include <string.h>
extern "C" {

/* please set 2's complement, 16-bit */
unsigned int BT709_c00 = 0x2236;
unsigned int BT709_c01 = 0xFE0D;
unsigned int BT709_c02 = 0xFFBD;
unsigned int BT709_c10 = 0xFF03;
unsigned int BT709_c11 = 0x2140;
unsigned int BT709_c12 = 0xFFBD;
unsigned int BT709_c20 = 0xFF03;
unsigned int BT709_c21 = 0xFF0D;
unsigned int BT709_c22 = 0x22F0;
/* set gamut_matrix_en = 1 */
unsigned int gamut_matrix_en = 1;
}
