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

void updateDiskSize(int size_change);
void createRootNode();

#endif
