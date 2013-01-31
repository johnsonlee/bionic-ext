#include <errno.h>
#include <stdio.h>

/**
 * missed in `bioni/libc/include/pwd.h'
 */
struct passwd* getpwent(void)
{
  	errno = 0;

#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return NULL;
}

/**
 * missed in `bioni/libc/include/pwd.h'
 */
int setpwent(void)
{
#ifndef NDEBUG
 	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}
