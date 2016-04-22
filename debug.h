/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description:  debug utilities based on http://c.learncodethehardway.org/book/ex20.html
 *
 *        Version:  1.0
 *        Created:  01/19/2016 03:33:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jignesh Darji (jignesh), jndarji@ncsu.edu
 *   Organization:  North Carolina State University
 *
 * =====================================================================================
 */


#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define LOG_FILE "/tmp/ramdisk.log"

#define DEBUGGING_MODE 1 
#define log_err(M, ...) if(DEBUGGING_MODE){ FILE* _log = fopen(LOG_FILE, "a"); do{ fprintf(_log, "[ERR] %06d : %06d %s | %s | %-15s | " M "\n", getpgid(0), getpid(), __DATE__, __TIME__, __FUNCTION__, ##__VA_ARGS__); } while(0); fclose(_log);}
#define log_inf(M, ...) if(DEBUGGING_MODE){ FILE* _log = fopen(LOG_FILE, "a"); do{ fprintf(_log, "[INF] %06d : %06d %s | %s | %-15s | " M "\n", getpgid(0), getpid(), __DATE__, __TIME__, __FUNCTION__, ##__VA_ARGS__); } while(0); fclose(_log);}
#define log_wrn(M, ...) if(DEBUGGING_MODE){ FILE* _log = fopen(LOG_FILE, "a"); do{ fprintf(_log, "[WRN] %06d : %06d %s | %s | %-15s | " M "\n", getpgid(0), getpid(), __DATE__, __TIME__, __FUNCTION__, ##__VA_ARGS__); } while(0); fclose(_log);}
#define log_dbg(M, ...) if(DEBUGGING_MODE){ FILE* _log = fopen(LOG_FILE, "a"); do{ fprintf(_log, "[DBG] %06d : %06d %s | %s | %-15s | " M "\n", getpgid(0), getpid(), __DATE__, __TIME__, __FUNCTION__, ##__VA_ARGS__); } while(0); fclose(_log);}

#endif

