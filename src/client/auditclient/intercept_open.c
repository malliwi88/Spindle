/*
  This file is part of Spindle.  For copyright information see the COPYRIGHT 
  file in the top level directory, or at 
  https://github.com/hpc/Spindle/blob/master/COPYRIGHT

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License (as published by the Free Software
  Foundation) version 2.1 dated February 1999.  This program is distributed in the
  hope that it will be useful, but WITHOUT ANY WARRANTY; without even the IMPLIED
  WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms 
  and conditions of the GNU General Public License for more details.  You should 
  have received a copy of the GNU Lesser General Public License along with this 
  program; if not, write to the Free Software Foundation, Inc., 59 Temple
  Place, Suite 330, Boston, MA 02111-1307 USA
*/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "ldcs_api.h"
#include "client.h"
#include "client_heap.h"
#include "client_api.h"
#include "should_intercept.h"

static int (*orig_open)(const char *pathname, int flags, ...);
static int (*orig_open64)(const char *pathname, int flags, ...);
static FILE* (*orig_fopen)(const char *pathname, const char *mode);
static FILE* (*orig_fopen64)(const char *pathname, const char *mode);

/* returns:
   0 if not existent
   -1 could not check, use orig open
   1 exists, newpath contains real location */
static int do_check_file(const char *path, char **newpath) {
   char *myname, *newname;
  
   myname=(char *) path;

   check_for_fork();
   if (ldcsid < 0 || !use_ldcs) {
      debug_printf3("no ldcs: open file query %s\n", myname);
      return -1;
   }
   sync_cwd();

   debug_printf2("Open operation requesting file: %s\n", path);
   send_file_query(ldcsid, myname, &newname);
   debug_printf("Open file request returned %s -> %s\n", path, newname ? newname : "NULL");

   if (newname != NULL) {
      *newpath=newname;
      debug_printf3("file found under path %s\n", *newpath);
      return 1;
   } else {
      *newpath=NULL;
      errno = ENOENT;
      debug_printf3("file not found file, set errno to ENOENT\n");
      return(0);
   }
}

static int call_orig_open(const char *path, int oflag, mode_t mode, int is_64)
{
   test_log(path);
   if (is_64) {
      if (orig_open64)
         return orig_open64(path, oflag, mode);
      else 
         return open64(path, oflag, mode);
   }
   else {
      if (orig_open) {
         return orig_open(path, oflag, mode);
      }
      else {
         return open(path, oflag, mode);
      }
   }
}

int open_worker(const char *path, int oflag, mode_t mode, int is_64)
{
   int rc;
   char *newpath;
   int result, exists;
   
   result = open_filter(path, oflag);
   if (ldcsid < 0 || result == ORIG_CALL) {
      /* Use the original open */
      return call_orig_open(path, oflag, mode, is_64);
   }
   else if (result == REDIRECT) {
      /* Lookup and do open through local path */
      result = do_check_file(path, &newpath);
      if (result == 0) {
         /* File doesn't exist */
         set_errno(ENOENT);
         return -1;
      }
      else if (result < 0) {
         /* Spindle error, fallback to orig open */
         return call_orig_open(path, oflag, mode, is_64);
      }
      else {
         /* Successfully redirect open */
         debug_printf("Redirecting 'open' call, %s to %s\n", path, newpath);
         rc = call_orig_open(newpath, oflag, mode, is_64);
         spindle_free(newpath);
         return rc;
      }
   }
   else if (result == EXCL_OPEN) {
      /* The file is being exclusively created.  Short-circuit error return
         if it exists */
      debug_printf("Testing for existance before exclusive open of %s\n", path);
      result = send_existance_test(ldcsid, (char *) path, &exists);
      if (result == -1 || !exists) {
         debug_printf3("File %s does not exist, allowing exclusive open\n", path);
         return call_orig_open(path, oflag, mode, is_64);
      }
      else {
         debug_printf3("File %s exists, faking EEXIST error return\n", path);
         set_errno(EEXIST);
         return -1;
      }
   }
   assert(0);
   return -1;
}

static FILE *call_orig_fopen(const char *path, const char *mode, int is_64)
{
   if (is_64) {
      if (orig_fopen64)
         return orig_fopen64(path, mode);
      else 
         return fopen(path, mode);
   }
   else {
      if (orig_fopen)
         return orig_fopen(path, mode);
      else 
         return fopen(path, mode);
   }
}

