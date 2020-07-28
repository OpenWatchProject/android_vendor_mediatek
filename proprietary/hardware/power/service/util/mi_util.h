/**
 * @file    mi_util.h
 * @brief   mi utilities include file
 *
 * @version $Revision$
 * @date    $Date$
 * @author  tehsin.lin
 *
 * @par Fixes:
 *    None.
 * @par To Fix:
 *    None.
 * @par Reference:
 *    None.
 *****************************************************************************
 *  <b>CONFIDENTIAL</b><br>
 *****************************************************************************/

#ifndef __MI_UTIL_H__
#define __MI_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/


/*** MACROS ******************************************************************/
#define MI_NODEENTRY(ptr, type, member) \
    ((type *)((U8 *)(ptr)-(U8 *)(&((type *)0)->member)))

#define MI_DLEMPTY(L) ((L)->count == 0)
#define MI_HEMPTY(L) ((L)->count == 0)
#define MI_PQEMPTY(L) ((L)->count == 0)

#define MI_UTIL_MAXCOUNT 0xFFFFFFFE
#define MI_UTIL_ERROR 0xFFFFFFFF


/*** GLOBAL TYPES DEFINITIONS ************************************************/
typedef struct tMI_DLNODE
{
   struct tMI_DLNODE *next;
   struct tMI_DLNODE *prev;
} tMI_DLNODE;

typedef struct tMI_DLIST
{
   tMI_DLNODE node;
   U32 count;
} tMI_DLIST;

typedef struct tMI_HNODE
{
   struct tMI_HNODE *link;
} tMI_HNODE;

typedef struct tMI_HASH
{
   U32 (*hash)(const void *);
   S32 (*compare)(const void *, const void *);
   const void *(*keyof)(const tMI_HNODE *);
   tMI_HNODE **table;
   U32 nbuckets;
   U32 count;
} tMI_HASH;

typedef struct tMI_PQNODE
{
   struct
   {
      struct tMI_PQNODE *next;
      struct tMI_PQNODE *previous;
   } v, h;
} tMI_PQNODE;

typedef struct tMI_PQUEUE
{
   tMI_PQNODE *head;
   tMI_PQNODE *tail;
   U32 count;
   S32 (*priority)(const tMI_PQNODE *);
} tMI_PQUEUE;
/******/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PUBLIC FUNCTION PROTOTYPES **********************************************/


/** Doubly Linked List ******************************************************/
void MI_DlInit(tMI_DLIST *pList);

U32 MI_DlCount(const tMI_DLIST *pList);
tMI_DLNODE *MI_DlFirst(const tMI_DLIST *pList);
tMI_DLNODE *MI_DlLast(const tMI_DLIST *pList);
tMI_DLNODE *MI_DlNext(const tMI_DLNODE *pNode);
tMI_DLNODE *MI_DlPrev(const tMI_DLNODE *pNode);

U32 MI_DlInsertAfter(tMI_DLIST *pList, tMI_DLNODE *pPrev, tMI_DLNODE *pNode);
U32 MI_DlInsertBefore(tMI_DLIST *pList, tMI_DLNODE *pNext, tMI_DLNODE *pNode);
void MI_DlDelete(tMI_DLIST *pList, tMI_DLNODE *pNode);

U32 MI_DlPushHead(tMI_DLIST *pList, tMI_DLNODE *pNode);
U32 MI_DlPushTail(tMI_DLIST *pList, tMI_DLNODE *pNode);
tMI_DLNODE *MI_DlPopHead(tMI_DLIST *pList);
tMI_DLNODE *MI_DlPopTail(tMI_DLIST *pList);

U32 MI_DlInsert(tMI_DLIST *, tMI_DLNODE *, const void *(*)(const tMI_DLNODE *), S32 (*)(const void *, const void *));
void MI_DlWalk(const tMI_DLIST *, void (*)(tMI_DLNODE *, void *), void *);
tMI_DLNODE *MI_DlFind(const tMI_DLIST *, const void *, const void *(*)(const tMI_DLNODE *), S32 (*)(const void *, const void *));
void MI_DlSort(tMI_DLIST *, const void *(*)(const tMI_DLNODE *), S32 (*)(const void *, const void *));

U32 MI_DlConcat(tMI_DLIST *pDstList, tMI_DLIST *pAddList);

tMI_DLNODE *MI_DlNth(const tMI_DLIST *pList, U32 nodenum);
tMI_DLNODE *MI_DlStep(tMI_DLNODE *pNode, S32 nStep);
U32 MI_DlIndex(const tMI_DLIST *pList, tMI_DLNODE *pNode);

/** Hash Tables *************************************************************/
void MI_HInit( tMI_HASH *, U32, U32 (*)(const void *),
               S32 (*)(const void *, const void *), const void *(*)(const tMI_HNODE *), tMI_HNODE **);
U32 MI_HInsert(tMI_HASH *, tMI_HNODE *);
void MI_HDelete(tMI_HASH *, tMI_HNODE *);
U32 MI_HCount(const tMI_HASH *);

tMI_HNODE *MI_HFind(const tMI_HASH *, const void *);
tMI_HNODE *MI_HFirst( const tMI_HASH *H );
tMI_HNODE *MI_HNext( const tMI_HASH *H, const tMI_HNODE *N );

/** Priority Queues *********************************************************/
void MI_PQInit(tMI_PQUEUE *, S32 (*)(const tMI_PQNODE *));
U32 MI_PQCount(const tMI_PQUEUE *);
tMI_PQNODE *MI_PQPopHead(tMI_PQUEUE *);
U32 MI_PQPushTail(tMI_PQUEUE *, tMI_PQNODE *);
/******/


/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __MI_UTIL_H__ */

/******************************************************************************
 *  CONFIDENTIAL
 *****************************************************************************/
