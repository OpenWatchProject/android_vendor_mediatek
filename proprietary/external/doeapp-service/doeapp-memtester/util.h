/*
 * vim:ts=4:sw=4:expandtab
 *
 * memtester - customized memtester-4.3.0
 *
 * util.c: Utilities for memtester
 *
 */

#if !defined(__UTIL_H)
#define __UTIL_H

extern ull virt2phys(ull vaddr);

extern int find_byteinvert(ul v1, ul v2);

extern int find_bitflips(ul v1, ul v2);

#endif
