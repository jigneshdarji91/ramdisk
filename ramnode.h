/*
 * =====================================================================================
 *
 *       Filename:  ramnode.h
 *
 *    Description:  inode structure implementation for ramfs
 *
 *        Version:  1.0
 *        Created:  04/22/2016 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jignesh Darji (jignesh), jndarji@ncsu.edu
 *   Organization:  North Carolina State University
 *
 * =====================================================================================
 */

#ifndef __RAMNODE__
#define __RAMNODE__

#include <list>
#include <string>

typedef unsigned int ramnode_id;
typedef unsigned int ramnode_size;

typedef enum _ramnode_type 
{
    TYPE_FILE,
    TYPE_DIR
} ramnode_type; 


typedef struct _ramnode 
{
    // ID
    ramnode_id          id;

    // Metadata
    string              name;
    ramnode_type        type;
    ramnode_size        size;

    //Time
    time_t  time_access;
    time_t  time_modified;
    time_t  time_create;
    
    list<ramnode_id>    child;

} ramnode;

#endif /* __RAMNODE__ */
