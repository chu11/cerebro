/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_hostsfile.c,v 1.5 2005-03-17 05:05:52 achu Exp $
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
#include <assert.h>
#include <errno.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

List hosts = NULL;
char *hostsfile_file = NULL;

static void
_parse_options(char **options)
{
  int i = 0;

  assert(options);

  while (options[i] != NULL)
    {
      if (strstr(options[i], "filename"))
	{
	  char *p = strchr(options[i], '=');

	  if (!p)
	    err_exit("hostsfile clusterlist module: filename unspecified");

	  p++;
	  if (p == '\0')
	    err_exit("hostsfile clusterlist module: filename unspecified");

	  hostsfile_file = Strdup(p);
	}
      else
	err_exit("hostsfile clusterlist module: option '%s' unrecognized", options[i]);

      i++;
    }
}

static int
_readline(int fd, char *buf, int buflen)
{
  int ret;

  assert(buf);

  if ((ret = fd_read_line(fd, buf, buflen)) < 0)
    err_exit("_readline: fd_read_line: %s", strerror(errno));

  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (ret >= (buflen-1))
    err_exit("_readline: fd_read_line: line truncation");

  return ret;
}

static int
_remove_comments(char *buf, int buflen)
{
  int i, comment_flag, retlen;

  assert(buf);

  if (strchr(buf, '#') == NULL)
    return buflen;

  i = 0;
  comment_flag = 0;
  retlen = buflen;
  while (i < buflen)
    {
      if (comment_flag)
        {
          buf[i] = '\0';
          retlen--;
        }

      if (buf[i] == '#')
        {
          buf[i] = '\0';
          comment_flag++;
          retlen--;
        }
    }

  return retlen;
}

static int
_remove_trailing_whitespace(char *buf, int buflen)
{
  char *temp;
  
  assert(buf);

  temp = buf + buflen;
  for (--temp; temp >= buf; temp--) 
    {
      if (isspace(*temp))
        *temp = '\0';
      else
        break;
      buflen--;
    }

  return buflen;
}

static char *
_move_past_whitespace(char *buf)
{
  assert(buf);

  while (*buf != '\0' && isspace(*buf))
    buf++;

  if (*buf == '\0')
    return NULL;

  return buf;
}

int 
hostsfile_clusterlist_init(char **options)
{
  int fd, len;
  char buf[CEREBROD_PARSE_BUFLEN];
  char *file;

  assert(!hosts);

  if (options)
    _parse_options(options);

  hosts = List_create((ListDelF)_Free);

  if (hostsfile_file)
    file = hostsfile_file;
  else
    file = CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT;

  if ((fd = open(file, O_RDONLY)) < 0)
    err_exit("hostsfile clusterlist file '%s' cannot be opened", hostsfile_file);

  while ((len = _readline(fd, buf, CEREBROD_PARSE_BUFLEN)) > 0)
    {
      char *hostPtr;
      char *str;

      if ((len = _remove_comments(buf, len)) == 0)
        continue;

      if ((len = _remove_trailing_whitespace(buf, len)) == 0)
        continue;

      if ((hostPtr = _move_past_whitespace(buf)) == NULL)
        continue;

      if (hostPtr[0] == '\0')
        continue;

      if (strchr(hostPtr, ' ') || strchr(hostPtr, '\t'))
        err_exit("hostsfile clusterlist parse error: host contains whitespace");

      if (strlen(hostPtr) > MAXHOSTNAMELEN)
        err_exit("hostsfile clusterlist parse error: hostname '%s' exceeds "
                 "maximum length", hostPtr);
      
      str = Strdup(hostPtr);
      List_append(hosts, str);
    }
  
  close(fd);
  return 0;
}

int
hostsfile_clusterlist_finish(void)
{
  assert(hosts);

  List_destroy(hosts);
  Free(hostsfile_file);
  hosts = NULL;
  hostsfile_file = NULL;

  return 0;
}

int
hostsfile_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  char *node;
  ListIterator itr;
  int numnodes, i = 0;

  assert(hosts);
  assert(nodes);

  numnodes = list_count(hosts);

  if (numnodes > nodeslen)
    err_exit("hostsfile_clusterlist_get_all_nodes: nodeslen too smalll");

  itr = List_iterator_create(hosts);

  while ((node = list_next(itr)) && (i < numnodes))
    nodes[i++] = Strdup(node);

  if (i > numnodes)
    err_exit("hostsfile_clusterlist_get_all_nodes: iterator count error");

  List_iterator_destroy(itr);

  return numnodes;
}

int 
hostsfile_clusterlist_numnodes(void)
{
  assert(hosts);

  return list_count(hosts);
}

int
hostsfile_clusterlist_node_in_cluster(char *node)
{
  void *ret;

  assert(hosts);
  assert(node);

  ret = list_find_first(hosts, (ListFindF)strcmp, node);

  return ((ret) ? 1: 0);
}

int
hostsfile_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len;

  assert(hosts);
  assert(node);
  assert(buf);
  assert(buflen > 0);

  len = strlen(node);

  if ((len + 1) > buflen)
    err_exit("hostsfile_clusterlist_get_nodename: buflen too small: %d %d",
	     len, buflen);

  strcpy(buf, node);

  return 0;
}

struct cerebrod_clusterlist_ops clusterlist_ops =
  {
    &hostsfile_clusterlist_init,
    &hostsfile_clusterlist_finish,
    &hostsfile_clusterlist_get_all_nodes,
    &hostsfile_clusterlist_numnodes,
    &hostsfile_clusterlist_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
  };


