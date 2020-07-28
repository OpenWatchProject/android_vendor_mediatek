/* Reference AN4248.pdf */

#include <seos.h>
#include "stk_hand_detect.h"

/* Calculate result = y / x with y <= x, and both of x,y > 0 */
static int divide(int y, int x)
{
    const int MINDELTADIV = 1; /* final step size for divide */

    if (0 < y && 0 < x)
    {
        /* Set result to 0 and binary search step to 16384 = 0.5 */
        int temp; /* scratch */
        int result = 0; /* result = y/x range 0., 1. returned in range 0 to 32767 */
        int delta = 16384; /* delat on candidate result dividing each stage by factor of 2 */
                            /* set as 2^14 = 0.5 */

        if (y > x)
            y = x;

        /* Reduce quatization effects, boost x and y to the maximum signed 16 bit value */
        while (16384 > x && 16384 > y)
        {
            x = x + x;
            y = y + y;
        }

        /* Loop over binary sub-division algorithm solving for result * x = y */
        do {
            temp = result + delta;
            temp = (temp * x) / 32768;

            if (temp <= y)
                result += delta;

            delta = delta / 2;
        } while (MINDELTADIV <= delta); /* last loop is performed for MINDELTADIV == delta */

        return result;
    }
    else
        return (int)(-1);
}

/* Calculates 100*atan(y/x) range 0 to 9000 for all x, y positive in range 0 to 32767 */
static int hundredArctanDegQuad(int y, int x)
{
    const int K1 = 5701;
    const int K2 = -1645;
    const int K3 = 446;
    int angle; /* angle in degrees times 100 */
    int ratio; /* ratio of y/x or vice versa */
    int temp; /* temporary variable */

    /* check for pathological cases */
    if (0 == x)
    {
        if (0 == y)
            return 0;
        else
            return 9000;
    }

    if (y <= x)
        ratio = divide(y, x); /* return a fraction in range 0, to 32767 = 0. to 1. */
    else
        ratio = divide(x, y); /* return a fraction in range 0. to 32767 = 0. to 1. */

    /* First, third and fifth order polynomial approximation */
    angle = K1 * ratio;
    temp = (ratio / 32) * (ratio / 32) * (ratio / 32);
    angle += K2 * (temp / 32768);
    temp = (temp / 1048576) * (ratio / 32) * (ratio / 32);
    angle += K3 * (temp / 32768);
    angle /= 32768;

    /* Check if above 45 degrees */
    if (y > x)
        angle = (9000 - angle);

    /* For tidiness, limit result from range 0 to 9000 equals 0.0 to 90.0 degrees */
    if (0 > angle)
        angle = 0;
    if (9000 < angle)
        angle = 9000;

    return angle;
}

/* Calculates 100*atan2(y/x) = 100*atan2(y,x) in deg from x, y in range -32768 to 32767 */
static int hundredArctan2Deg(int y, int x)
{
    /* Check for 32767 which is not handled correctly */
    if (32767 < x)
        x = 32767;
    if (32767 < y)
        y = 32767;
    /* Check for -32768 which is not handled correctly */
    if (-32768 >= x)
        x = -32767;
    if (-32768 >= y)
        y = -32767;

    /* Check for quadrants */
    if (0 <= x && 0 <= y) /* range 0 to 90 degrees */
        return hundredArctanDegQuad(y, x);
    else if (0 > x && 0 <= y) /* range 90 to 180 degrees */
        return (int)18000 - hundredArctanDegQuad(y, (int)(-x));
    else if (0 > x && 0 > y) /* range -180 to -90 degrees */
        return (int)(-18000) + hundredArctanDegQuad((int)(-y), (int)(-x));
    else /* range -90 to 0 degree */
        return -hundredArctanDegQuad((int)(-y), x);
}

static int stk_atan(int y, int x)
{
    return hundredArctan2Deg(y, x) / (int)100;
}

static int stk_sqrt(int in)
{
    long root, bit;
    root = 0;
    for (bit = 0x40000000; bit > 0; bit >>= 2)
    {
        long trial = root + bit;
        root >>= 1;
        if (trial <= in)
        {
            root += bit;
            in -= trial;
        }
    }
    return (int)root;
}

int stk_hand_detect(int16_t *acc)
{
    int x_tilt_angle, y_tilt_angle, z_tilt_angle;;

    x_tilt_angle = 90 - stk_atan(stk_sqrt(acc[1]*acc[1] + acc[2]*acc[2]), acc[0]);
    y_tilt_angle = 90 - stk_atan(stk_sqrt(acc[0]*acc[0] + acc[2]*acc[2]), acc[1]);
    z_tilt_angle = 90 - stk_atan(stk_sqrt(acc[0]*acc[0] + acc[1]*acc[1]), acc[2]);
    //osLog(LOG_INFO, "%s, x_tilt_angle=%d, y_tilt_angle=%d, z_tilt_angle=%d\n", __FUNCTION__, x_tilt_angle, y_tilt_angle, z_tilt_angle);

#if 0
    if ((-90 < z_tilt_angle && -70 > z_tilt_angle)
            || ( 70 < y_tilt_angle && 90 > y_tilt_angle))
        return 1;
    else
        return 0;
#endif

    if ((-85 < z_tilt_angle && -50 > z_tilt_angle)
            || ((60 < z_tilt_angle && 85 > z_tilt_angle) && (-40 < y_tilt_angle && -20 > y_tilt_angle))) //wake up, screen on
	    return HD_WAKEUP_SCREENON_ACTION;
    else if (((30 < x_tilt_angle && 60 > x_tilt_angle) || (-60 < x_tilt_angle && -30 > x_tilt_angle))
            && (-90 < y_tilt_angle && -30 > y_tilt_angle)
            && (-20 < z_tilt_angle && 20 > z_tilt_angle)) //right/left side, screen off
        return HD_RIGHTLEFT_SIDE_SCREENOFF_ACTION;
    else if ((-90 < x_tilt_angle && -70 > x_tilt_angle)
            || (70 < x_tilt_angle && 90 > x_tilt_angle)) //front/back lie, screen off
        return HD_FRONTBACK_LIE_SCREENOFF_ACTION;
    else if (70 < z_tilt_angle && 90 > z_tilt_angle) //right/left lie, screen off
        return HD_RIGHTLEFT_LIE_SCREENOFF_ACTION;
    else
       return HD_NONE_ACTION;
}
