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

#include <iostream>
#include <cstdlib>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <map>
#include <sys/stat.h>
#include <malloc.h>

#include "ramdisk.h"
#include "debug.h"
#include "ramnode.h"
#include "constants.h"

static int ramfs_size = 0;
static int ramfs_max_size = 0;
static ramnode_id curr_id = 1;
static struct fuse_operations ramdisk_oper;
map<string, ramnode_id>     m_path;
map<ramnode_id, ramnode*>   m_node;

static int ramdiskGetAttr(const char *path, struct stat *stbuf)
{
    log_dbg("begin path: %s", path);
    memset(stbuf, 0, sizeof(struct stat));

    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_inf("DOESN'T EXIST");
        return -ENOENT;
    }

    int retVal = 0;

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    stbuf->st_uid = node->uid;
    stbuf->st_gid = node->gid;
    stbuf->st_atime = node->atime;
    stbuf->st_mtime = node->mtime;
    stbuf->st_ctime = node->ctime;

    if (node->type == TYPE_DIR) {
        stbuf->st_mode = S_IFDIR | node->mode;
        stbuf->st_nlink = 2;
        stbuf->st_size = SIZE_DIR;
    } else if (node->type == TYPE_FILE) {
        stbuf->st_mode = S_IFREG | node->mode;
        stbuf->st_nlink = 1;
        stbuf->st_size = node->size;
    }

    log_dbg("end");
    return retVal;
}

static int ramdiskReadDir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);

    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("path: %s NOT FOUND", path);
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    if(node->type != TYPE_DIR)
    {
        log_err("It's NOT a DIRECTORY");
        return -ENOENT;
    }

    if(strcmp(path, "/") != 0)
    {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
    }

    list<ramnode_id>::const_iterator itr; 
    for(itr = node->child.begin(); itr != node->child.end(); ++itr)
    {
        ramnode* temp_child = m_node[*itr];
        string temp_name = getFilenameFromPath(temp_child->name);
        filler(buf, temp_name.c_str(), NULL, 0);
    }

    log_dbg("end");
    return 0;
}

static int ramdiskOpen(const char *path, struct fuse_file_info *fi)
{
    log_dbg("begin path: %s", path);
    
    int retVal = 0;

    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("path: %s NOT FOUND", path);
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    // check if it's a directory
    if(node->type == TYPE_DIR)
    {
        log_err("It's a DIRECTORY");
        return -ENOENT;
    }

    log_dbg("end");
    return retVal;
}

static int ramdiskRead(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi)
{
    log_dbg("begin path: %s size: %d offset: %d", path, (int) size, (int) offset);
    
    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    // check if it's a directory
    if(node->type == TYPE_DIR)
    {
        log_err("It's a DIRECTORY");
        return -ENOENT;
    }

    if(node->size - offset <= (unsigned int) size)
        size = node->size - offset;
    
    memcpy(buf, node->data + offset, size);

    log_dbg("end size: %d", (int)size);
    return size;
}

static int ramdiskWrite(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
    log_dbg("begin path: %s offset: %d size: %d", path, (int) offset, (int)size);
    
    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    // check if it's a directory
    if(node->type == TYPE_DIR)
    {
        log_err("It's a DIRECTORY");
        return -ENOENT;
    }

    if(ramfs_size + size > (unsigned int) ramfs_max_size)
    {
        log_err("NO SPACE");
        return -ENOSPC;
    }

    if(node->data == NULL)
    {
        node->data = (char*) malloc(size);
        memcpy(node->data, buf, size);
    }
    else
    {
        node->data = (char*) realloc(node->data, offset + size);
        memcpy(node->data + offset, buf, size);
    }

    ramfs_size += offset + size - node->size;
    node->size = offset + size;

    log_dbg("end size: %d ramfs_size: %d", (int) size, ramfs_size);
    return size;
}

static int ramdiskMakeDir(const char* path, mode_t mode) 
{
    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }
    string path_string = path;
    return createDirNode(path_string, mode);
}

static int ramdiskRemoveDir(const char * path) 
{
    log_dbg("begin path: %s", path);
    
    int retVal = 0;

    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    // check if it's a directory
    if(node->type != TYPE_DIR)
    {
        log_err("It's NOT a DIRECTORY");
        return -ENOENT;
    }

    // No deletion allowed if directory isn't empty
    if(!node->child.empty())
    {
        log_err("NOT EMPTY");
        return -EPERM;
    }

    // Remove from parent's list
    string parent = getParentFromPath(path_string);
    ramnode_id parent_id = m_path[parent];
    ramnode* parent_node = m_node[parent_id];

    parent_node->child.remove(id);

    // Remove from maps
    m_path.erase(path_string);
    m_node.erase(id);

    // Free the memory allocated
    free(node->data);
    delete node;
   
    // Update disk size
    ramfs_size -= sizeof(ramnode);
    log_dbg("end ramfs_size: %d", ramfs_size);
    return retVal;
}

static int ramdiskOpenDir(const char * path, struct fuse_file_info * fi) 
{
    log_dbg("begin path: %s", path);
    int retVal = 0;
    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    if(node->type != TYPE_DIR)
    {
        log_err("It's NOT a DIRECTORY");
        return -ENOENT;
    }

    log_dbg("end");
    return retVal;
}

