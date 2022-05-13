#include <errno.h>
#include "Menu.h"

char* enterString()
{
    char buf[81] = { 0 };
    char* res = NULL;
    size_t len = 0;
    int n;
    do {
        n = scanf("%80[^\n]", buf);
        if (n < 0)
        {
            if (!res)
            {
                return NULL;
            }
        }
        else if (n > 0) {
            size_t chunk_len = strlen(buf);
            size_t str_len = len + chunk_len;
            res = realloc(res, str_len + 2);
            memcpy(res + len, buf, chunk_len);
            len = str_len;
        }
        else {
            scanf("%*c");
        }
    } while (n > 0);
    if (len > 0)
    {
        res[len] = '\0';
    }
    else {
        res = calloc(1, sizeof(char));
        if (res != NULL)
            *res = '\0';
    }
    return res;
}

void printItem(const Item *item) {
    printf("%d | %s | %s | %zu\n", item->busy, item->key, item->data, item->release);
}

void ShowFindMenu() {
    printf("Find\n");
    printf("Choice\n");
    printf("1) Find in table by release and key\n");
    printf("2) Find in table by key\n");
    printf("0) Exit\n");
    printf(" : ");
}

void ShowDeleteMenu() {
    printf("Delete\n");
    printf("Choice\n");
    printf("1) Delete by key\n");
    printf("2) Delete by release and key\n");
    printf("0) Exit\n");
    printf(" : ");
}

void ShowError() {
    printf("Miss choice\n");
}

void ShowMainMenu() {
    printf("Menu\n");
    printf("Choice\n");
    printf("1. Print table\n");
    printf("2. Insert item\n");
    printf("3. Delete item\n");
    printf("4. FindInTable item\n");
    printf("5. Load from file\n");
    printf("0. Quit\n");
    printf(" : ");
}



void printTable(Table* t) {
    if (t == NULL) {
        printf("Doesn't exist\n");
        return;
    }
    printf("Output\n");
    if (EmptyCheck(t)) {
        TablePrint(t, stdout);
    } else printf("Table is empty\n");
}

int GetInt() {
    char* str = "Doesn't number!\n : ";
    char* ptr;
    do {
        char* input = (char*) enterString();
        int n = (int)strtol(input, &ptr, 10);
        if (input == ptr - strlen(input) && strlen(input) != 0) {
            free(input);
            return n;
        }
        printf("%s", str);
        free(input);
    } while (1);
}



void Menu() {
    Table *table = Create();
    do {
        ShowMainMenu();
        int n = GetInt();
        switch (n) {
            case 0: {
                FreeTable(table);
                return;
            }
            case 1: {
                printTable(table);
                break;
            }
            case 2: {
                InsertInTable(table);
                break;
            }
            case 3: {
                ShowDeleteMenu();
                n = GetInt();
                Delete(table, n);
                break;
            }
            case 4: {
                ShowFindMenu();
                n = GetInt();
                FindInTable(table, n);
                break;
            }
            default: {
                ShowError();
                break;
            }
        }
    } while (1);
}

Table *Create() {
    FILE* f;
    Table* table;
    printf("Name of file:\n");
    char* nfile = enterString();
    f = fopen(nfile, "r+b");
    if (f == NULL) {
        if (errno == ENOENT) {
            f = fopen(nfile, "w+b");
            if (f == NULL) {
                fprintf(stderr, "Could not create file: %s\n", strerror(errno));
                free(nfile);
                return NULL;
            }
            // файл пуст или только что был создан
            printf("Enter size of table:\n");
            printf(" : ");
            char* str = "Must be greater then zero!!!\n : ";
            int size;
            do {
                size = GetInt();
                if (size > 0)
                    break;
                puts(str);
            } while (1);
            table = TableCreate(size, f);
        } else {
            fprintf(stderr, "Could not open file: %s\n", strerror(errno));
            free(nfile);
            return NULL;
        }
    } else {
        free(nfile);
        table = TableLoad(f);
    }
    return table;
}

void Delete(Table *table, int mod) {
    char* key, * msg = "";
    switch (mod) {
        case 0:
            break;

        case 1: {
            printf("Enter:\n");
            printf(" Key : ");
            do {
                printf("%s", msg);
                key = enterString();
                msg = "Key cannot be empty\n Key : ";
                if(strlen(key) != 0) break;
                free(key);
            } while (1);
            if(!DeleteByKey(table, key))
                printf("Doesn't exist\n");
            free(key);
            break;
        }
        case 2: {
            printf("Enter:\n");
            printf(" Key : ");
            key = enterString();
            printf(" Release : ");
            int release = GetInt();
            if(!DeleteByReleaseKey(table, key, release))
                printf("Doesn't exist\n");
            free(key);
            break;
        }
        default:
            ShowError();
            break;
    }
}

void InsertInTable(Table* table) {
    char* str = "Data cannot be empty\n Data : ";
    printf(" Enter\n");
    printf(" Data : ");
    char* data;
    do {
        data = (char*) enterString();
        if (strlen(data) != 0)
            break;
        printf("%s", str);
        free(data);
    } while (1);
    printf(" Key : ");
    char* key;
    do {
        key = enterString();
        if (strlen(key) != 0)
            break;
        free(key);
        printf("%s", str);
    } while (1);
    if (!Insert(table, key, data)) {
        printf("Check count of elements in table, because it is full.\n ");
        printf("If you are sure that it isn't full, then you dont have enough memory.\n");
    }
    free(data);
    free(key);
}

void FindInTable(Table *table, size_t mod) {
    switch (mod) {
        case 0:
            break;
        case 1: {
            printf("Enter:\n");
            printf(" Key : ");
            char* key = enterString();
            printf(" Release : ");
            size_t release = GetInt();
            Item* item = FindByReleaseKey(table, key, release);
            if (item != NULL) {
                printItem(item);
                FreeItem(item);
            } else printf("Doesn't exist\n");
            free(key);
            break;
        }
        case 2: {
            printf("Enter:\n");
            printf(" Key : ");
            char *key = enterString();
            Vector result;
            if (FindByKey(table, key, &result)) {
                for (size_t i = 0; i < result.size; i++) {
                    printItem(&result.items[i]);
                }
            }
            free(key);
            break;
        }
        default:
            ShowError();
            break;
    }
}


