#include <stdio.h>

/**
 * missed in `bionic/libc/kernel/common/linux/quota.h'
*/
long quotactl(unsigned int cmd, const char *special, int id, caddr_t addr)
{
#ifndef NDEBUG
  	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}
