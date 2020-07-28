#ifndef DATA_ITEM_H
#define DATA_ITEM_H

#include "types.h"
#include "BufferManager.h"

#define DEST_TYPE_STANDARD_XMP 0
#define DEST_TYPE_EXTENDED_XMP 1

typedef struct NameSpaceItem {
    int dest;
    StereoString nameSpace;
    StereoString nameSpacePrefix;
} NameSpaceItem_t;

typedef struct SimpleItem {
    int dest;
    NameSpaceItem nameSpaceItem;
    StereoString name;
    StereoString value;
} SimpleItem_t;

typedef struct BufferItem {
    int dest;
    NameSpaceItem nameSpaceItem;
    StereoString name;
    stereo::StereoBuffer_t value;
} BufferItem_t;

typedef struct ArrayItem {
    int dest;
    int index;
    NameSpaceItem nameSpaceItem;
    StereoString name;
    StereoString value;
} ArrayItem_t;

typedef struct StructItem {
    int dest;
    NameSpaceItem structNameSpaceItem;
    NameSpaceItem fieldNameSpaceItem;
    StereoString structName;
    StereoString fieldName;
    StereoString fieldValue;
} StructItem_t;

typedef struct DataCollections {
    int dest;
    StereoVector<SimpleItem*> *listOfSimpleValue;
    StereoVector<BufferItem*> *listOfBufferItem ;
    StereoVector<ArrayItem*> *listOfArrayItem;
    StereoVector<StructItem*> *listOfStructItem;
    StereoVector<BufferItem*> *listOfCustomDataItem;

    DataCollections() :
        dest(0), listOfSimpleValue(nullptr),
        listOfBufferItem(nullptr), listOfArrayItem(nullptr),
        listOfStructItem(nullptr), listOfCustomDataItem(nullptr) {}
} DataCollections_t;

#endif