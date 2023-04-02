
#if !defined(resusage_H)
#define resusage_H

/* header where rusage and time structs are defined */

#ifdef FOUR
#define NOTOTHER 1
#include <sys/time.h>
#include <sys/resource.h>
struct rusage timestuff;
#endif

#ifdef FIVE
#define NOTOTHER 1
#include <sys/types.h>
#include <sys/param.h>
#include <sys/times.h>
struct tms timestuff;
#endif

/* define macros for time and resident memory usage checks */

#ifdef NOTOTHER

#ifdef FOUR                     /* 4.2,3BSD (tested: Sun4, IBM6000, DEC5000) */
static double dtime = 0.0;
static long stime, utime;
#define starttimer getrusage(RUSAGE_SELF, &timestuff); \
stime = timestuff.ru_utime.tv_sec; \
utime = timestuff.ru_utime.tv_usec
#define stoptimer getrusage(RUSAGE_SELF, &timestuff); \
dtime = (double)(timestuff.ru_utime.tv_sec - stime) \
        + 1.0e-6*(double)(timestuff.ru_utime.tv_usec - utime)
#endif /* FOUR */

#ifdef FIVE                     /* for System V (tested: HP300) */
static double dtime = 0.0;
static long stime, utime;
#define starttimer times(&timestuff); \
utime = timestuff.tms_utime
#define stoptimer times(&timestuff); \
dtime = (timestuff.tms_utime)-utime; \
dtime /= HZ
#endif /* FIVE */

#else                           /* default - no timers */

#define starttimer              /*  */
#define stoptimer               /*  */
const double dtime = 0.0;

#endif /* NOTOTHER */

#endif
