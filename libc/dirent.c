#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

struct DIR
{
	int              _DIR_fd;
	size_t           _DIR_avail;
	struct dirent*   _DIR_next;
	pthread_mutex_t  _DIR_lock;
	struct dirent    _DIR_buff[15];
};

long telldir(DIR *dirp)
{
	if (!dirp) {
		errno = EBADF;
		return -1;
	}

	return dirp->_DIR_next->d_off;
}

void seekdir(DIR *dirp, long offset)
{
	if (!dirp) {
		errno = EBADF;
		return;
	}

	pthread_mutex_lock(&dirp->_DIR_lock);
	lseek(dirp->_DIR_fd, offset, SEEK_SET);
	dirp->_DIR_avail = 0;
	pthread_mutex_unlock(&dirp->_DIR_lock);
}
