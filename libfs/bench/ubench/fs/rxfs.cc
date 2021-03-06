#include <stdlib.h>
#include "rxfs/client/libfs.h"
#include "ubench/fs/rxfs.h"

int dummy(const char*f, const char *t)
{
	return 0;
}

int (*fs_open)(const char*, int flags) = rxfs_open;
int (*fs_open2)(const char*, int flags, mode_t mode) = rxfs_open2;
int (*fs_unlink)(const char*) = rxfs_unlink;
int (*fs_rename)(const char*, const char*) = dummy;
int (*fs_close)(int fd) = rxfs_close;
int (*fs_fsync)(int fd) = rxfs_fsync;
int (*fs_sync)() = rxfs_sync;
int (*fs_mkdir)(const char*, int mode) = rxfs_mkdir;
ssize_t (*fs_write)(int fd, const void* buf, size_t count) = rxfs_write;
ssize_t (*fs_read)(int fd, void* buf, size_t count) = rxfs_read;
ssize_t (*fs_pwrite)(int fd, const void* buf, size_t count, off_t offset) = rxfs_pwrite;
ssize_t (*fs_pread)(int fd, void* buf, size_t count, off_t offset) = rxfs_pread;

RFile* (*fs_fopen)(const char*, int flags) = rxfs_fopen;
ssize_t (*fs_fread)(RFile* fp, void* buf, size_t count) = rxfs_fread;
ssize_t (*fs_fpread)(RFile* fp, void* buf, size_t count, off_t offset) = rxfs_fpread;
int (*fs_fclose)(RFile* fp) = rxfs_fclose;


int 
Init(int debug_level, const char* xdst)
{
	int ret;

	rxfs_init3(xdst, debug_level);
	rxfs_mount("/tmp/stamnos_pool", "/rxfs", "rxfs", 0);
	return 0;
}


int
ShutDown()
{
	rxfs_shutdown();
	return 0;
}
