/*
 * =====================================================================================
 *
 *       Filename:  ramdisk.c
 *
 *    Description:  FUSE-based RAMFS
 *
 *        Version:  1.0
 *        Created:  04/22/2016 12:33:25 PM
 *      Reference:  based on the hello.c example code from FUSE 2.6
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jignesh Darji (jignesh), jndarji@ncsu.edu
 *   Organization:  North Carolina State University
 *
 * =====================================================================================
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "debug.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static unsigned int ramfs_size = 0;
static ramnode_id curr_id = 1;

map<string, ramnode_id> m_path;
map<ramnode_id, ramnode*> m_node;

static int ramdiskGetAddr(const char *path, struct stat *stbuf)
{
    log_dbg("begin path: %s", path);
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else
		res = -ENOENT;

    log_dbg("end");
	return res;
}

static int ramdiskReadDir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

    log_dbg("end");
	return 0;
}

static int ramdiskOpen(const char *path, struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

    log_dbg("end");
	return 0;
}

static int ramdiskRead(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

    log_dbg("end");
	return size;
}

static int ramdiskWrite(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{

}

static int ramdiskMakeDir(const char* path, mode_t mode) 
{
}

static int ramdiskRemovedir(const char * path) 
{
}

static int ramdiskOpenDir(const char * path, struct fuse_file_info * fi) 
{
}

static int ramdiskUnlink(const char * path) 
{
}

static int ramdiskCreate(const char * path, mode_t mode, struct fuse_file_info * fi) 
{
}


static struct fuse_operations ramdisk_oper;

int main(int argc, char *argv[])
{
    log_dbg("");
    ramdisk_oper.getattr = ramdiskGetAddr;
    ramdisk_oper.readdir = ramdiskReadDir;
    ramdisk_oper.opendir = ramdiskOpenDir;
    ramdisk_oper.open    = ramdiskOpen;
    ramdisk_oper.read    = ramdiskRead;
    ramdisk_oper.write   = ramdiskWrite;
    ramdisk_oper.mkdir   = ramdiskMakeDir;
    ramdisk_oper.rmdir   = ramdiskRemoveDir;
    ramdisk_oper.unlink  = ramdiskUnlink;
    ramdisk_oper.create  = ramdiskCreate;
	return fuse_main(argc, argv, &ramdisk_oper, NULL);
}
