#ifndef __AAL_IP_H__
#define __AAL_IP_H__

#if defined(MT6582) || defined(MT6592)
#define __AAL_10__
#elif defined(MT8127)
#else
#define __AAL_20__
#endif

#endif
