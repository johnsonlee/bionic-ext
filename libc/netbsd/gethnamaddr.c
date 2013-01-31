#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>

int gethostbyaddr_r(const void *addr, int len, int type,
        struct hostent *ret, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop)
{
#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}

int gethostbyname_r(const char *name, struct hostent *ret,
        char *buffer, size_t bufflen, struct hostent **result, int *h_errnop)
{
#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1;
}

int gethostent_r(struct hostent *ret, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop)
{
#ifndef NDEBUG
	printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

	return -1; 
}

