#ifndef _LOG_H_
#define _LOG_H_

#include <cstdio>

//#define NO_LOGERR
//#define NO_LOGINFO


#ifndef NO_LOGERR
# define tucube_logerr(err)    ({printf("error: %s. -> (File: %s, Line: %d)\n", err, __FILE__, __LINE__);})
#else
# define tucube_logerr(err) 
#endif

#ifndef NO_LOGINFO
# define tucube_loginfo(info)  ({printf("info: %s.\n", info);})
#else
# define tucube_loginfo(info) 
#endif

#endif /* _LOG_H_ */
