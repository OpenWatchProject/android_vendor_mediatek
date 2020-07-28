#include <stdlib.h>
#include <string.h>

#include "os/api_log.h"
#include "os/api_channel.h"
#include "os/api_mem.h"
#include "os/api_task.h"
#include "common/api_string.h"
#include "interface/volte_event.h"
#include "interface/volte_cap.h"

#undef DEBUG_MSG_TAG
#define DEBUG_MSG_TAG       "DISP"

#ifdef __ANDROID__
  #define ENTER(f, a...)
  #define EXIT(f, a...)
#else
  #define ENTER(f, a...)      //printf("%u, > [Disp] %s, " f "\n", __LINE__, __func__, ##a)
  #define EXIT(f, a...)       //printf("%u, < [Disp] %s, " f "\n", __LINE__, __func__, ##a)
#endif

// ---------------------
// Matching Rule
// ---------------------
#define DPadBase    (4)

//#define DMalloc(s)      malloc(s)
//#define DFree(p)        free(p)
#define DMalloc(s)      sip_get_mem(s)
#define DFree(p)        while (p) { sip_free_mem(p); break; }

#define DDBG(f, a...)
#define DMSG(f, a...)
#define DERR(f, a...)

#define DGetLenWithPad(sl, l) \
    l=(((sl)/DPadBase)+((sl)%DPadBase?1:0))*DPadBase


#define DCheckPointerAndReturn(p, rt) \
    if (!(p)) { \
        ErrMsg("invalid pointer"); \
        EXIT(); \
        return (rt); \
    }


// -------------------------
// Level 2 generic function
// -------------------------
int Rule_Append_RuleSet_with_Single_Header_Param(struct rule_level_2 *lv2, const int method, const int header, char *param);
int Rule_Append_RuleSet_with_Multi_Header_Param(struct rule_level_2 *lv2, const int method, RuleCond_Context_t multi[], const int num);

int Rule_Replace_RuleSet_with_Single_Header_Param(struct rule_level_2 *lv2, const int method, const int header, char *param);
int Rule_Replace_RuleSet_with_Multi_Header_Param(struct rule_level_2 *lv2, const int method, RuleCond_Context_t multi[], const int num);




