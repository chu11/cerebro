/*****************************************************************************\
 *  $Id: cerebrod_data.h,v 1.1 2005-03-20 20:10:14 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CACHE_H
#define _CEREBROD_CACHE_H

/* 
 * cerebrod_load_data
 *
 * Load and cache cerebrod starttme, machine boottime, and machine
 * hostname
 */
void cerebrod_load_data(void);

/* 
 * cerebrod_get_starttime
 *
 * Return the cached cerebrod starttime
 */
u_int32_t cerebrod_get_starttime(void);

/* 
 * cerebrod_get_boottime
 *
 * Return the cached system boottime
 */
u_int32_t cerebrod_get_boottime(void);


/* 
 * cerebrod_get_hostname
 *
 * Return the cached system hostname
 */
void cerebrod_get_hostname(char *buf, unsigned int len);

#endif /* _CEREBROD_CACHE_H */