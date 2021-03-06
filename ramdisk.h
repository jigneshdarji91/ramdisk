/*
 * =====================================================================================
 *
 *       Filename:  ramdisk.h
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

#ifndef __RAMDISK__
#define __RAMDISK__

#include <string>
#include "ramnode.h"

using namespace std;

void updateDiskSize(int size_change);
string getParentFromPath(string path);
string getFilenameFromPath(string path);

int createDirNode(string path, mode_t mode);
int createRootNode();

#endif
