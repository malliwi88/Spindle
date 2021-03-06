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

#if !defined(SPINDLE_LAUNCH_H_)
#define SPINDLE_LAUNCH_H_

#if defined(__cplusplus)
extern "C" {
#endif
/* Bitfield values for opts parameter */
#define OPT_COBO       (1 << 1)
#define OPT_DEBUG      (1 << 2)
#define OPT_FOLLOWFORK (1 << 3)
#define OPT_PRELOAD    (1 << 4)
#define OPT_PUSH       (1 << 5)
#define OPT_PULL       (1 << 6)
#define OPT_RELOCAOUT  (1 << 7)
#define OPT_RELOCSO    (1 << 8)
#define OPT_RELOCEXEC  (1 << 9)
#define OPT_RELOCPY    (1 << 10)
#define OPT_STRIP      (1 << 11)
#define OPT_NOCLEAN    (1 << 12)
#define OPT_NOHIDE     (1 << 13)
#define OPT_REMAPEXEC  (1 << 14)
#define OPT_SEC        ((1 << 15) | (1 << 16))

#define OPT_SET_SEC(OPT, X) OPT |= (X << 16)
#define OPT_GET_SEC(OPT) ((OPT >> 16) & 3)
#define OPT_SEC_MUNGE 0
#define OPT_SEC_KEYLMON 1
#define OPT_SEC_KEYFILE 2
#define OPT_SEC_NULL 3

/* Possible values for use_launcher */
#define srun_launcher (1 << 0)
#define serial_launcher (1 << 1)
#define openmpi_launcher (1 << 2)
#define wreckrun_launcher (1 << 3)
#define marker_launcher (1 << 4)

/* Possible values for startup_type */
#define startup_serial 0
#define startup_lmon 1
#define startup_hostbin 2

/* Parameters for configuring Spindle */
typedef struct {
   /* A unique number that will be used to identify this spindle session */
   unsigned int number;

   /* The beginning port in a range that will be used for server->server communication */
   unsigned int port;

   /* The number of ports in the port range */
   unsigned int num_ports;

   /* A bitfield of the above OPT_* values */
   unsigned int opts;

   /* A unique (and hopefully random) shared-secret number that all servers will need
      to provide to join the Spindle network */
   unsigned int shared_secret;

   /* The type of the MPI launcher */
   unsigned int use_launcher;

   /* The mechanism used to start Spindle daemons */
   unsigned int startup_type;

   /* The local-disk location where Spindle will store its cache */
   char *location;

   /* Colon-seperated list of directories where Python is installed */
   char *pythonprefix;

   /* Name of a white-space delimited file containing a list of files that will be preloaded */
   char *preloadfile;
} spindle_args_t;

/* Functions used to startup Spindle on the front-end. Init returns after finishing start-up,
   and it is the caller's responsibility to call spindleCloseFE when the servers terminate*/
int spindleInitFE(const char **hosts, spindle_args_t *params);
int spindleCloseFE();

/* Runs the server process on a BE, returns when server is done */
int spindleRunBE(unsigned int port, unsigned int num_ports, unsigned int shared_secret, int (*post_setup)(spindle_args_t *));

/* Bitmask of values for the test_launchers parameter */
#define TEST_PRESETUP 1<<0
#define TEST_SERIAL   1<<1
#define TEST_SLURM    1<<2
#define TEST_FLUX     1<<3

#if defined(__cplusplus)
}
#endif

#endif

