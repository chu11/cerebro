/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders.c,v 1.10 2005-03-18 19:04:25 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include <genders.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_clusterlist_util.h"
#include "error.h"
#include "wrappers.h"

genders_t handle = NULL;
char *genders_file = NULL;

int
genders_clusterlist_parse_options(char **options)
{
  assert(!handle);

  if (options)
    cerebrod_clusterlist_parse_filename(options, &genders_file);
}

int 
genders_clusterlist_init(void)
{
  assert(!handle);

  return cerebrod_clusterlist_genders_init(&handle, genders_file);
}

int
genders_clusterlist_finish(void)
{
  assert(handle);

  return cerebrod_clusterlist_genders_finish(&handle, &genders_file);
}

int
genders_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(handle);
  assert(nodes);
  
  return cerebrod_clusterlist_genders_get_all_nodes(handle, nodes, nodeslen);
}

int 
genders_clusterlist_numnodes(void)
{
  assert(handle);

  return cerebrod_clusterlist_genders_numnodes(handle);
}

int
genders_clusterlist_node_in_cluster(char *node)
{
  int ret, free_flag = 0;
  char *nodePtr = NULL;

  assert(handle);
  assert(node);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
      nodePtr = Strdup(node);
      p = strchr(nodePtr, '.');
      *p = '\0';
      free_flag++;
    }
  else
    nodePtr = node;

  if ((ret = genders_isnode(handle, nodePtr)) < 0)
    err_exit("genders_clusterlist_node_in_cluster: genders_isnode: %s",
	     genders_errormsg(handle));

  if (free_flag)
    Free(nodePtr);

  return ret;
}

int
genders_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  assert(handle);
  assert(node);
  assert(buf);
  assert(buflen > 0);

  return cerebrod_clusterlist_copy_nodename(node, buf, buflen);
}

struct cerebrod_clusterlist_module_info clusterlist_module_info =
  {
    "genders",
  };

struct cerebrod_clusterlist_module_ops clusterlist_module_ops =
  {
    &genders_clusterlist_parse_options,
    &genders_clusterlist_init,
    &genders_clusterlist_finish,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
