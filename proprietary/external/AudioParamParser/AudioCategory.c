/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
 * Description:
 *   Implement CategoryType related APIs
 */
#include "AudioParamParserPriv.h"

EXPORT CategoryAlias *categoryAliasCreate(const char *alias, Category *category) {
    CategoryAlias *categoryAlias = NULL;
    categoryAlias = (CategoryAlias *)malloc(sizeof(CategoryAlias));
    if (!categoryAlias) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    categoryAlias->alias = strdup(alias);
    categoryAlias->category = category;

    return categoryAlias;
}

EXPORT void categoryAliasRelease(CategoryAlias *categoryAlias) {
    free(categoryAlias->alias);
    free(categoryAlias);
}

EXPORT CategoryType *categoryTypeCreate(const char *name, const char *wording, AudioType *audioType, int visible) {
    CategoryType *categoryType = NULL;
    categoryType = (CategoryType *)malloc(sizeof(CategoryType));
    if (!categoryType) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    categoryType->name = strdup(name);
    categoryType->wording = strdup(wording);
    categoryType->audioType = audioType;
    categoryType->visible = visible;
    categoryType->categoryHash = NULL;
    categoryType->categoryAliasHash = NULL;
    categoryType->categoryGroupHash = NULL;
    categoryType->allCategoryHash = NULL;
    return categoryType;
}

EXPORT void categoryTypeRelease(CategoryType *categoryType) {
    free(categoryType->name);
    free(categoryType->wording);

    if (categoryType->allCategoryHash) {
        Category *tmp, *item;
        HASH_ITER(hh2, categoryType->allCategoryHash, item, tmp) {
            if (categoryType->allCategoryHash) {
                HASH_DELETE(hh2, categoryType->allCategoryHash, item);
            }
        }
    }

    if (categoryType->categoryHash) {
        Category *tmp, *item;
        HASH_ITER(hh, categoryType->categoryHash, item, tmp) {
            if (categoryType->categoryHash) {
                HASH_DEL(categoryType->categoryHash, item);
                categoryRelease(item);
            }
        }
    }

    if (categoryType->categoryAliasHash) {
        CategoryAlias *tmp, *item;
        HASH_ITER(hh, categoryType->categoryAliasHash, item, tmp) {
            if (categoryType->categoryAliasHash) {
                HASH_DEL(categoryType->categoryAliasHash, item);
                categoryAliasRelease(item);
            }
        }
    }
    free(categoryType);
}

EXPORT size_t categoryTypeGetNumOfCategory(CategoryType *categoryType) {
    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return 0;
    }

    return HASH_COUNT(categoryType->categoryHash);
}

EXPORT size_t categoryTypeGetNumOfAllCategory(CategoryType *categoryType) {
    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return 0;
    }

    return HASH_CNT(hh2, categoryType->allCategoryHash);
}

EXPORT size_t categoryTypeGetNumOfCategoryGroup(CategoryType *categoryType) {
    if (!categoryType) {
        ERR_LOG("CategoryType is NULL\n");
        return 0;
    }

    return HASH_COUNT(categoryType->categoryGroupHash);
}

EXPORT Category *categoryTypeGetAllCategoryByIndex(CategoryType *categoryType, size_t index) {
    Category *category = NULL;
    size_t i = 0;

    for (category = categoryType->allCategoryHash; category ; category = category->hh2.next) {
        if (index == i++) {
            return category;
        }
    }

    return NULL;
}

EXPORT Category *categoryTypeGetCategoryByIndex(CategoryType *categoryType, size_t index) {
    Category *category = NULL;
    size_t i = 0;

    for (category = categoryType->categoryHash; category ; category = category->hh.next) {
        if (index == i++) {
            return category;
        }
    }

    return NULL;
}

EXPORT CategoryGroup *categoryTypeGetCategoryGroupByIndex(CategoryType *categoryType, size_t index) {
    CategoryGroup *categoryGroup = NULL;
    size_t i = 0;

    if (!categoryType) {
        ERR_LOG("CategoryType is NULL\n");
        return NULL;
    }

    for (categoryGroup = categoryType->categoryGroupHash; categoryGroup ; categoryGroup = categoryGroup->hh.next) {
        if (index == i++) {
            return categoryGroup;
        }
    }

    return NULL;
}

