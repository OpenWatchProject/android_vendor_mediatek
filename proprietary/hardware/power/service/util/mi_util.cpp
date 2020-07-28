/**
 * @file    mi_util.c
 * @brief   MI utilities, include double linked list, hash table and piority
 *          queue
 *
 * @version $Revision$
 * @date    $Date$
 * @author  tehsin.lin
 *
 * @par Fixes:
 *   None.
 * @par To Fix:
 * @par Reference:
 *****************************************************************************
 *  <b>CONFIDENTIAL</b><br>
 *****************************************************************************/

/** @addtogroup MI
 *  @addtogroup MI_Util Utilities
 *  @ingroup MI
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/
#include "mi_types.h"
#include "mi_util.h"


/*** MACROS ******************************************************************/
#define DLNODE_NIL ((tMI_DLNODE *)0)
#define HNODE_NIL ((tMI_HNODE *)0)
#define PQNODE_NIL ((tMI_PQNODE *)0)


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   This function is used for initializing the Double-linked list header.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_DlInit(tMI_DLIST *pList)
{
   if (pList)
   {
      pList->node.next = DLNODE_NIL;
      pList->node.prev = DLNODE_NIL;
      pList->count = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Append a node at the tail of the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pNode
///   The pointer of the node which is being inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlPushTail(tMI_DLIST *pList, tMI_DLNODE *pNode)
{
   if (pList && pNode)
   {
      if (pList->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      if (pList->node.next != DLNODE_NIL)
      {
         pNode->next = DLNODE_NIL;
         pNode->prev = pList->node.prev;
         pList->node.prev->next = pNode;
         pList->node.prev = pNode;
         pList->count++;
      }
      else
      {
         /** first element of the list **/
         pList->node.next = pNode;
         pList->node.prev = pNode;
         pNode->next = DLNODE_NIL;
         pNode->prev = DLNODE_NIL;
         pList->count = 1;
      }

      return pList->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Insert a node at the start of the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pNode
///   The pointer of the node which is inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlPushHead(tMI_DLIST *pList, tMI_DLNODE *pNode)
{
   if (pList && pNode)
   {
      if (pList->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      /** Add to the head of the list **/
      if(pList->node.next != DLNODE_NIL)
      {
         pNode->prev = DLNODE_NIL;
         pNode->next = pList->node.next;
         pList->node.next->prev = pNode;
         pList->node.next = pNode;
         pList->count++;
      }
      else
      {
         /** the first element of the list **/
         pList->node.next = pNode;
         pList->node.prev = pNode;
         pNode->next = DLNODE_NIL;
         pNode->prev = DLNODE_NIL;
         pList->count = 1;
      }

      return pList->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Total number of nodes are storing in the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   0 ~ 0xfffffffe: Total number of nodes.<br>
///   0xffffffff:     Error.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlCount(const tMI_DLIST *pList)
{
   if (pList) return (pList->count);
   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Delete a node which is storing in the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pNode
///   The pointer of the node which is being removed.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_DlDelete(tMI_DLIST *pList, tMI_DLNODE *pNode)
{
   if (pList && pNode)
   {
      if (pList->count == 0)
      {
         return;
      }

      if (pNode->next == DLNODE_NIL)
      {
         /** Last in List **/
         pList->node.prev = pNode->prev;
      }
      else
      {
         pNode->next->prev = pNode->prev;
      }

      if (pNode->prev == DLNODE_NIL)
      {
         /** First in List **/
         pList->node.next = pNode->next;
      }
      else
      {
         pNode->prev->next = pNode->next;
      }

      pList->count--;

      pNode->prev = DLNODE_NIL;
      pNode->next = DLNODE_NIL;
   }
}


///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the first node in the Double-linked list.<br>
///   The node is not removed from the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   pNode: the first node.<br>
///   NULL:  pList is null.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlFirst(const tMI_DLIST *pList)
{
   if (pList) return (pList->node.next);
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the last node in the Double-linked list.<br>
///   The node is not removed from the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   pNode:         the last node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlLast(const tMI_DLIST *pList)
{
   if (pList) return (pList->node.prev);
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Insert a node after a specific node of the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pPrev
///   The pointer of the node which is already in the Double-linked list.<br>
///
/// @param pNode
///   The pointer of the node which is being inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlInsertAfter(tMI_DLIST *pList, tMI_DLNODE *pPrev, tMI_DLNODE *pNode)
{
   if (pList && pNode)
   {
      if (pList->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      if(pPrev == DLNODE_NIL)
      {
         return MI_DlPushHead(pList, pNode);
      }
      else if ((pList->count == 0) || (pPrev->next == 0))
      {
         return MI_DlPushTail(pList, pNode);
      }

      pNode->next = pPrev->next;
      pNode->prev = pPrev;
      pPrev->next->prev = pNode;
      pPrev->next = pNode;

      pList->count++;

      return pList->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Insert a node in front of a specific node of the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pNext
///   The pointer of the node which is already in the Double-linked list.<br>
///
/// @param pNode
///   The pointer of the node which is being inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlInsertBefore(tMI_DLIST *pList, tMI_DLNODE *pNext, tMI_DLNODE *pNode)
{
   if (pList && pNode)
   {
      if (pList->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      if(pNext == DLNODE_NIL)
      {
         return MI_DlPushTail(pList, pNode);
      }
      else if ((pList->count == 0) || (pNext->prev == 0))
      {
         return MI_DlPushHead(pList, pNode);
      }

      pNode->prev = pNext->prev;
      pNode->next = pNext;
      pNext->prev->next = pNode;
      pNext->prev = pNode;

      pList->count++;

      return pList->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return a node after a specific node of the Double-linked list.<br>
///
/// @param pNode
///   The pointer of the node which is already in the Double-linked list.<br>
///
/// @return
///   Node:         a node after the pNode.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlNext(const tMI_DLNODE *pNode)
{
   if (pNode) return (pNode->next);
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return a node before a specific node of the Double-linked list.<br>
///
/// @param pNode
///   The pointer of the node which is already in the Double-linked list.<br>
///
/// @return
///   Node:         a node before the pNode.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlPrev(const tMI_DLNODE *pNode)
{
   if (pNode) return (pNode->prev);
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the first node in the Double-linked list.<br>
///   The node is also removed from the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   pNode:         the first node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlPopHead(tMI_DLIST *pList)
{
   tMI_DLNODE *vpN = NULL;

   if (pList)
   {
      vpN = pList->node.next;
      if (vpN)
      {
         MI_DlDelete(pList, vpN);
      }
   }
   return (vpN);
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the last node in the Double-linked list.<br>
///   The node is also removed from the Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @return
///   pNode:         the last node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlPopTail(tMI_DLIST *pList)
{
   tMI_DLNODE *vpN = NULL;

   if (pList)
   {
      vpN = pList->node.prev;
      if (vpN)
      {
         MI_DlDelete(pList, vpN);
      }
   }
   return (vpN);
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the node before or after specific hop of the node.<br>
///
/// @param pNode
///   The pointer of the anchor node.<br>
///
/// @param nStep
///   If nStep < 0, hop to previous.<br>
///   If nStep = 0, return pNode.<br>
///   If nStep > 0, hop to next.<br>
///
/// @return
///   Node:         the node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlStep(tMI_DLNODE *pNode, S32 nStep)
{
   S32 i;
   tMI_DLNODE *vpN = pNode;

   if (vpN)
   {
      if (nStep < 0)
      {
         for (i = 0; i < (-nStep); i++)
         {
            if (vpN->prev)
            {
               vpN = vpN->prev;
            }
            else
            {
               return DLNODE_NIL;
            }
         }
      }
      else if (nStep > 0)
      {
         for (i = 0; i < nStep; i++)
         {
            if (vpN->next)
            {
               vpN = vpN->next;
            }
            else
            {
               return DLNODE_NIL;
            }
         }
      }
   }

   return (vpN);
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Find the node in the forward direction of the Double-linked list header.<br>
///
/// @param pList
///   The pointer of node of the Double-linked list header.<br>
///
/// @param nodenum
///   The total number which is needed to travel from the fisrt node.<br>
///
/// @return
///   Node:         the node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlNth(const tMI_DLIST *pList, U32 nodenum)
{
   tMI_DLNODE *vpN;

   if (pList)
   {
      vpN = MI_DlFirst(pList);

      /** Assuming that numbering starts from 0 **/
      return (MI_DlStep(vpN, nodenum));
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Get the position of the node of the Double-linked list header.<br>
///
/// @param pList
///   The pointer of node of the Double-linked list header.<br>
///
/// @param pNode
///   The node to be determined.<br>
///
/// @return
///   0 ~ 4294967294:         the position.<br>
///   4294967295:             the pNode is not in the Double-linked list.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlIndex(const tMI_DLIST *pList, tMI_DLNODE *pNode)
{
   tMI_DLNODE *vpN;
   U32 i = 0;

   if (pList && pNode)
   {
      vpN = MI_DlFirst(pList);
      for(;; i++, vpN = vpN->next)
      {
         if (vpN == pNode)
         {
            return i;
         }

         if (vpN == DLNODE_NIL)
         {
            return MI_UTIL_ERROR;
         }
      }
   }
   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Append a Double-linked list after the target list.<br>
///
/// @param pDstList
///   The pointer of the Double-linked list header which will contain new nodes.<br>
///
/// @param pAddList
///   The pointer of the Double-linked list header which will provide nodes.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlConcat(tMI_DLIST *pDstList, tMI_DLIST *pAddList)
{
   if (pDstList && pAddList)
   {
      tMI_DLNODE *last = MI_DlLast(pDstList);
      tMI_DLNODE *first = MI_DlFirst(pAddList);

      U32 vSC = MI_UTIL_MAXCOUNT - pDstList->count;

      if (!first)
      {
         return pDstList->count;
      }

      if (vSC < pAddList->count)
      {
         return MI_UTIL_ERROR;
      }

      if (!last)
      {
         pDstList->node.next = first;
      }
      else
      {
         last->next = first;
      }

      if(!first)
      {
         /* Nothing */
      }
      else
      {
         first->prev = last;
      }

      pDstList->node.prev = pAddList->node.prev;

      pDstList->count += pAddList->count;

      MI_DlInit(pAddList);

      return pDstList->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Travel all nodes of a Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param walker
///   The pointer of function which will execute on all nodes.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_DlWalk(const tMI_DLIST *pList,  void (*walker)(tMI_DLNODE *, void *), void *z)
{
   if (pList && walker)
   {
      tMI_DLNODE *vpN;

      for (vpN = MI_DlFirst(pList); vpN; vpN = MI_DlNext(vpN))
      {
         walker(vpN,z);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Insert a node into a sorted Double-linked list.<br>
///
/// @param L
///   The pointer of the Double-linked list header.<br>
///
/// @param N
///   The node to be inserted.<br>
///
/// @param keyof
///   The pointer of function which retrive the key from a node to compare
///   with the key of N .<br>
///
/// @param compare
///   The pointer of function which will compare the key of N and of nodes in
///   the Double-linked list.<br>
///   if the result of compare is < 0, the process will continue.<br>
///   if the result of compare is >= 0, the node will be inserted in front of vpN.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_DlInsert(tMI_DLIST *L, tMI_DLNODE *N, const void *(*keyof)(const tMI_DLNODE *), S32 (*compare)(const void *, const void *))
{
   U32 vStart = 0;
   U32 vEnd = 0;
   U32 vMid = 0;
   U32 vNewMid = 0;
   S32 vCompare = 0;

   tMI_DLNODE *vpN = 0;
   tMI_DLNODE *vpStartN = 0;

   if (!L || !N || !keyof || !compare)
   {
      return MI_UTIL_ERROR;
   }

   if (L->count >= MI_UTIL_MAXCOUNT)
   {
      return MI_UTIL_ERROR;
   }

   if (L->count == 0)
   {
      return MI_DlPushTail(L, N);
   }

   vpStartN = MI_DlFirst(L);

   vCompare = compare(keyof(N), keyof(vpStartN));

   if (vCompare >= 0)
   {
      return MI_DlInsertBefore(L, vpStartN, N);
   }
   else if (L->count == 1)
   {
      return MI_DlPushTail(L, N);
   }

   vpN = MI_DlLast(L);

   vCompare = compare(keyof(N), keyof(vpN));

   if (vCompare < 0)
   {
      return MI_DlPushTail(L, N);
   }

   if (L->count)
   {
      vEnd = L->count - 1;
   }

   vMid = (vStart + 1 + vEnd) / 2;
   vpN = MI_DlStep(vpStartN, vMid - vStart);

   while (vpN)
   {
      vCompare = compare(keyof(N), keyof(vpN));

      if (vCompare == 0)
      {
         MI_DlInsertBefore(L, vpN, N);
         break;
      }

      if (vCompare < 0)
      {
         if (vMid + 1 == vEnd)
         {
            MI_DlInsertAfter(L, vpN, N);
            break;
         }

         vNewMid = (vMid + 1 + vEnd) / 2;
         vStart = vMid;
         vpStartN = vpN;
      }
      else
      {
         if (vMid - 1 == vStart)
         {
            MI_DlInsertBefore(L, vpN, N);
            break;
         }

         vNewMid = (vStart + 1 + vMid) / 2;
         vEnd = vMid;
      }

      vMid = vNewMid;
      vpN = MI_DlStep(vpStartN, vMid - vStart);
   }

   return L->count;
}


///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Find a node of a Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param pValue
///   The key to find the specific node.<br>
///
/// @param keyof
///   The pointer of function which retrieve the key from a node to compare
///   with the pValue.<br>
///
/// @param compare
///   The pointer of function which will compare the pValue and of nodes in
///   the Double-linked list.<br>
///   if the result of compare is equal 0, the node will be returned.<br>
///
/// @return
///   Node: the node with the same key of pValue.<br>
///   0:    Not found or parameter error.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_DLNODE *MI_DlFind( const tMI_DLIST *pList, const void *pValue,
                       const void *(*keyof)(const tMI_DLNODE *),
                       S32 (*compare)(const void *, const void *) )
{
   tMI_DLNODE *vpN;

   if (pList && pValue && keyof && compare)
   {
      for(vpN = pList->node.next; vpN ; vpN = vpN->next )
      {
         if(!compare(pValue, keyof(vpN)))
         {
            return vpN;
         }
      }
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Sort the nodes of a Double-linked list.<br>
///
/// @param pList
///   The pointer of the Double-linked list header.<br>
///
/// @param keyof
///   The pointer of function which retrieve the key from a node to compare
///   with the pValue.<br>
///
/// @param compare
///   The pointer of function which will compare the key of nodes in
///   the Double-linked list.<br>
///   The increasingly or decreasingly order is control by compare.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_DlSort(tMI_DLIST *L, const void *(*keyof)(const tMI_DLNODE *), S32 (*compare)(const void *, const void *))
{
   if (L && keyof && compare)
   {
      tMI_DLNODE *p, *q, *e, *tail, *head = L->node.next;
      U32 insize = 1, nmerges, psize, qsize, i;

      if (!head)
      {
         return;
      }

      for(;;)
      {
         p = head;
         head = DLNODE_NIL;
         tail = DLNODE_NIL;
         nmerges = 0;

         while (p)
         {
            nmerges++;
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++)
            {
               psize++;
               q = q->next;
               if (!q)
               {
                  break;
               }
            }

            qsize = insize;

            while ( (psize > 0) || ( (qsize > 0) && q))
            {
               if (psize == 0)
               {
                  e = q; q = q->next; qsize--;
               }
               else if (qsize == 0 || !q)
               {
                  e = p; p = p->next; psize--;
               }
               else if (compare(keyof(p), keyof(q)) <= 0)
               {
                  e = p; p = p->next; psize--;
               }
               else
               {
                  e = q; q = q->next; qsize--;
               }

               if (tail)
               {
                  tail->next = e;
               }
               else
               {
                  head = e;
               }

               e->prev = tail;
               tail = e;
            }

            p = q;
         }

         if (tail) tail->next = DLNODE_NIL;

         if (nmerges <= 1)
         {
             L->node.next = head;
             L->node.prev = tail;
             return;
         }

         insize *= 2;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   This function is used for initializing the Hash table header.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @param nbuckets
///   The total number of buckets to store the nodes.<br>
///
/// @param hash
///   The pointer of function which generate the hash code.<br>
///
/// @param compare
///   The pointer of function which will compare the key of nodes in
///   the Double-linked list.<br>
///   The increasingly or decreasingly order is control by compare.<br>
///
/// @param keyof
///   The pointer of function which retrieve the key from a node to compare
///   with the target key.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_HInit( tMI_HASH *pH,
                 U32 nbuckets,
                 U32 (*hash)(const void *),
                 S32 (*compare)(const void *, const void *),
                 const void *(*keyof)(const tMI_HNODE *),
                 tMI_HNODE **table )
{
   U32 i;

   if (pH && hash && compare && keyof && table)
   {
      pH->count = 0;
      pH->nbuckets = nbuckets;
      pH->hash = hash;
      pH->keyof = keyof;
      pH->compare = compare;
      pH->table = table;

      for (i = 0; i < nbuckets; i++)
      {
         table[i] = 0;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   This function is used for inserting a node into the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @param pNode
///   The pointer of the node to be inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_HInsert(tMI_HASH *pH, tMI_HNODE *pNode)
{
   U32 index;

   if (pH && pNode)
   {
      if (pH->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      index = (pH->hash(pH->keyof(pNode))) % (pH->nbuckets);
      pNode->link = pH->table[index];
      pH->table[index] = pNode;
      pH->count++;

      return pH->count;
   }

   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   This function is used for removing a node from the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @param pNode
///   The pointer of the node to be removed.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_HDelete(tMI_HASH *pH, tMI_HNODE *pNode)
{
   if (pH && pNode)
   {
      U32 index;
      tMI_HNODE *vpN;

      index = (pH->hash(pH->keyof(pNode))) % (pH->nbuckets);
      vpN = pH->table[index];

      if (vpN == pNode)
      {
         pH->table[index] = pNode->link;
         pNode->link = 0;
         pH->count--;
      }
      else
      {
         while (vpN && (vpN->link != pNode))
         {
            vpN = vpN->link;
         }

         if (vpN == 0)
         {
            return;
         }

         vpN->link = pNode->link;
         pNode->link = 0;
         pH->count--;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the total number of nodes of the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @return
///   0 ~ 0xfffffffe: total number of nodes.<br>
///   0xffffffff:     pH is null.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_HCount(const tMI_HASH *pH)
{
   if (pH) return pH->count;
   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Find a specific node of the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @param item
///   The pointer of the key.<br>
///
/// @return
///   Node:         the node with the same key of item.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_HNODE *MI_HFind(const tMI_HASH *pH, const void *item)
{
   U32 index;
   tMI_HNODE *vpN = NULL;

   if (pH && item)
   {
      index = (pH->hash(item)) % (pH->nbuckets);
      vpN = pH->table[index];

      while (vpN && pH->compare(item, pH->keyof(vpN)))
      {
         vpN = vpN->link;
      }
   }

   return vpN;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the first node of the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @return
///   Node:         the first node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_HNODE *MI_HFirst(const tMI_HASH *pH)
{
   U32 index;

   if (pH)
   {
      for (index = 0; index < pH->nbuckets; index++)
      {
         if (pH->table[index])
         {
            return pH->table[index];
         }
      }
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the node after a specific node of the Hash table.<br>
///
/// @param pH
///   The pointer of the Hash table header.<br>
///
/// @param pNode
///   The pointer of the specific node.<br>
///
/// @return
///   Node:         the node after the pNode.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_HNODE *MI_HNext(const tMI_HASH *pH, const tMI_HNODE *pNode)
{
   if (pH && pNode)
   {
      U32 index;

      if (pNode->link)
      {
         return pNode->link;
      }

      index = pH->hash(pH->keyof(pNode)) % (pH->nbuckets);
      index++;

      for(; index < pH->nbuckets; index++)
      {
         if (pH->table[index])
         {
            return pH->table[index];
         }
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   This function is used for initializing the Priority Queue header.<br>
///
/// @param pH
///   The pointer of the Priority Queue header.<br>
///
/// @param priority
///   The pointer of the function which can determine the priroiry of nodes.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
void MI_PQInit(tMI_PQUEUE *pQ, S32 (*priority)(const tMI_PQNODE *))
{
   if (pQ && priority)
   {
      pQ->count = 0;
      pQ->head = pQ->tail = 0;
      pQ->priority = priority;
   }
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the total number of nodes of the Priority Queue.<br>
///
/// @param pH
///   The pointer of the Priority Queue header.<br>
///
/// @return
///   0 ~ 0xfffffffe: total number of nodes.<br>
///   0xffffffff:     pQ is null.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_PQCount(const tMI_PQUEUE *pQ)
{
   if (pQ) return pQ->count;
   return MI_UTIL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Return the first node in the Priority Queue.<br>
///   The node is also removed from the Priority Queue.<br>
///
/// @param pQ
///   The pointer of the Priority Queue header.<br>
///
/// @return
///   pNode:         the first node.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
tMI_PQNODE *MI_PQPopHead(tMI_PQUEUE *pQ)
{
   tMI_PQNODE *vpN = NULL;

   if (pQ)
   {
      vpN = pQ->head;
      if (vpN == PQNODE_NIL)
      {
         return 0;
      }

      if (vpN->h.next == 0)
      {
         pQ->head = vpN->v.next;

         if (vpN->v.next == 0)
         {
            pQ->tail = 0;
         }
         else
         {
            vpN->v.next->v.previous = vpN->v.previous;
         }
      }
      else
      {
         pQ->head = vpN->h.next;
         vpN->h.next->v.previous = 0;
         vpN->h.next->v.next = vpN->v.next;

         if (vpN->h.previous == vpN->h.next )
         {
            vpN->h.next->h.previous = 0;
         }
         else
         {
            vpN->h.next->h.previous = vpN->h.previous;
         }

         if (vpN->v.next == 0)
         {
            pQ->tail = vpN->h.next;
         }
         else
         {
            vpN->v.next->v.previous = vpN->h.next;
         }
      }

      pQ->count--;
   }
   return vpN;
}

///////////////////////////////////////////////////////////////////////////////
/// @par Declaration
///   Append a node at the tail of the Priority Queue.<br>
///
/// @param pQ
///   The pointer of the Priority Queue header.<br>
///
/// @param pNode
///   The pointer of the node which is being inserted.<br>
///
/// @return
///   None:         return with void value.<br>
///
/// @par Conditions
///   None.<br>
///
/// @par Usage
///   None.<br>
///
///////////////////////////////////////////////////////////////////////////////
U32 MI_PQPushTail(tMI_PQUEUE *pQ, tMI_PQNODE *pNode)
{
   if (pQ && pNode)
   {
      if (pQ->count >= MI_UTIL_MAXCOUNT)
      {
         return MI_UTIL_ERROR;
      }

      if (pQ->head == 0)
      {
         pQ->head = pQ->tail = pNode;
         pNode->v.previous = pNode->h.previous = pNode->v.next = pNode->h.next = 0;

         pQ->count++;

         return pQ->count;
      }
      else
      {
         tMI_PQNODE *scan = pQ->head;
         S32 priority = pQ->priority(pNode);

         while (scan)
         {
            S32 thisprio = pQ->priority(scan);

            if (priority == thisprio)
            {
               pNode->h.next = 0;
               if (scan->h.previous)
               {
                  pNode->h.previous = scan->h.previous;
                  scan->h.previous->h.next = pNode;
               }
               else
               {
                  scan->h.next = pNode;
                  pNode->h.previous = scan;
               }

               scan->h.previous = pNode;

               pQ->count++;

               return pQ->count;
            }
            else if (priority < thisprio)
            {
               pNode->h.previous = pNode->h.next = 0;

               if (scan->v.previous == 0)
               {
                  pQ->head = pNode;
                  pNode->v.previous = 0;
               }
               else
               {
                  scan->v.previous->v.next = pNode;
                  pNode->v.previous = scan->v.previous;
               }

               pNode->v.next = scan; scan->v.previous = pNode;
               pQ->count++;

               return pQ->count;
            }
            else
            {
               scan = scan->v.next;
            }
         }

         pNode->h.previous = pNode->h.next = 0;
         pQ->tail->v.next = pNode;

         pNode->v.previous = pQ->tail;
         pNode->v.next = 0;
         pQ->tail = pNode;

         pQ->count++;

         return pQ->count;
      }
   }

   return MI_UTIL_ERROR;
}

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

/**  @} */
/******************************************************************************
 *  CONFIDENTIAL
 *****************************************************************************/
