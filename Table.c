#include <errno.h>
#include "Table.h"

typedef struct DiskTableHeader {
    size_t msize;
    size_t csize;
    KeySpace keySpace[];
} DiskTableHeader;

struct Table{
    FILE* file;
    struct DiskTableHeader header;
};

 bool Insert(Table *table, const char* key, const char* data) {
     if (table->header.csize == table->header.msize) {
         return false;
     }

     size_t hashKey = hashFunc(key, table->header.msize);
     size_t maxRel = 0;
     for (size_t i = 0; i < table->header.msize; i++) {
         char* curk = GetStrForDataAndKey(table->header.keySpace[i].LenKey,
          table->header.keySpace[i].OffsetKey,table->file);
         if (curk != NULL) {
             if (strcmp(curk, key) == 0) {
                 if (table->header.keySpace[i].release > maxRel)
                     maxRel = table->header.keySpace[i].release;
             }
             free(curk);
         }
     }

     fseek(table->file, 0L, SEEK_END);
     long OffsetKey = ftell(table->file);
     size_t LenKey = strlen(key);
     size_t LenData = strlen(data);
     fwrite(key, sizeof(char), LenKey, table->file);
     fwrite(data, sizeof(char), LenData, table->file);

     size_t i = hashKey;
     do {
         if (table->header.keySpace[i].busy == 0 || table->header.keySpace[i].busy == 2) {
             table->header.keySpace[i].OffsetKey = OffsetKey;
             table->header.keySpace[i].LenKey = LenKey;
             table->header.keySpace[i].LenData = LenData;
             table->header.keySpace[i].busy = 1;
             table->header.keySpace[i].release = maxRel + 1;
             table->header.csize++;
             fseek(table->file, sizeof(table->header.msize), SEEK_SET);
             fwrite(&table->header.csize, 1, sizeof(table->header.csize), table->file);
             fseek(table->file, i*sizeof(KeySpace), SEEK_CUR);
             fwrite(&table->header.keySpace[i], 1, sizeof(KeySpace), table->file);
             fflush(table->file);
             return true;
         }
         i = (i + 1) % table->header.msize;
     } while (i != hashKey);

     return 0;
}

bool FindByKey(const Table* table, const char* key, Vector *result) {
    Item *res = NULL;
    size_t nres = 0; // количество найденных элементов
    size_t hashKey = hashFunc(key, table->header.msize);
    size_t i = hashKey;
    do {
        char *curk = GetStrForDataAndKey(table->header.keySpace[i].LenKey,
        table->header.keySpace[i].OffsetKey, table->file);
        if (curk != NULL && table->header.keySpace[i].busy != 0 && strcmp(key, curk) == 0) {
            Item *tmp = realloc(res, (nres + 1) * sizeof(Item));
            if (tmp == NULL) {
                free(curk);
                return false;
            }
            res = tmp;
            res[nres].key = curk;
            res[nres].busy = table->header.keySpace[i].busy;
            res[nres].release = table->header.keySpace[i].release;
            res[nres].data = GetStrForDataAndKey(table->header.keySpace[i].LenData,
             table->header.keySpace[i].OffsetKey + table->header.keySpace[i].LenKey,table->file);
            nres++;
        } else
            free(curk);
        i = (i + 1) % table->header.msize;
    } while (i != hashKey);
    if (res == NULL) {
        errno = ESRCH;
        return false;
    }
    result->items = res;
    result->size = nres;
    return true;

}

bool DeleteByKey( Table* table,  const char* key){
    Vector result;
    if (!FindByKey(table, key, &result))
        return false;  // НЕТ ТАКИХ ЭЛЕМЕНТОВ
    for (size_t i = 0; i < table->header.msize; i++){
        char* curk = GetStrForDataAndKey(table->header.keySpace[i].LenKey,
        table->header.keySpace[i].OffsetKey,table->file);
        for (size_t j = 0; j < result.size; j++){
            char* ftcurk = result.items[j].key;
            if (curk != NULL && table->header.keySpace[i].busy != 0) {
                if (strcmp(curk, ftcurk) == 0) {
                    table->header.keySpace[i].release = 0;
                    table->header.keySpace[i].busy = 2;
                    table->header.csize--;
                    fseek(table->file, sizeof(table->header.msize), SEEK_SET);
                    fwrite(&table->header.csize, 1, sizeof(table->header.csize), table->file);
                    fseek(table->file, i*sizeof(KeySpace), SEEK_CUR);
                    fwrite(&table->header.keySpace[i], 1, sizeof(KeySpace), table->file);
                    fflush(table->file);
                }
            }
        }
        free(curk);
    }
    freeVector(&result);
    return true;
}

