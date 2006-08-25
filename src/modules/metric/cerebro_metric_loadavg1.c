/*****************************************************************************\
 *  $Id: cerebro_metric_loadavg1.c,v 1.1 2006-08-25 16:11:25 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define LOADAVG1_FILE                "/proc/loadavg"
#define LOADAVG1_BUFLEN              4096
#define LOADAVG1_METRIC_MODULE_NAME  "loadavg1"
#define LOADAVG1_METRIC_NAME         "loadavg1"

/*
 * loadavg1_metric_setup
 *
 * loadavg1 metric module setup function.  Read and store the loadavg1
 * out of /proc.
 */
static int
loadavg1_metric_setup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * loadavg1_metric_cleanup
 *
 * loadavg1 metric module cleanup function
 */
static int
loadavg1_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * loadavg1_metric_get_metric_name
 *
 * loadavg1 metric module get_metric_name function
 */
static char *
loadavg1_metric_get_metric_name(void)
{
  return LOADAVG1_METRIC_NAME;
}

/*
 * loadavg1_metric_get_metric_period
 *
 * loadavg1 metric module get_metric_period function
 */
static int
loadavg1_metric_get_metric_period(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  *period = 60;
  return 0;
}

/*
 * loadavg1_metric_get_metric_value
 *
 * loadavg1 metric module get_metric_value function
 */
static int
loadavg1_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  int fd, len;
  float loadavg1, loadavg5, loadavg15;
  float *loadavgptr = NULL;
  char buf[LOADAVG1_BUFLEN];
  int rv = -1;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
 
  if ((fd = open(LOADAVG1_FILE, O_RDONLY, 0)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  memset(buf, '\0', LOADAVG1_BUFLEN);
  if ((len = read(fd, buf, LOADAVG1_BUFLEN)) < 0)
    {
      CEREBRO_DBG(("read: %s", strerror(errno)));
      goto cleanup;
    }

  if (sscanf(buf, "%f %f %f", &loadavg1, &loadavg5, &loadavg15) != 3)
    {
      CEREBRO_DBG(("loadavg1 file parse error"));
      goto cleanup;
    }
  
  if (!(loadavgptr = (float *)malloc(sizeof(float))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  *loadavgptr = loadavg1;

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_FLOAT;
  *metric_value_len = sizeof(float);
  *metric_value = (void *)loadavgptr;

  rv = 0;
 cleanup:
  close(fd);
  if (rv < 0 && loadavgptr)
    free(loadavgptr);
  return rv;
}

/*
 * loadavg1_metric_destroy_metric_value
 *
 * loadavg1 metric module destroy_metric_value function
 */
static int
loadavg1_metric_destroy_metric_value(void *metric_value)
{
  if (!metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  free(metric_value);
  return 0;
}

/*
 * loadavg1_metric_get_metric_thread
 *
 * loadavg1 metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
loadavg1_metric_get_metric_thread(void)
{
  return NULL;
}

/*
 * loadavg1_metric_send_heartbeat_function_pointer
 *
 * loadavg1 metric module send_heartbeat_function_pointer function
 */
static int
loadavg1_metric_send_heartbeat_function_pointer(Cerebro_metric_send_heartbeat function_pointer)
{
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info loadavg1_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    LOADAVG1_METRIC_MODULE_NAME,
    &loadavg1_metric_setup,
    &loadavg1_metric_cleanup,
    &loadavg1_metric_get_metric_name,
    &loadavg1_metric_get_metric_period,
    &loadavg1_metric_get_metric_value,
    &loadavg1_metric_destroy_metric_value,
    &loadavg1_metric_get_metric_thread,
    &loadavg1_metric_send_heartbeat_function_pointer,
  };
