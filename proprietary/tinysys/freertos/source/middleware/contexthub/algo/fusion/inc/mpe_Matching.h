#ifndef __MPE_MATCHING_H_INCLUDED__
#define __MPE_MATCHING_H_INCLUDED__

//#define NODE_SIZE 362
//#define WAY_SIZE 56

//#define NODE_SIZE 3951
//#define WAY_SIZE 840

//#define NODE_SIZE 80
//#define WAY_SIZE 8

//#define NODE_SIZE 2335
//#define WAY_SIZE 452

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VEC
{
    double x;
    double y;
} VEC, *LPVEC;

typedef struct ROAD
{
    VEC p1;
    VEC p2;
    double direction;
} ROAD, *LPROAD;


float find_matching_point(VEC SensorLocation, ROAD mapRoad, LPVEC pMatchLocation_meter, LPVEC pMatchLocation_degree);
float find_road_direction_deviation(ROAD road_lla, float bearing, float *road_direction);
void dr_lla2utm_int(double lat, double log, double alt);
void maplla2utm(double* x, double* y, double* lon, double* lat);
void dr_lla2utm(double *x, double *y, double lon_deg, double lat_deg);
void dr_utm2lla(double x, double y, double *lon_deg, double *lat_deg);



#ifdef __cplusplus
}
#endif

#endif /* __MPE_MATCHING_H_INCLUDED__ */
