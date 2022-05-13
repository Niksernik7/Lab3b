#ifndef TABLE_H
#define TABLE_H
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"

typedef struct Item {
    char* data;
    char* key;
    int busy;
    size_t release;
} Item;

typedef struct KeySpace {
    size_t LenKey;
    size_t OffsetKey;
    size_t LenData;
    size_t release;
    int busy;
} KeySpace;

typedef struct Vector{
    Item* items;
    size_t size;
} Vector;

struct Table;
typedef struct Table Table;

bool Insert(Table*, const char*, const char*);
bool FindByKey(const Table*, const char*, Vector*);
bool DeleteByKey(Table*, const char*);
Item* FindByReleaseKey(const Table*, const char*, size_t);
bool DeleteByReleaseKey(Table*, const char*, size_t);

 Table* TableLoad(FILE*);
 size_t EmptyCheck(const Table*);
 size_t hashFunc(const char*, size_t);
 void FreeItem(Item*);
 void TablePrint(Table*, FILE*);
 void FreeTable(Table*);
 Table* TableCreate(size_t, FILE*);
 char* GetStrForDataAndKey(size_t, size_t, FILE*);
#endif
