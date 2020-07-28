// Converts a hexadecimal string to integer
//   0    - Conversion is successful
//   1    - String is empty
//   2    - String has more than 8 bytes
//   4    - Conversion is in process but abnormally terminated by 
//          illegal hexadecimal character


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int xtoi(const char* xs, unsigned int* result)
{
    size_t szlen = strlen(xs);
    int i, xv, fact;

    if(szlen >= 2) {
        /* filter out 0x prefix */
        if(xs[0] == '0' && (xs[1] == 'x' || xs[1] == 'X')) {
            xs += 2;
            szlen -= 2;
        }
    }

    if (szlen > 0)
    {
    
        //Converting more than 32bit hexadecimal value?
        if (szlen > 8) 
            return 2; 

        // Begin conversion here
        *result = 0;
        fact = 1;

        for(i=szlen-1; i>=0 ;i--)
        {
            if (isxdigit(*(xs+i)))
            {
                if (*(xs+i)>=97)
                {
                    xv = ( *(xs+i) - 97) + 10;
                }
                else if ( *(xs+i) >= 65)
                {
                    xv = (*(xs+i) - 65) + 10;
                }
                else
                {
                    xv = *(xs+i) - 48;
                }
                *result += (xv * fact);
                fact *= 16;
            }
            else
            {
                return 4;
            }
        }
    }
    return 1;
}


int xtoAddrptr(const char* xs, unsigned char* ptr)
{
    size_t szlen = strlen(xs);
    unsigned int i, xv, res;

    if (szlen != 12) 
	return 0;

    for (i=0 ;i<szlen; i+=2)
    {
	res = 0;
	if (isxdigit(*(xs+i)) && isxdigit(*(xs+i+1)))
	{
	    if (*(xs+i)>=97)
            {
                xv = ( *(xs+i) - 97) + 10;
            }
            else if ( *(xs+i) >= 65)
            {
                xv = (*(xs+i) - 65) + 10;
            }
            else
            {
                xv = *(xs+i) - 48;
            }
	    res += xv << 4;
			
	    if (*(xs+i+1)>=97)
            {
                xv = ( *(xs+i+1) - 97) + 10;
            }
            else if ( *(xs+i+1) >= 65)
            {
                xv = (*(xs+i+1) - 65) + 10;
            }
            else
            {
                xv = *(xs+i+1) - 48;
            }
	    res += xv;
	    *(ptr+(i>>1)) = res;			
        }
    }
    return 1;
}



