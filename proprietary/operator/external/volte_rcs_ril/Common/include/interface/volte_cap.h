#ifndef __VOLTE_CAP_H__
#define __VOLTE_CAP_H__

/**
 * @brief
 */
enum {
    VoLTE_STACK_RULE_RSP_OK = 0,

    VoLTE_STACK_RULE_RSP_ERROR                  = -1,

    VoLTE_STACK_RULE_RSP_INVALID_POINTER        = -2,
    VoLTE_STACK_RULE_RSP_INSUFFICIENT_MEMORY    = -3,
    VoLTE_STACK_RULE_RSP_MEMORY_NOT_AVAILABLE   = -4,
    VoLTE_STACK_RULE_RSP_RULE_SET_NOT_FOUND     = -5,
};


/**
 *	@brief Level 2 of matching rules
 */
typedef struct rule_cond {
    int     len_of_param;
    int     header;
    char    *param_p;
    struct rule_cond *next_p;
} VoLTE_Stack_Match_Rule_Cond;

typedef struct rule_item {
    int     len_of_rule_item;
    int     num_of_cond;
    struct rule_cond *cond_p;
    struct rule_item *next_p;
} VoLTE_Stack_Match_Rule_Item;

typedef struct rule_set {
    int     len_of_rule_set;
    int     num_of_rule;
    int     method;

    struct rule_item    *item_p;
    struct rule_set     *next_p;
} VoLTE_Stack_Match_Rule_Set;

typedef struct rule_level_2{
    int     len_of_level_2;
    int     num_of_set;
    struct rule_set     *set_p;
} VoLTE_Stack_Match_Level_2;


typedef struct _VoLTE_Stack_Ua_Capability {
    char                        *user_info;         // lv0, ex. "volte_ua"
    char                        *associated_set;    // lv0, ex. "sip:ua@home,tel:+19911"
    char                        *feature_sets;      // lv1, ex. "+g.3gpp.cs-voice,urn:urn-7:3gpp-application.ims.iari.gsma-is"
    char                        *body_str;          // lv3, ex. "\r\nm=audio "
    VoLTE_Stack_Match_Level_2   *lv2;               // lv2, a tree obj
} VoLTE_Stack_Ua_Capability_t;


// -------------------------
// Rule generic function
// -------------------------
int Rule_Capability_Init(VoLTE_Stack_Ua_Capability_t *cap);
int Rule_Capability_Deinit(VoLTE_Stack_Ua_Capability_t *cap);

int Rule_Level0_Set(VoLTE_Stack_Ua_Capability_t *cap, char *user_info, char *assoc_set);

int Rule_Level1_Set(VoLTE_Stack_Ua_Capability_t *cap, char *feature_set);

typedef struct {
    int     header;
    char    *param;
} RuleCond_Context_t;

int Rule_Level2_Set(VoLTE_Stack_Ua_Capability_t *cap, const int method, const int header, char *field);
int Rule_Level3_Set(VoLTE_Stack_Ua_Capability_t *cap, char *body_str);


// -----------------
// level 2
// -----------------
int Level2_Create(struct rule_level_2 **lv2);
int Level2_Delete(struct rule_level_2 *lv2);

/**
 *  @brief calculate rule_cond for continuous memory allocation
 *  @param rc
 *  @note  size contains "len_of_param(4)+header(4)+param(n)",
 *         not include param_p, next_p
 */
#define Level2_GetSize(l) \
    (sizeof(struct rule_level_2)+((l)->len_of_level_2)-(sizeof(struct rule_set *)))

#define Level2_GetTotoalRuleSetNum(l) \
    ((l)->num_of_set)

struct rule_set * Level2_GetRuleSetbyMethod(struct rule_level_2 *lv2, int method);
struct rule_set * Level2_GetRuleSetbyIndex(struct rule_level_2 *lv2, int index);

int Level2_SetRuleSet(struct rule_level_2 *lv2, struct rule_set *set);
int Level2_RemoveRuleSetbyMethod(struct rule_level_2 *lv2, int method);

int Level2_PackAll(struct rule_level_2 *lv2, void *p, int len);
int Level2_Unpack2Level2(void *p, struct rule_level_2 **lv2);


