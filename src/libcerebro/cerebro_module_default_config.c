/*****************************************************************************\
 *  $Id: cerebro_module_default_config.c,v 1.1 2005-06-17 16:59:30 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro/cerebro_config_module.h"

#define DEFAULT_CONFIG_MODULE_NAME "default"

int
default_config_setup(void)
{
  return 0;
}

int
default_config_cleanup(void)
{
  return 0;
}

int
default_config_load_default(struct cerebro_config *conf)
{
  return 0;
}

struct cerebro_config_module_info default_config_module_info =
  {
    DEFAULT_CONFIG_MODULE_NAME,
    &default_config_setup,
    &default_config_cleanup,
    &default_config_load_default,
  };