Item* FindByReleaseKey(const Table* table, const char* key, size_t release){
    Item* res = NULL;
    size_t hashKey = hashFunc(key, table->header.msize);
    size_t i = hashKey;
    do {
        char* curk = GetStrForDataAndKey(table->header.keySpace[i].LenKey,
        table->header.keySpace[i].OffsetKey,table->file);
        if (curk != NULL && table->header.keySpace[i].busy != 0 && strcmp(curk, key) == 0 &&
                (table->header.keySpace[i].release == release)) {
                res = calloc(1, sizeof(KeySpace));
                res->busy = table->header.keySpace[i].busy;
                res->release = table->header.keySpace[i].release;
                res->data = GetStrForDataAndKey(table->header.keySpace[i].LenData,
                table->header.keySpace[i].OffsetKey + table->header.keySpace[i].LenKey,table->file);
                res->key = curk;
                return res;
        }
        free(curk);
        i = (i + 1) % table->header.msize;
    } while (i != hashKey);
    return NULL;
}

bool DeleteByReleaseKey(Table* table, const char* key, size_t release){
    for (int i = 0; i < table->header.msize; i++) {
        char* curk = GetStrForDataAndKey(table->header.keySpace[i].LenKey,
        table->header.keySpace[i].OffsetKey,table->file);
        if (curk != NULL && table->header.keySpace[i].busy != 0) {
            if (strcmp(curk, key) == 0 && (table->header.keySpace[i].release == release)) {
                table->header.keySpace[i].release = 0;
                table->header.keySpace[i].busy = 2;
                table->header.csize--;
                free(curk);
                return true;
            }
        }
    }
    return false;
}

 Table* TableCreate(size_t size, FILE* file) {
    size_t sizes[2] = {size, 0};
    Table* table = calloc(1,  sizeof(Table) + sizeof(KeySpace) * size);
    table->header.msize = size;
    fwrite(sizes, sizeof(size_t), 2, file);
    fwrite(table->header.keySpace, sizeof(KeySpace), size, file);
    table->file = file;
    return table;
}

 Table* TableLoad(FILE* file) {
    size_t sizes[2];
    fread(sizes, sizeof(size_t), 2, file);
    Table *table = malloc(sizeof(Table) + sizeof(KeySpace) * sizes[0]);
    fread(table->header.keySpace, sizeof(KeySpace), sizes[0], file);
    table->header.msize = sizes[0];
    table->header.csize = sizes[1];
    table->file = file;
    return table;
}

 size_t hashFunc(const char* s, size_t n) {
    size_t h = 2353 % n;
    for (size_t i = 0; i < strlen(s); i++) {
        h = (h + (h << 5) + s[i]) % n;
    }
    return h % n;
}




 void FreeTable(Table* table) {
     fclose(table->file);
     free(table);
}

 void TablePrint(Table* table, FILE *f) {

     for (size_t i = 0; i < table->header.msize; i++){
         const KeySpace *ks = &table->header.keySpace[i];
         if (ks->busy == 0)
             continue;
         char* data = GetStrForDataAndKey(ks->LenData,ks->OffsetKey + ks->LenKey,table->file);
         char* key  = GetStrForDataAndKey(ks->LenKey, ks->OffsetKey,table->file);
         fprintf(f, "%d | %s | %s | %zu\n",ks->busy, key, data, ks->release);
     }
}

char* GetStrForDataAndKey(size_t len, size_t offset, FILE* file) {
    char* str = calloc(len + 1, sizeof(char));
    fseek(file, offset, SEEK_SET);
    fread(str, sizeof(char), len, file);
    return str;
}

void FreeItem(Item* item){
    free(item->key);
    free(item->data);
}

size_t EmptyCheck(const Table* t){
    return t->header.csize;
}


void freeVector(Vector *v) {
    if (v == NULL)
        return;
    free(v->items);
}