static int ramdiskUnlink(const char * path) 
{
    log_dbg("begin path: %s", path);
    
    int retVal = 0;

    if(path == NULL)
    {
        log_err("Path is EMPTY");
        return -ENOENT;
    }

    string path_string = path; 
    if(m_path.find(path_string) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    ramnode_id id = m_path[path_string];
    ramnode* node = m_node[id];

    // check if it's a directory
    if(node->type == TYPE_DIR)
    {
        log_err("It's a DIRECTORY");
        return -ENOENT;
    }

    // Remove from parent's list
    string parent = getParentFromPath(path_string);
    ramnode_id parent_id = m_path[parent];
    ramnode* parent_node = m_node[parent_id];

    parent_node->child.remove(id);

    // Remove from maps
    m_path.erase(path_string);
    m_node.erase(id);

    int freed_size = sizeof(ramnode) + node->size; 

    // Free the memory allocated
    free(node->data);
    delete node;
   
    // Update disk size
    ramfs_size -= freed_size;

    log_dbg("end ramfs_size: %d", ramfs_size);
    return retVal;
}

static int ramdiskCreate(const char * path_c, mode_t mode, struct fuse_file_info * fi) 
{
    log_dbg("begin path: %s mode: %d", path_c, mode);

    int retVal = 0;

    string path = path_c;
    string parent(getParentFromPath(path));
    string fname(getFilenameFromPath(path));

    // check whether parent exists
    if(m_path.find(parent) == m_path.end())
    {
        log_err("DOESN'T EXIST");
        return -ENOENT;
    }

    // Check whether parent is a directory
    ramnode_id parent_id = m_path[parent];
    ramnode *parent_node = m_node[parent_id];

    if(parent_node->type != TYPE_DIR)
    {
        log_err("It's NOT a DIRECTORY");
        return -ENOENT;
    }

    // Create only if within limits
    int fs_size = ramfs_size + sizeof(ramnode);
    if(fs_size > ramfs_max_size)
    {
        log_err("NO SPACE");
        return -ENOSPC;
    }


    parent_node->child.push_back(curr_id);
    ramnode *node = new ramnode();

    // ID
    node->id = curr_id++;

    // Metadata
    node->name   = path;
    node->type   = TYPE_FILE; 
    node->size   = 0;
    node->mode   = S_IFREG | mode;

    // owner info
    node->gid = fuse_get_context()->gid;
    node->uid = fuse_get_context()->uid;

    // Time
    time(&node->atime);
    time(&node->mtime);
    time(&node->ctime);

    node->data   = NULL;

    //Update disk size
    ramfs_size = fs_size;

    // Update Maps
    m_path[node->name]   = node->id;
    m_node[node->id]     = node;

    log_dbg("end ramfs_size: %d", ramfs_size);
    return retVal; 
}

static int ramdiskFlush(const char* path, struct fuse_file_info* fi)
{
    log_dbg();
    return 0;
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
            log_err("DOESN'T EXIST");
            return -ENOENT;
        }

        ramnode_id parent_id = m_path[parent];
        ramnode *parent_node = m_node[parent_id];

        parent_node->child.push_back(curr_id);
    }

    // Create only if within limits
    int fs_size = ramfs_size + sizeof(ramnode);
    if(fs_size > ramfs_max_size)
    {
        log_err("NO SPACE");
        return -ENOSPC;
    }

    ramnode *node = new ramnode();

    // ID
    node->id = curr_id++;

    // Metadata
    node->name   = path;
    node->type   = TYPE_DIR; 
    node->size   = 0;
    node->mode   = S_IFDIR | mode;

    // owner info
    node->gid = fuse_get_context()->gid;
    node->uid = fuse_get_context()->uid;

    // Time
    time(&node->atime);
    time(&node->mtime);
    time(&node->ctime);

    node->data   = NULL;

    //Update disk size
    ramfs_size += sizeof(ramnode);

    // Update Maps
    m_path[node->name]   = node->id;
    m_node[node->id]     = node;

    log_dbg("end ramfs_size: %d", ramfs_size);
    return retVal; 
}

int createRootNode()
{
    log_dbg("");

    string root_path = "/";
    return createDirNode(root_path, ACCESSPERMS);
}

int main(int argc, char *argv[])
{
    log_dbg("");

    if (argc != 3) 
    {
        cout << "Usage: ./ramdisk <dir> <size(MB)>" << endl;
        exit(-1);
    }

    int disk_size = atoi(argv[2]);
    ramfs_max_size = disk_size * 1024 * 1024;

    ramdisk_oper.getattr = ramdiskGetAttr;
    ramdisk_oper.readdir = ramdiskReadDir;
    ramdisk_oper.opendir = ramdiskOpenDir;
    ramdisk_oper.open    = ramdiskOpen;
    ramdisk_oper.read    = ramdiskRead;
    ramdisk_oper.write   = ramdiskWrite;
    ramdisk_oper.mkdir   = ramdiskMakeDir;
    ramdisk_oper.rmdir   = ramdiskRemoveDir;
    ramdisk_oper.unlink  = ramdiskUnlink;
    ramdisk_oper.create  = ramdiskCreate;
    ramdisk_oper.flush   = ramdiskFlush;

    createRootNode();

    int new_argc = argc - 1;
    char *new_argv[2];
    new_argv[0] = argv[0];
    new_argv[1] = argv[1];
    return fuse_main(new_argc, new_argv, &ramdisk_oper, NULL);
}

