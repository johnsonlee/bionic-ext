#include <errno.h>
#include <stdio.h>

#include <mntent.h>

/**
 * missed in `bioni/libc/include/mntent.h'
 */
FILE *setmntent(__const char *__file, __const char *__mode)
{
#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return NULL;
}

/**
 * missed in `bioni/libc/include/mntent.h'
 */
int addmntent(FILE *__restrict __stream, __const struct mntent *__restrict __mnt) 
{
#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}

/**
 * missed in `bioni/libc/include/mntent.h'
 */
int endmntent(FILE *__stream) 
{
#ifndef NDEBUG
 	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}

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

