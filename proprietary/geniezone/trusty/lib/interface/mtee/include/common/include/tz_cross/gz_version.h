/*
 * Header file for GenieZone API version control
 * version = Main.Sub
*/
/* Use lk version if exists */
#ifdef VERSION_MAJOR
#define GZ_API_MAIN_VERSION VERSION_MAJOR
#else
#define GZ_API_MAIN_VERSION 2
#endif

#ifdef  VERSION_MINOR
#define GZ_API_SUB_VERSION VERSION_MINOR
#else
#define GZ_API_SUB_VERSION 0
#endif

#ifdef  VERSION_DEV
#define GZ_API_DEV_VERSION VERSION_DEV
#else
#define GZ_API_DEV_VERSION 0
#endif

