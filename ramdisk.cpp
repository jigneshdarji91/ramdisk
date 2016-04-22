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
#include <libgen.h>
#include <map>
#include <sys/stat.h>

#include "ramdisk.h"
#include "debug.h"
#include "ramnode.h"
#include "constants.h"

static unsigned int ramfs_size = 0;
static ramnode_id curr_id = 1;
static struct fuse_operations ramdisk_oper;

map<string, ramnode_id>     m_path;
map<ramnode_id, ramnode*>   m_node;

static int ramdiskGetAddr(const char *path, struct stat *stbuf)
{
    log_dbg("begin path: %s", path);
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
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

    log_dbg("end");
	return 0;
}

static int ramdiskOpen(const char *path, struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
    int retVal = 0;

    log_dbg("end");
	return retVal;
}

static int ramdiskRead(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
    log_dbg("end");
	return size;
}

static int ramdiskWrite(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    log_dbg("end");
    return retVal;
}

static int ramdiskMakeDir(const char* path, mode_t mode) 
{
    if(path )
    {
        return -ENOENT;
    }
    string path_string = path;
    return createDirNode(path_string, mode);
}

static int ramdiskRemoveDir(const char * path) 
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    log_dbg("end");
    return retVal;
}

static int ramdiskOpenDir(const char * path, struct fuse_file_info * fi) 
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    log_dbg("end");
    return retVal;
}

static int ramdiskUnlink(const char * path) 
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    log_dbg("end");
    return retVal;
}

static int ramdiskCreate(const char * path, mode_t mode, struct fuse_file_info * fi) 
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    log_dbg("end");
    return retVal;
}



string getParentFromPath(string path)
{
    char* path_str  = strdup(path.c_str());
    char* dir       = dirname(path_str);
    string parent(dir);

    log_dbg("path: %s parent: %s", path.c_str(), parent.c_str());
    return parent; 
}

string getFilenameFromPath(string path)
{
    char* path_str  = strdup(path.c_str());
    char* fname       = basename(path_str);
    string filename(fname);

    log_dbg("path: %s filename: %s", path.c_str() , filename.c_str());
    return filename; 
}

int createDirNode(string path, mode_t mode)
{
    log_dbg("begin path: %s mode: %d", path.c_str(), mode);

    int retVal = 0;

    if(path.compare("/") != 0)
    {
        string parent(getParentFromPath(path));
        string fname(getFilenameFromPath(path));

        if(m_path.find(parent) == m_path.end())
        {
            return -ENOENT;
        }

        ramnode_id parent_id = m_path[parent];
        ramnode *parent_node = m_node[parent_id];

        parent_node->child.push_back(curr_id);
    }

    ramnode *node = new ramnode();

    // ID
    node->id = curr_id++;

    // Metadata
    node->name   = path;
    node->type   = TYPE_DIR; 
    node->size   = 0;
    node->mode   = mode;

    // Time
    time(&node->time_create);
    time(&node->time_access);
    time(&node->time_modified);

    node->data   = NULL;

    //Update disk size
    updateDiskSize(sizeof(ramnode) + node->name.length());

    // Update Maps
    m_path[node->name]   = node->id;
    m_node[node->id]     = node;

    log_dbg("end");
    return retVal; 
}

int createRootNode()
{
    log_dbg("begin");

    string root_path = "/";
    return createDirNode(root_path, ACCESSPERMS);
}

void updateDiskSize(int size_change)
{
    ramfs_size += size_change;
    log_dbg("new: %d change:%d", ramfs_size, size_change);
}

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


    createRootNode();

	return fuse_main(argc, argv, &ramdisk_oper, NULL);
}
