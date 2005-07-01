/*****************************************************************************\
 *  $Id: monitor_module.c,v 1.11 2005-07-01 17:13:50 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_monitor_module.h"

#include "monitor_module.h"
#include "module_util.h"

#include "debug.h"
#include "ltdl.h"

#define MONITOR_FILENAME_SIGNATURE  "cerebro_monitor_"
#define MONITOR_MODULE_INFO_SYM     "monitor_module_info"
#define MONITOR_MODULE_DIR          MONITOR_MODULE_BUILDDIR "/.libs"
#define MONITOR_MODULE_MAGIC_NUMBER 0x33882233

/* 
 * struct monitor_module
 *
 * monitor module handle
 */
struct monitor_module
{
  int32_t magic;
  unsigned int modules_max;
  unsigned int modules_count;
  lt_dlhandle *dl_handle;
  struct cerebro_monitor_module_info **module_info;
};

/*
 * _monitor_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int 
_monitor_module_cb(void *handle, void *dl_handle, void *module_info)
{
  monitor_modules_t monitor_handle;
  struct cerebro_monitor_module_info *monitor_module_info;
  lt_dlhandle monitor_dl_handle;

  if (!handle || !dl_handle || !module_info)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  monitor_handle = handle;
  monitor_module_info = module_info;
  monitor_dl_handle = dl_handle;

  if (monitor_handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!monitor_module_info->monitor_module_name
      || !monitor_module_info->setup
      || !monitor_module_info->cleanup
      || !monitor_module_info->metric_name
      || !monitor_module_info->metric_update)
    {
      CEREBRO_DBG(("invalid module info"));
      return 0;
    }

  if (monitor_handle->modules_count < monitor_handle->modules_max)
    {
      monitor_handle->dl_handle[monitor_handle->modules_count] = monitor_dl_handle;
      monitor_handle->module_info[monitor_handle->modules_count] = monitor_module_info;
      monitor_handle->modules_count++;
      return 1;
    }
  else
    return 0;
}

monitor_modules_t 
monitor_modules_load(unsigned int modules_max)
{
  struct monitor_module *handle = NULL;
  int rv;

  if (!modules_max)
    {
      CEREBRO_DBG(("modules_max invalid"));
      return NULL;
    }

  if (module_setup() < 0)
    return NULL;
                                                                                    
  if (!(handle = (struct monitor_module *)malloc(sizeof(struct monitor_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct monitor_module));
  handle->magic = MONITOR_MODULE_MAGIC_NUMBER;
  handle->modules_max = modules_max;
  handle->modules_count = 0;

  if (!(handle->dl_handle = (lt_dlhandle *)malloc(sizeof(lt_dlhandle)*handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(handle->dl_handle, '\0', sizeof(lt_dlhandle)*handle->modules_max);
  
  if (!(handle->module_info = (struct cerebro_monitor_module_info * *)malloc(sizeof(struct cerebro_monitor_module_info *)*handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(handle->module_info, '\0', sizeof(struct cerebro_monitor_module_info *)*handle->modules_max);

  if ((rv = find_and_load_modules(MONITOR_MODULE_DIR,
                                  NULL,
                                  0,
                                  MONITOR_FILENAME_SIGNATURE,
                                  _monitor_module_cb,
                                  MONITOR_MODULE_INFO_SYM,
                                  handle,
                                  handle->modules_max)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  /* Responsibility of caller to call count to see if no modules were
   * loaded 
   */

 out:
  return handle;

 cleanup:
  if (handle)
    {
      if (handle->dl_handle)
        {
          int i;
          for (i = 0; i < handle->modules_count; i++)
            lt_dlclose(handle->dl_handle[i]);
          free(handle->dl_handle);
        }
      if (handle->module_info)
        free(handle->module_info);
      free(handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * _handle_check
 *
 * Check for proper monitor module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(monitor_modules_t handle)
{
  if (!handle || handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

/*
 * _handle_index_check
 *
 * Check for proper monitor module handle and index
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_index_check(monitor_modules_t handle, unsigned int index)
{
  if (_handle_check(handle) < 0)
    return -1;

  if (!(index < handle->modules_count))
    {
      CEREBRO_DBG(("invalid index"));
      return -1;
    }

  return 0;
}

int 
monitor_modules_unload(monitor_modules_t handle)
{
  int i;

  if (_handle_check(handle) < 0)
    return -1;

  for (i = 0; i < handle->modules_count; i++)
    monitor_module_cleanup(handle, i);

  if (handle->dl_handle)
    {
      for (i = 0; i < handle->modules_count; i++)
        lt_dlclose(handle->dl_handle[i]);
      free(handle->dl_handle);
      handle->dl_handle = NULL;
    }
  if (handle->module_info)
    {
      free(handle->module_info);
      handle->module_info = NULL;
    }

  handle->magic = ~MONITOR_MODULE_MAGIC_NUMBER;
  handle->modules_max = 0;
  handle->modules_count = 0;

  free(handle);

  module_cleanup();
  return 0;
  
}

int 
monitor_modules_count(monitor_modules_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return handle->modules_count;
}

char *
monitor_module_name(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return NULL;

  return (handle->module_info[index])->monitor_module_name;
}

int 
monitor_module_setup(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->setup)());
}

int 
monitor_module_cleanup(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->cleanup)());
}

char *
monitor_module_metric_name(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return NULL;

  return ((*(handle->module_info[index])->metric_name)());
}

int 
monitor_module_metric_update(monitor_modules_t handle,
			     unsigned int index,
			     const char *nodename,
			     unsigned int metric_value_type,
			     unsigned int metric_value_len,
			     void *metric_value)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->metric_update)(nodename,
                                                         metric_value_type,
                                                         metric_value_len,
                                                         metric_value));
}