int Rule_Append_RuleSet_with_Single_Header_Param(struct rule_level_2 *lv2, const int method, const int header, char *param)
{
    struct rule_set  *set  = 0;
    struct rule_item *item = 0;
    struct rule_cond *cond = 0;
    int append = 0;


    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    RuleCond_Create(header, param, &cond);

    RuleItem_Create(&item);
    RuleItem_SetRuleCond(item, cond);
    append = RuleItem_GetSize(item);
    DDBG("Rule_Append, item(%p, size:%d)", item, append);

    set = Level2_GetRuleSetbyMethod(lv2, method);
    if (set) {
        RuleSet_SetRuleItem(set, item);
        DDBG("Rule_Append, re-calc lv2_size(%d) with append_item_size(%d)", lv2->len_of_level_2, append);
        lv2->len_of_level_2 += append;
    } else {
        RuleSet_Create(method, &set);
        RuleSet_SetRuleItem(set, item);
        Level2_SetRuleSet(lv2, set);
    }

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Append_RuleSet_with_Multi_Header_Param(struct rule_level_2 *lv2, const int method, RuleCond_Context_t multi[], const int num)
{
    int i = 0, append = 0;
    struct rule_set  *set  = 0;
    struct rule_item *item = 0;
    struct rule_cond *cond = 0;


    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    RuleItem_Create(&item);
    for (; i < num; i++) {
        RuleCond_Create(multi[i].header, multi[i].param, &cond);
        RuleItem_SetRuleCond(item, cond);
    }
    append = RuleItem_GetSize(item);
    DDBG("Rule_Append, item(%p, size:%d)", item, append);

    set = Level2_GetRuleSetbyMethod(lv2, method);
    if (set) {
        RuleSet_SetRuleItem(set, item);
        DDBG("Rule_Append, re-calc lv2_size(%d) with append_item_size(%d)", lv2->len_of_level_2, append);
        lv2->len_of_level_2 += append;
    } else {
        RuleSet_Create(method, &set);
        RuleSet_SetRuleItem(set, item);
        Level2_SetRuleSet(lv2, set);
    }

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Replace_RuleSet_with_Single_Header_Param(struct rule_level_2 *lv2, const int method, const int header, char *param)
{
    struct rule_set  *set  = 0;
    struct rule_item *item = 0;
    struct rule_cond *cond = 0;


    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    RuleSet_Create(method, &set);
    RuleItem_Create(&item);
    RuleCond_Create(header, param, &cond);

    RuleItem_SetRuleCond(item, cond);
    RuleSet_SetRuleItem(set, item);
    Level2_SetRuleSet(lv2, set);

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Replace_RuleSet_with_Multi_Header_Param(struct rule_level_2 *lv2, const int method, RuleCond_Context_t multi[], const int num)
{
    int i = 0;
    struct rule_set  *set  = 0;
    struct rule_item *item = 0;
    struct rule_cond *cond = 0;


    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    RuleSet_Create(method, &set);
    RuleItem_Create(&item);
    for (; i < num; i++) {
        RuleCond_Create(multi[i].header, multi[i].param, &cond);
        RuleItem_SetRuleCond(item, cond);
    }

    RuleSet_SetRuleItem(set, item);
    Level2_SetRuleSet(lv2, set);

    return VoLTE_STACK_RULE_RSP_OK;
}


// -----------------
// level 2
// -----------------
int Level2_Create(struct rule_level_2 **lv2)
{
    struct rule_level_2 *p = 0;

    ENTER();

    p = DMalloc(sizeof(struct rule_level_2));
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE);

    *lv2 = p;

    DDBG("created lv2(%p)", *lv2);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int Level2_Delete(struct rule_level_2 *lv2)
{
    struct rule_set *cur = 0, *next = 0;
    int d = 0, num = 0;


    ENTER("lv2:%p", lv2);

    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cur = lv2->set_p;
    num = lv2->num_of_set;

    while (cur) {
        next = cur->next_p;

        RuleSet_Delete(cur);
        d++;
        cur = next;
    }

    DDBG("deleted lv2, lv2(%p, num_of_set:%d), del_set:%d", lv2, num, d);
    DFree(lv2);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


struct rule_set * Level2_GetRuleSetbyMethod(struct rule_level_2 *lv2, int method)
{
    struct rule_set *cur = 0;


    ENTER("lv2:%p, method:%d", lv2, method);

    DCheckPointerAndReturn(lv2, 0);

    cur = lv2->set_p;
    while (cur) {
        if (cur->method == method)
            break;
        else
            cur = cur->next_p;
    }

    // DbgMsg("get lv2, get rs(%p, method:%d) from lv2(%p)", cur, method, lv2);

    EXIT();
    return cur;
}


struct rule_set * Level2_GetRuleSetbyIndex(struct rule_level_2 *lv2, int index)
{
    int i = 0;
    struct rule_set *cur = 0;


    ENTER("lv2:%p, index:%d", lv2, index);

    DCheckPointerAndReturn(lv2, 0);

    cur = lv2->set_p;
    while (cur) {
        if (i == index)
            break;

        cur = cur->next_p;
        i++;
    }

    DDBG("lv2(%p) get rs(%p), index:%d", lv2, cur, index);

    EXIT();
    return cur;
}


int Level2_SetRuleSet(struct rule_level_2 *lv2, struct rule_set *set)
{
    struct rule_set *cur = 0, *prv = 0;
    int done = 0;


    ENTER("lv2:%p, set:%p", lv2, set);

    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(set, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    DDBG("lv2(%p, set_p:%p num_of_set:%d)", \
        lv2, lv2->set_p, lv2->num_of_set);
    if (lv2->num_of_set) {
        cur = prv = lv2->set_p;

        while (cur) {
            if (set->method == cur->method) {
                // replace cur by set
                if (lv2->set_p == cur)
                    lv2->set_p = set;
                else
                    prv->next_p = set;

                set->next_p  = cur->next_p;

                // delete cur when method conflict !
                lv2->num_of_set--;
                lv2->len_of_level_2 -= RuleSet_GetSize(cur);
                RuleSet_Delete(cur);
                done = 1;

                break;
            } else {
                prv = cur;
                cur = cur->next_p;
            }
        }

        // append to last
        if (!done) {
            prv->next_p = set;
            set->next_p = 0;
        }
    } else {
        lv2->set_p = set;
    }

    // update num_of_set, len_of_level_2
    lv2->num_of_set++;
    lv2->len_of_level_2 += RuleSet_GetSize(set);

    DDBG("set lv2, set rs(%p) to lv2(%p, len_of_level_2:%d, num_of_set:%d)", \
        set, lv2, lv2->len_of_level_2, lv2->num_of_set);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int Level2_RemoveRuleSetbyMethod(struct rule_level_2 *lv2, int method)
{
    struct rule_set *cur = 0, *prv = 0;


    ENTER("lv2:%p, method:%d", lv2, method);

    DCheckPointerAndReturn(lv2, 0);

    cur = lv2->set_p;
    while (cur) {
        if (cur->method == method) {
            break;
        } else {
            prv = cur;
            cur = cur->next_p;
        }
    }

    if (cur && prv) {
        prv->next_p = cur->next_p;

        // delete cur when method conflict !
        lv2->num_of_set--;
        lv2->len_of_level_2 -= RuleSet_GetSize(cur);
        DDBG("lv2(%p) rm rs(%p), method:%d", lv2, cur, method);
        RuleSet_Delete(cur);
    }

    EXIT();
    return VoLTE_STACK_RULE_RSP_ERROR;
}


int Level2_PackAll(struct rule_level_2 *lv2, void *p, int len)
{
    int size = 0;


    ENTER("lv2:%p, void*:%p, len:%d", lv2, p, len);

    DCheckPointerAndReturn(lv2, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(p,   VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    if ((unsigned int)len < Level2_GetSize(lv2)) {
        ErrMsg("insufficient memory space, len:%d, lv2_size:%d", len, Level2_GetSize(lv2));
        return VoLTE_STACK_RULE_RSP_INSUFFICIENT_MEMORY;
    }

    // pack lv2's "len_of_level_2", "num_of_set"
    memcpy(p, (void *)lv2, sizeof(int)*2);

    // pack lv2's rule_set
    size = sizeof(int)*2 + RuleSet_PackAll(lv2->set_p, ((char *)p)+sizeof(int)*2, len-sizeof(int)*2);

    EXIT();
    return size;
}


int Level2_Unpack2Level2(void *p, struct rule_level_2 **lv2)
{
    struct rule_level_2 *cur = 0;
    char *b = p;
    int ret = VoLTE_STACK_RULE_RSP_OK;


    ENTER("void*:%p", p);

    DCheckPointerAndReturn(p,   VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    // unpack rule_set and set to lv2
    //  -------------------- ---------------- --------
    // | len_of_level_2 (4) | num_of_set (4) | set ...
    //  -------------------- ---------------- --------
    ret = Level2_Create(&cur);
    if (VoLTE_STACK_RULE_RSP_OK != ret) {
        ErrMsg("lv2 create fail, ret:%d", ret);
        EXIT();
        return ret;
    }

    ret = RuleSet_Unpack2Level2(*((int *)b), *((int *)(b+sizeof(int))), (void *)(b+sizeof(int)*2), cur);
    if (VoLTE_STACK_RULE_RSP_OK != ret) {
        ErrMsg("lv2 create fail, ret:%d", ret);
        EXIT();
        return ret;
    }

    *lv2 = cur;

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


// -----------------
// rule set
// -----------------
int RuleSet_Create(const int method, struct rule_set **set)
{
    struct rule_set *p = 0;


    ENTER("method:%d", method);

    p = DMalloc(sizeof(struct rule_set));
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE);

    p->method = method;
    *set = p;

    DDBG("created lv2, rs(%p, method:%d)", *set, p->method);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleSet_Delete(struct rule_set *set)
{
    struct rule_item *cur = 0, *next = 0;
    int d = 0, num = 0;


    ENTER("set:%p", set);

    DCheckPointerAndReturn(set, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cur = set->item_p;
    num = set->num_of_rule;

    while (cur) {
        next = cur->next_p;

        RuleItem_Delete(cur);
        d++;
        cur = next;
    }

    DDBG("deleted lv2, rs(%p, num_of_rule:%d), del_rule:%d", set, num, d);
    DFree(set);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleSet_DeleteAll(struct rule_set *set)
{
    struct rule_set *cur = 0, *next = 0;
    int d = 0;

    ENTER("set:%p", set);

    DCheckPointerAndReturn(set, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cur = set;
    while (cur) {
        next = cur->next_p;
        RuleItem_DeleteAll(cur->item_p);
        DFree(cur);

        cur = next;
        d++;
    }

    DDBG("deleted lv2, all rs, del_set:%d", d);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


struct rule_item * RuleSet_GetRuleItembyIndex(struct rule_set *set, int index)
{
    // Note: NOT IMPLEMENT YET
    return 0;
}

int RuleSet_SetRuleItem(struct rule_set *set, struct rule_item *item)
{
    struct rule_item *cur = 0, *prv = 0;


    ENTER("set:%p, item:%p", set, item);

    DCheckPointerAndReturn(set,  VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    DDBG("set(%p, item_p:%p num_of_rule:%d)", \
        set, set->item_p, set->num_of_rule);
    if (set->num_of_rule) {
        cur = prv = set->item_p;

        while (cur) {
            prv = cur;
            cur = cur->next_p;
        }

        prv->next_p = item;
        item->next_p  = 0;
    } else {
        set->item_p = item;
    }

    set->num_of_rule++;
    set->len_of_rule_set += RuleItem_GetSize(item);

    DDBG("set lv2, set ri(%p) to rs(%p, len_of_rule_set:%d, num_of_item:%d)", \
        item, set, set->len_of_rule_set, set->num_of_rule);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleSet_RemoveRuleItembyIndex(struct rule_set *set, int index)
{
    // Note: NOT IMPLEMENT YET
    return VoLTE_STACK_RULE_RSP_ERROR;
}


int RuleSet_PackAll(struct rule_set *set, void *p, const int len)
{
    struct rule_set *cur = set;
    int size = 0;


    ENTER("set:%p, void*:%p", set, p);
    DCheckPointerAndReturn(set, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(p,   VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    while (cur) {
        memcpy((((char *)p)+size), (void *)cur, sizeof(int)*3);
        RuleItem_PackAll(cur->item_p, (((char *)p)+size)+sizeof(int)*3, len-size-sizeof(int)*3);

        size += RuleSet_GetSize(cur);
        DDBG("length, rs_len:%d", size);
        if (size > len) {
            ErrMsg("insufficient memory space, len:%d, rule_set size:%d", len, size);
            return VoLTE_STACK_RULE_RSP_INSUFFICIENT_MEMORY;
        }
        cur = cur->next_p;
    }

    EXIT();
    return size;
}


int RuleSet_Unpack2Set(int len_of_level_2, int num_of_set, void *p, struct rule_set **set)
{
    int i = 0;
    char *offset = p;

    struct rule_set *cur = 0, *prv = 0, *first = 0;


    ENTER("num_of_set:%d, len_of_level_2:%d, void*:%p", num_of_set, len_of_level_2, p);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    for ( ; i < num_of_set; i++) {
        RuleSet_Create((int)*(offset+sizeof(int)*2), &cur);
        RuleItem_Unpack2Set((int)*offset,
                            (int)*(offset+sizeof(int)*1),
                            (void *)(offset+sizeof(int)*3),
                            cur);

        if (i != 0) {
            prv->next_p = cur;
        } else {
            first = cur;
        }

        DDBG("rs:%p, rs_size:%d, offset:%p", cur, RuleSet_GetSize(cur), offset);

        prv = cur;
        offset += RuleSet_GetSize(cur);
    }

    *set = first;

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleSet_Unpack2Level2(int len_of_level_2, int num_of_set, void *p, struct rule_level_2 *lv2)
{
    int i = 0, ret = VoLTE_STACK_RULE_RSP_OK;
    char *offset = p;

    struct rule_set *cur = 0;


    ENTER("num_of_set:%d, len_of_level_2:%d, void*:%p", num_of_set, len_of_level_2, p);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    //  --------------------- ----------------- ------------ ----------
    // | len_of_rule_set (4) | num_of_rule (4) | method (4) | item ...
    //  --------------------- ----------------- ------------ ----------
    for ( ; i < num_of_set; i++) {
        ret = RuleSet_Create(*((int *)(offset+sizeof(int)*2)), &cur);
        if (VoLTE_STACK_RULE_RSP_OK != ret)
            goto RuleSet_Unpack2Level2_exception_handle;

        ret = RuleItem_Unpack2Set(*((int *)offset),
                            *((int *)(offset+sizeof(int)*1)),
                            offset+sizeof(int)*3,
                            cur);
        if (VoLTE_STACK_RULE_RSP_OK != ret)
            goto RuleSet_Unpack2Level2_exception_handle;

        ret = Level2_SetRuleSet(lv2, cur);
        if (VoLTE_STACK_RULE_RSP_OK != ret)
            goto RuleSet_Unpack2Level2_exception_handle;

        DDBG("rs:%p, rs_size:%d, offset:%p", cur, RuleSet_GetSize(cur), offset);
        offset += RuleSet_GetSize(cur);
    }

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;

RuleSet_Unpack2Level2_exception_handle:
    Level2_Delete(lv2);
    EXIT();
    return ret;
}


// -----------------
// rule item
// -----------------
int RuleItem_Create(struct rule_item **item)
{
    struct rule_item *p = 0;

    ENTER();

    p = DMalloc(sizeof(struct rule_item));
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE);

    *item = p;

    DDBG("created lv2, ri(%p)", *item);
    EXIT();

    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleItem_Delete(struct rule_item *item)
{
    struct rule_cond *cur = 0, *next = 0;
    int d = 0, num = 0;


    ENTER("item:%p", item);

    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cur = item->cond_p;
    num = item->num_of_cond;

    //RuleCond_DeleteAll(cur);
    while (cur) {
        next = cur->next_p;

        RuleCond_Delete(cur);
        d++;
        cur = next;
    }

    DDBG("deleted lv2, ri(%p, num_of_cond:%d), del_cond:%d", item, num, d);
    DFree(item);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleItem_DeleteAll(struct rule_item *item)
{
    struct rule_item *cur = 0, *next = 0;
    int d = 0;


    ENTER("item:%p", item);

    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cur = item;
    while (cur) {
        next = cur->next_p;
        RuleCond_DeleteAll(cur->cond_p);
        DFree(cur);

        cur = next;
        d++;
    }

    DDBG("deleted lv2, all ri, del_ri:%d", d);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


struct rule_cond * RuleItem_GetRuleCondbyHeader(struct rule_item *item, const int header)
{
    struct rule_cond *cur = 0;


    ENTER("item:%p, h:%d", item, header);

    DCheckPointerAndReturn(item, 0);

    cur  = item->cond_p;

    while (cur) {
        if (header == cur->header)
            break;

        cur = cur->next_p;
    }

    DDBG("get rc by header(%d), found rc:%p", header, cur);

    EXIT();
    return cur;
}


int RuleItem_SetRuleCond(struct rule_item *item, struct rule_cond *cond)
{
    struct rule_cond *cur = 0, *prv = 0;


    ENTER("item:%p, cond:%p", item, cond);

    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(cond, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    DDBG("item(%p, cond_p:%p num_of_cond:%d)", \
        item, item->cond_p, item->num_of_cond);
    if (item->num_of_cond) {
        cur = prv = item->cond_p;

        while (cur) {
            prv = cur;
            cur = cur->next_p;
        }

        prv->next_p = cond;
        cond->next_p  = 0;
    } else {
        item->cond_p = cond;
    }

    item->num_of_cond++;
    item->len_of_rule_item += RuleCond_GetSize(cond);

    DDBG("set lv2, set rc(%p) to ri(%p, len_of_rule_item:%d, num_of_cond:%d)", \
        cond, item, item->len_of_rule_item, item->num_of_cond);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleItem_PackAll(struct rule_item *item, void *p, const int len)
{
    struct rule_item *cur = item;
    int size = 0;


    ENTER("item:%p, void*:%p", item, p);
    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    while (cur) {
        memcpy((((char *)p)+size), (void *)cur, sizeof(int)*2);

        RuleCond_PackAll(cur->cond_p, ((char *)p)+size+sizeof(int)*2, len-size-sizeof(int)*2);

        size += RuleItem_GetSize(cur);
        DDBG("length, ri_len:%d", size);
        if (size > len) {
            ErrMsg("insufficient memory space, len:%d, rule_item size:%d", len, size);
            return VoLTE_STACK_RULE_RSP_INSUFFICIENT_MEMORY;
        }
        cur = cur->next_p;
    }

    EXIT();
    return size;
}


int RuleItem_Unpack2Item(int len_of_rule_set, int num_of_rule, void *p, struct rule_item **item)
{
    int i = 0;
    char *offset = p;

    struct rule_item *cur = 0, *prv = 0, *first = 0;


    ENTER("num_of_rule:%d, len_of_rule_set:%d, void*:%p", num_of_rule, len_of_rule_set, p);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    for ( ; i < num_of_rule; i++) {
        RuleItem_Create(&cur);
        RuleCond_Unpack2Item((int)*offset,
                             (int)*(offset+sizeof(int)),
                             offset+sizeof(int)*2,
                             cur);

        if (i != 0) {
            prv->next_p = cur;
        } else {
            first = cur;
        }

        DDBG("item:%p, size:%d, offset:%p", cur, RuleItem_GetSize(cur), offset);

        prv = cur;
        offset += RuleItem_GetSize(cur);
    }

    *item = first;

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleItem_Unpack2Set(int len_of_rule_set, int num_of_rule, void *p, struct rule_set *set)
{
    int i = 0;
    char *offset = p;

    struct rule_item *cur = 0;


    ENTER("num_of_rule:%d, len_of_rule_set:%d, void*:%p", num_of_rule, len_of_rule_set, p);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    //  ---------------------- ----------------- ------------
    // | len_of_rule_item (4) | num_of_cond (4) | cond ...
    //  ---------------------- ----------------- ------------
    for ( ; i < num_of_rule; i++) {
        RuleItem_Create(&cur);
        RuleCond_Unpack2Item(*((int *)offset),
                             *((int *)(offset+sizeof(int))),
                             offset+sizeof(int)*2,
                             cur);
        RuleSet_SetRuleItem(set, cur);

        DDBG("item:%p, size:%d, offset:%p", cur, RuleItem_GetSize(cur), offset);

        offset += RuleItem_GetSize(cur);
    }

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


// -----------------
// rule condition
// -----------------
int RuleCond_Create(const int header, char *str, struct rule_cond **rc)
{
    struct rule_cond *p = 0;
    int sl = 0, l = 0;
    char *s = 0;


    ENTER("h:%d, str:%s", header, str);
    DCheckPointerAndReturn(str, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    p = DMalloc(sizeof(struct rule_cond));
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE);

    sl = strlen(str)+1;
    DGetLenWithPad(sl, l);

    s = DMalloc(sizeof(char)*l);
    if (!s) {
        DFree(p);

        ErrMsg("%s, memory is not available", __func__);
        EXIT();
        return VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE;
    }

    memcpy(s, str, sl);

    p->len_of_param = l;
    p->header       = header;
    p->param_p      = s;

    *rc = p;

    DDBG("created lv2, rc(%p, h:%d, p:%s(%d),l:%d)", \
        p, p->header, p->param_p, sl, l);
    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleCond_Delete(struct rule_cond *cond)
{
    ENTER("cond:%p", cond);

    DCheckPointerAndReturn(cond, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DDBG("delete lv2, rc(%p)", cond);

    DFree(cond->param_p);
    DFree(cond);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleCond_DeleteAll(struct rule_cond * cond)
{
    struct rule_cond *cur = cond, *next = 0;
    int d = 0;


    ENTER("cond:%p", cond);
    DCheckPointerAndReturn(cond, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    while (cur) {
        next = cur->next_p;

        DFree(cur->param_p);
        DFree(cur);

        cur = next;
        d++;
    }

    DDBG("delete lv2, rm all, del_ri:%d", d);

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleCond_SetParam(struct rule_cond *rc, char *str)
{
    int sl = 0, l = 0;
    char *s = 0;


    ENTER("rc:%p, str:%s", rc, str);
    DCheckPointerAndReturn(rc,  VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(str, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    if (rc->param_p)
        DFree(rc->param_p);

    sl = strlen(str);
    DGetLenWithPad(sl, l);

    s = DMalloc(sizeof(char)*l);
    DCheckPointerAndReturn(s, VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE);

    memcpy(s, str, sl);

    rc->len_of_param = l;
    rc->param_p      = s;

    DDBG("rc(%p) set param(%p, %s(%d))", rc, &rc->param_p, rc->param_p, l);
    EXIT();

    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleCond_PackSingle(struct rule_cond *cond, void *p)
{
    ENTER("cond:%p, void*:%p", cond, p);
    DCheckPointerAndReturn(cond, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(p,    VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    memcpy(p, (void *)cond, sizeof(int)*2);
    memcpy(((char *)p)+sizeof(int)*2, cond->param_p, cond->len_of_param);

    EXIT();
    return RuleCond_GetSize(cond);
}


int RuleCond_PackAll(struct rule_cond *cond, void *p, const int len)
{
    struct rule_cond *cur = cond;
    int size = 0;


    ENTER("cond:%p, void*:%p", cond, p);
    DCheckPointerAndReturn(cond, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(p,    VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    while (cur) {
        memcpy(((char *)p)+size, (void *)cur, sizeof(int)*2);
        memcpy(((char *)p)+size+sizeof(int)*2, cur->param_p, cur->len_of_param);

        size += RuleCond_GetSize(cur);
        DDBG("length, rc_len:%d", size);
        if (size > len) {
            ErrMsg("insufficient memory space, len:%d, rule_cond size:%d", len, size);
            return VoLTE_STACK_RULE_RSP_INSUFFICIENT_MEMORY;
        }
        cur = cur->next_p;
    }

    EXIT();
    return size;
}



int RuleCond_Unpack2Cond(int num_of_cond, int len_of_rule_item, void *p, struct rule_cond **cond)
{
    int i = 0, len = 0, header = 0;
    char *s = 0, *offset = p;

    struct rule_cond *cur = 0, *prv = 0, *first = 0;


    ENTER("num_of_cond:%d, len_of_rule_item:%d, void*:%p", num_of_cond, len_of_rule_item, p);
    DCheckPointerAndReturn(p, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    for ( ; i < num_of_cond; i++) {
        len     = (int)(*offset);
        header  = (int)(*(offset+sizeof(int)*1));
        s       = (char *)(offset+sizeof(int)*1);
        DDBG("len:%d, header:%d, str:%s(%d)", len, header, s, strlen(s));

        RuleCond_Create(header, s, &cur);

        if (i != 0) {
            prv->next_p = cur;
        } else {
            first = cur;
        }

        DDBG("rule_cond:%p, size:%d, offset:%p", cur, RuleCond_GetSize(cur), offset);

        prv = cur;
        offset += RuleCond_GetSize(cur);
    }

    *cond = first;

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


int RuleCond_Unpack2Item(int len_of_rule_item, int num_of_cond, void *p, struct rule_item *item)
{
    int i = 0,  header = 0;
    char *s = 0, *offset = p;

    struct rule_cond *cond = 0;

    ENTER("len_of_rule_item:%d, num_of_cond:%d, void*:%p", len_of_rule_item, num_of_cond, p);

    DCheckPointerAndReturn(p,    VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(item, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    for ( ; i < num_of_cond; i++) {
        header  = *((int *)(offset+sizeof(int)*1));
        s       = (char *)(offset+sizeof(int)*2);

        RuleCond_Create(header, s, &cond);
        RuleItem_SetRuleCond(item, cond);

        DDBG("rule_cond:%p, size:%d, offset:%p", cond, RuleCond_GetSize(cond), offset);

        offset += RuleCond_GetSize(cond);
    }

    EXIT();
    return VoLTE_STACK_RULE_RSP_OK;
}


// -------------------------
// generic Level 2 function
// -------------------------
int Rule_Capability_Init(VoLTE_Stack_Ua_Capability_t *cap)
{
    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cap->user_info      = 0;
    cap->associated_set = 0;
    cap->feature_sets   = 0;
    cap->body_str       = 0;
    cap->lv2            = 0;

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Capability_Deinit(VoLTE_Stack_Ua_Capability_t *cap)
{
    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    if (cap->lv2) {
        Level2_Delete(cap->lv2);
        cap->lv2 = 0;
    }

    if (cap->user_info) {
        sip_free_mem(cap->user_info);
        cap->user_info = 0;
    }

    if (cap->associated_set) {
        sip_free_mem(cap->associated_set);
        cap->associated_set = 0;
    }

    if (cap->feature_sets) {
        sip_free_mem(cap->feature_sets);
        cap->feature_sets = 0;
    }

    if (cap->body_str) {
        sip_free_mem(cap->body_str);
        cap->body_str = 0;
    }

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Level0_Set(VoLTE_Stack_Ua_Capability_t *cap, char *user_info, char *assoc_set)
{
    char *ptr = 0;

    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    if (user_info) {
        cap->user_info      = voip_strdup(user_info);
        if (cap->user_info) {
            ptr = strchr(cap->user_info, '@');
            if (ptr) {
                *ptr = 0;
            }
        }
    }

    if (assoc_set) {
    	cap->associated_set = voip_strdup(assoc_set);
    }

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Level1_Set(VoLTE_Stack_Ua_Capability_t *cap, char *feature_set)
{
    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cap->feature_sets = voip_strdup(feature_set);

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Level2_Set(VoLTE_Stack_Ua_Capability_t *cap, const int method, const int header, char *field)
{
    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    if (!cap->lv2) {
        int ret = VoLTE_STACK_RULE_RSP_ERROR;

        ret = Level2_Create(&cap->lv2);
        if (VoLTE_STACK_RULE_RSP_OK != ret)
            return ret;
    }

    return Rule_Append_RuleSet_with_Single_Header_Param(cap->lv2, method, header, field);
}


int Rule_Level3_Set(VoLTE_Stack_Ua_Capability_t *cap, char *body_str)
{
    DCheckPointerAndReturn(cap, VoLTE_STACK_RULE_RSP_INVALID_POINTER);
    DCheckPointerAndReturn(body_str, VoLTE_STACK_RULE_RSP_INVALID_POINTER);

    cap->body_str = voip_strdup(body_str);

    return VoLTE_STACK_RULE_RSP_OK;
}


int Rule_Capability_Pack(VoLTE_Stack_Ua_Capability_t *cap, void **p, int *len)
{
    int size = 0, offset = 0, l = 0;
    char *pack = 0;


    if (!cap->user_info || 0 == strlen(cap->user_info)) {
        DDBG("invalid pointer or empty string, user_info(%p)", cap->user_info);
        return VoLTE_STACK_RULE_RSP_INVALID_POINTER;
    }

    // | <-- level 0 --->| <-- level 0 --->| <- level 1 -->| <- level 2 -->|
    //  ----- ----------- ----- ----------- ----- --------- ---------- ----
    // | len | user_info | len | assoc_set | len | ftr_set | lv2 ....      |
    //  ----- ----------- ----- ----------- ----- --------- ---------- ----
    DGetLenWithPad(strlen(cap->user_info), l);
    size = sizeof(int) + l;

    size += sizeof(int);
    if (cap->associated_set) {
        DGetLenWithPad(strlen(cap->associated_set), l);
        size += l;
    }

    size += sizeof(int);
    if (cap->feature_sets) {
        DGetLenWithPad(strlen(cap->feature_sets), l);
        size += l;
    }

    size += sizeof(int);
    if (cap->body_str) {
        DGetLenWithPad(strlen(cap->body_str), l);
        size += l;
    }

    if (cap->lv2) {
        size += Level2_GetSize(cap->lv2);
        DDBG("total length, lv2:%d, total:%d", Level2_GetSize(cap->lv2), size);
    }

    // pack
    {
        pack = DMalloc(size);
        memset(pack, 0, size);
        if (!pack) {
            ErrMsg("invalid pointer");
            return 0;
        }

        // lv0, user_info
        DGetLenWithPad(strlen(cap->user_info), l);
        *((int *)pack) = l;
        DDBG("pack lv0, user_info mem(%p, %d)", pack, *((int *)pack));
        offset = sizeof(int);

        memcpy(pack+offset, cap->user_info, strlen(cap->user_info));
        DDBG("pack lv0, user_info mem(%p, %s), offset:%d", \
            pack+offset, (char *)(pack+offset), offset);

        offset += l;


        // lv0, assoc_set
        if (cap->associated_set) {
            DGetLenWithPad(strlen(cap->associated_set), l);
            *((int *)(pack+offset)) = l;
        } else {
            *((int *)(pack+offset)) = 0;
        }
        DDBG("pack lv0, assoc_set mem(%p, %d), offset:%d", \
            pack+offset, *((int *)(pack+offset)), offset);

        offset += sizeof(int);

        if (cap->associated_set) {
            memcpy(pack+offset, cap->associated_set, strlen(cap->associated_set));
            DDBG("pack lv0, assoc_set mem(%p, %s), offset:%d", \
                pack+offset, (char *)(pack+offset), offset);

            offset += l;
        }

        // lv1, ftr_set
        if (cap->feature_sets) {
            DGetLenWithPad(strlen(cap->feature_sets), l);
            *((int *)(pack+offset)) = l;
        } else {
            *((int *)(pack+offset)) = 0;
        }
        DDBG("pack lv1, ftr_set mem(%p, %d), offset:%d", \
            pack+offset, *((int *)(pack+offset)), offset);

        offset += sizeof(int);

        if (cap->feature_sets) {
            memcpy(pack+offset, cap->feature_sets, strlen(cap->feature_sets));
            DDBG("pack lv1, ftr_set mem(%p, %s), offset:%d", \
                pack+offset, (char *)(pack+offset), offset);

            offset += l;
        }

        // lv3, body_str
        if (cap->body_str) {
            DGetLenWithPad(strlen(cap->body_str), l);
            *((int *)(pack+offset)) = l;
        } else {
            *((int *)(pack+offset)) = 0;
        }
        DDBG("pack lv3, body_str mem(%p, %d), offset:%d", \
            pack+offset, *((int *)(pack+offset)), offset);

        offset += sizeof(int);

        if (cap->body_str) {
            memcpy(pack+offset, cap->body_str, strlen(cap->body_str));
            DDBG("pack lv3, ftr_set mem(%p, %s), offset:%d", \
                pack+offset, (char *)(pack+offset), offset);

            offset += l;
        }

        if (cap->lv2) {
            // lv2
            Level2_PackAll(cap->lv2, (pack+offset), (size-offset));
        }
    }

    // update
    *p = pack;
    *len = size;

    return VoLTE_STACK_RULE_RSP_OK;
}


#if defined _UNIT_
#include <stdio.h>
#include <base/utest.h>

int
main(void) {
    printf("%s - %s\r\n", __DATE__, __TIME__);
    UTST_Run(0);

    return 0;
}

#elif defined _EXE_


int main(void)
{
    return 0;
}

#endif


