#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/time.h>

#define	RUSAGE_SELF	0		
#define	RUSAGE_CHILDREN	-1	

struct rusage {
  	struct timeval ru_utime;
	struct timeval ru_stime;
};

int	getrusage (int, struct rusage);

#endif