FILE *fopen_worker(const char *path, const char *mode, int is_64)
{
   FILE *rc;
   char *newpath;
   int result, exists;
   
   result = fopen_filter(path, mode);
   if (ldcsid < 0 || result == ORIG_CALL) {
      /* Use the original open */
      return call_orig_fopen(path, mode, is_64);
   }
   else if (result == REDIRECT) {
      /* Lookup and do open through local path */
      result = do_check_file(path, &newpath);
      if (result == 0) {
         /* File doesn't exist */
         set_errno(ENOENT);
         return NULL;
      }
      else if (result < 0) {
         /* Spindle error, fallback to orig open */
         return call_orig_fopen(path, mode, is_64);
      }
      else {
         /* Successfully redirect open */
         debug_printf("Redirecting 'open' call, %s to %s\n", path, newpath);
         rc = call_orig_fopen(newpath, mode, is_64);
         spindle_free(newpath);
         return rc;
      }
   }
   else if (result == EXCL_OPEN) {
      /* The file is being exclusively created.  Short-circuit error return
         if it exists */
      debug_printf("Testing for existance before exclusive open of %s\n", path);
      result = send_existance_test(ldcsid, (char *) path, &exists);
      if (result == -1 || !exists) {
         debug_printf3("File %s does not exist, allowing exclusive open\n", path);
         return call_orig_fopen(path, mode, is_64);
      } 
      else {
         debug_printf3("File %s exists, faking EEXIST error return\n", path);
         set_errno(EEXIST);
         return NULL;
      }
   }
   assert(0);
   return NULL;
}

static int rtcache_open(const char *path, int oflag, ...)
{
   va_list argp;
   mode_t mode = (mode_t) 0;

   va_start(argp, oflag);
   if (oflag & O_CREAT)
      mode = va_arg(argp, mode_t);
   va_end(argp);

   debug_printf3("potential open redirection of %s\n", path);

   return open_worker(path, oflag, mode, 0);
}

static int rtcache_open64(const char *path, int oflag, ...)
{
   va_list argp;
   mode_t mode = (mode_t) 0;

   va_start(argp, oflag);
   if (oflag & O_CREAT)
      mode = va_arg(argp, mode_t);
   va_end(argp);

   debug_printf3("potential open64 redirection of %s\n", path);

   return open_worker(path, oflag, mode, 1);
}

static FILE *rtcache_fopen(const char *path, const char *mode)
{
   debug_printf3("potential fopen redirection of %s\n", path);
   return fopen_worker(path, mode, 0);
}

static FILE *rtcache_fopen64(const char *path, const char *mode)
{
   debug_printf3("potential fopen64 redirection of %s\n", path);
   return fopen_worker(path, mode, 1);
}

static int rtcache_close(int fd)
{
   /* Don't let applications (looking at you, tcsh) close our FDs */
   check_for_fork();
   int result = fd_filter(fd);
   if (result == ERR_CALL) {
      set_errno(EBADF);
      return -1;
   }
   return close(fd);
}

ElfX_Addr redirect_open(const char *symname, ElfX_Addr value)
{
   if (strcmp(symname, "open") == 0) {
      if (!orig_open)
         orig_open = (void *) value;
      return (ElfX_Addr) rtcache_open;
   }
   else if (strcmp(symname, "open64") == 0) {
      if (!orig_open64)
         orig_open64 = (void *) value;
      return (ElfX_Addr) rtcache_open64;
   }
   else if (strcmp(symname, "fopen") == 0) {
      if (!orig_fopen)
         orig_fopen = (void *) value;
      return (ElfX_Addr) rtcache_fopen;
   }
   else if (strcmp(symname, "fopen64") == 0) {
      if (!orig_fopen64)
         orig_fopen64 = (void *) value;
      return (ElfX_Addr) rtcache_fopen64;
   }
   else
      return (ElfX_Addr) value;
}

ElfX_Addr redirect_close(const char *symname, ElfX_Addr value)
{
   if (strcmp(symname, "close") == 0) {
      return (ElfX_Addr) rtcache_close;
   }
   return value;
}