EXPORT CategoryAlias *categoryTypeGetCategoryByAlias(CategoryType *categoryType, const char *alais) {
    CategoryAlias *categoryAlias;

    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return NULL;
    }

    HASH_FIND_STR(categoryType->categoryAliasHash, alais, categoryAlias);

    return categoryAlias;
}

EXPORT Category *categoryTypeGetCategoryByName(CategoryType *categoryType, const char *name) {
    CategoryGroup *categoryGroup;
    Category *category;

    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return NULL;
    }

    if (!name) {
        ERR_LOG("name is NULL\n");
        return NULL;
    }

    for (categoryGroup = categoryType->categoryGroupHash; categoryGroup; categoryGroup = categoryGroup->hh.next) {
        for (category = categoryGroup->categoryHash; category; category = category->hh.next) {
            if (!strncmp(category->name, name, strlen(name) + 1)) {
                return category;
            }
        }
    }

    for (category = categoryType->categoryHash; category; category = category->hh.next)
    {
        if (!strcmp(category->name, name))
        {
            return category;
        }
    }

    return NULL;
}

EXPORT Category *categoryTypeGetCategoryByWording(CategoryType *categoryType, const char *wording) {
    Category *category;

    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return NULL;
    }

    HASH_FIND_STR(categoryType->categoryHash, wording, category);

    return category;
}

EXPORT CategoryGroup *categoryTypeGetCategoryGroupByWording(CategoryType *categoryType, const char *wording) {
    CategoryGroup *categoryGroup;

    if (!categoryType) {
        ERR_LOG("categoryType is NULL!\n");
        return NULL;
    }

    HASH_FIND_STR(categoryType->categoryGroupHash, wording, categoryGroup);

    return categoryGroup;
}

EXPORT CategoryGroup *categoryGroupCreate(const char *name, const char *wording, CategoryType *categoryType, int visible) {
    CategoryGroup *categoryGroup = NULL;
    categoryGroup = (CategoryGroup *)malloc(sizeof(CategoryGroup));
    if (!categoryGroup) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    categoryGroup->name = strdup(name);
    categoryGroup->wording = strdup(wording);
    categoryGroup->categoryType = categoryType;
    categoryGroup->visible = visible;
    categoryGroup->categoryHash = NULL;
    return categoryGroup;
}

EXPORT void categoryGroupRelease(CategoryGroup *categoryGroup) {
    free(categoryGroup->name);
    free(categoryGroup->wording);
    if (categoryGroup->categoryHash) {
        Category *tmp, *item;
        HASH_ITER(hh, categoryGroup->categoryHash, item, tmp) {
            if (categoryGroup->categoryHash) {
                HASH_DEL(categoryGroup->categoryHash, item);
                categoryRelease(item);
            }
        }
    }
    free(categoryGroup);
}

EXPORT size_t categoryGroupGetNumOfCategory(CategoryGroup *categoryGroup) {
    if (!categoryGroup) {
        ERR_LOG("categoryGroup is NULL!\n");
        return 0;
    }

    return HASH_COUNT(categoryGroup->categoryHash);
}

EXPORT Category *categoryCreate(const char *name, const char *wording, CATEGORY_PARENT_TYPE isCategoryGroup, void *parent, int visible) {
    Category *category = NULL;
    category = (Category *)malloc(sizeof(Category));
    if (!category) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    category->name = strdup(name);
    category->wording = strdup(wording);
    category->parentType = isCategoryGroup;
    if (isCategoryGroup) {
        category->parent.categoryType = (CategoryType *)parent;
    } else {
        category->parent.category = (Category *)parent;
    }
    category->visible = visible;

    return category;
}

EXPORT void categoryRelease(Category *category) {
    free(category->name);
    free(category->wording);
    free(category);
}

EXPORT Category *categoryGroupGetCategoryByIndex(CategoryGroup *categoryGroup, size_t index) {
    Category *category = NULL;
    size_t i = 0;

    if (!categoryGroup) {
        ERR_LOG("categoryGroup is NULL!\n");
        return NULL;
    }

    for (category = categoryGroup->categoryHash; category ; category = category->hh.next) {
        if (index == i++) {
            return category;
        }
    }

    return NULL;
}

EXPORT Category *categoryGroupGetCategoryByWording(CategoryGroup *categoryGroup, const char *wording) {
    Category *category;

    if (!categoryGroup) {
        ERR_LOG("categoryGroup is NULL!\n");
        return 0;
    }

    HASH_FIND_STR(categoryGroup->categoryHash, wording, category);

    return category;
}