// -----------------
// rule set
// -----------------
int RuleSet_Create(const int method, struct rule_set **set);
int RuleSet_Delete(struct rule_set *set);
int RuleSet_DeleteAll(struct rule_set *set);

struct rule_item * RuleSet_GetRuleItembyIndex(struct rule_set *set, int index);

int RuleSet_SetRuleItem(struct rule_set *set, struct rule_item *item);
int RuleSet_RemoveRuleItembyIndex(struct rule_set *set, int index);

/**
 *  @brief calculate rule_cond for continuous memory allocation
 *  @param rc
 *  @note  size contains "len_of_param(4)+header(4)+param(n)",
 *         not include param_p, next_p
 */
#define RuleSet_GetSize(rs) \
    (sizeof(struct rule_set)+(rs)->len_of_rule_set-(sizeof(struct rule_item *)+sizeof(struct rule_set *)))

#define RuleSet_GetTotoalRuleItemNum(rs) \
    (rs)->num_of_rule

int RuleSet_PackAll(struct rule_set *set, void *p, const int len);
int RuleSet_Unpack2Set(int len_of_level_2, int num_of_set, void *p, struct rule_set **set);
int RuleSet_Unpack2Level2(int len_of_level_2, int num_of_set, void *p, struct rule_level_2 *lv2);


// -----------------
// rule item
// -----------------
int RuleItem_Create(struct rule_item **item);
int RuleItem_Delete(struct rule_item *item);
int RuleItem_DeleteAll(struct rule_item *item);

struct rule_cond * RuleItem_GetRuleCondbyHeader(struct rule_item *item, const int header);

int RuleItem_SetRuleCond(struct rule_item *item, struct rule_cond *cond);

/**
 *  @brief calculate rule_cond for continuous memory allocation
 *  @param rc
 *  @note  size contains "len_of_param(4)+header(4)+param(n)",
 *         not include param_p, next_p
 */
#define RuleItem_GetSize(ri) \
    (sizeof(struct rule_item)+((ri)->len_of_rule_item)-(sizeof(struct rule_cond *)+sizeof(struct rule_item *)))

#define RuleItem_GetTotoalRuleCondNum(ri) \
    (ri)->num_of_cond

int RuleItem_PackAll(struct rule_item *item, void *p, const int len);
int RuleItem_Unpack2Item(int len_of_rule_set, int num_of_rule, void *p, struct rule_item **item);
int RuleItem_Unpack2Set(int len_of_rule_set, int num_of_rule, void *p, struct rule_set *set);


// -----------------
// rule condition
// -----------------
int RuleCond_Create(const int header, char *str, struct rule_cond **cond);
int RuleCond_Delete(struct rule_cond *cond);
int RuleCond_DeleteAll(struct rule_cond * cond);

#define RuleCond_SetHeader(rc, h) \
    (rc)->header=(h)

int RuleCond_SetParam(struct rule_cond *cond, char *str);

/**
 *  @brief calculate rule_cond for continuous memory allocation
 *  @param rc
 *  @note  size contains "len_of_param(4)+header(4)+param(n)",
 *         not include param_p, next_p
 */
#define RuleCond_GetSize(rc) \
    (sizeof(struct rule_cond)+((rc)->len_of_param)-(sizeof(char *)+sizeof(struct rule_cond *)))


int RuleCond_PackSingle(struct rule_cond *cond, void *p);
int RuleCond_PackAll(struct rule_cond *cond, void *p, const int len);

int RuleCond_Unpack2Cond(int num_of_cond, int len_of_rule_item, void *p, struct rule_cond **cond);
int RuleCond_Unpack2Item(int len_of_rule_item, int num_of_cond, void *p, struct rule_item *item);


// -------------------------------------------------
// VoLTE_Stack_Ua_Capability_t pack/unpack function
// -------------------------------------------------
int Rule_Capability_Pack(VoLTE_Stack_Ua_Capability_t * cap, void **p, int *len);

#define volte_pack_ua_cap(c, p, l)  Rule_Capability_Pack((c), (p), (l))


#endif /* __VOLTE_CAP_H__ */

