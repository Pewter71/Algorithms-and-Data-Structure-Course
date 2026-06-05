#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_LEN 64
#define MAX_TABLE_SIZE 100

const size_t MIN_ELEMENTS = 14;



typedef struct {
    char key[MAX_KEY_LEN];
    int data;
} Record;


typedef struct {
    Record items[MAX_TABLE_SIZE];
    size_t count;
} Table;


int CompareRecords(const void* a, const void* b) {
    const Record* ra = (const Record*)a;
    const Record* rb = (const Record*)b;
    return strcmp(ra->key, rb->key);
}


int CompareRecordsReverse(const void* a, const void* b) {
    return CompareRecords(b, a);
}


size_t FindInsertPosition(Table* table, size_t sorted_count, Record* target) {
    size_t left = 0;
    size_t right = sorted_count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (strcmp(target->key, table->items[mid].key) < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;
}


void BinaryInsertionSort(Table* table) {
    if (table->count <= 1) {
        return;
    }
    for (size_t i = 1; i < table->count; ++i) {
        Record temp = table->items[i];
        size_t pos = FindInsertPosition(table, i, &temp);
        for (size_t j = i; j > pos; --j) {
            table->items[j] = table->items[j - 1];
        }
        table->items[pos] = temp;
    }
}


int BinarySearch(Table* table, char* key) {
    if (table->count == 0) {
        return -1;
    }
    size_t left = 0;
    size_t right = table->count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = strcmp(key, table->items[mid].key);
        if (cmp == 0) {
            return (int)mid;
        } else if (cmp < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return -1;
}


void PrintTable(Table* table) {
    printf("%s %s\n", "Ключ", "Данные");
    for (size_t i = 0; i < table->count; ++i) {
        printf("%s %d\n", table->items[i].key, table->items[i].data);
    }
}


int ReadTable(Table* table) {
    size_t n = 0;
    printf("Введите количество элементов: ");
    if (scanf("%zu", &n) != 1) {
        fprintf(stderr, "Ошибка. Некорректный ввод числа элементов\n");
        return 0;
    }
    if (n < MIN_ELEMENTS) {
        fprintf(stderr, "Ошибка. Требуется не менее %zu элементов\n", MIN_ELEMENTS);
        return 0;
    }
    if (n > (size_t)MAX_TABLE_SIZE) {
        fprintf(stderr, "Ошибка. Таблица вмещает не более %d элементов\n", MAX_TABLE_SIZE);
        return 0;
    }
    table->count = n;
    printf("Введите элементы (в формате 'строка-ключ целое_число'):\n");
    for (size_t i = 0; i < n; ++i) {
        if (scanf("%63s %d", table->items[i].key, &table->items[i].data) != 2) {
            fprintf(stderr, "Ошибка при чтении элемента\n");
            return 0;
        }
    }
    return 1;
}


void RunSortTest(Table* table) {
    printf("До сортировки:\n");
    PrintTable(table);
    BinaryInsertionSort(table);
    printf("\nПосле сортировки:\n");
    PrintTable(table);
    printf("\n");
}


void RunInteractiveSearch(Table* table) {
    char key[MAX_KEY_LEN];
    printf("Введите ключи, EOF для завершения\n");
    while (scanf("%63s", key) == 1) {
        int index = BinarySearch(table, key);
        if (index == -1) {
            printf("Ключ '%s' не найден.\n", key);
        } else {
            printf("Найдено: ключ='%s', данные=%d\n",
                   table->items[index].key, table->items[index].data);
        }
    }
}


int main(void) {

    Table original = {0};

    if (!ReadTable(&original)) {
        return 1;
    }

    printf("Таблица уже упорядочена:\n");
    Table sorted = original;
    qsort(sorted.items, sorted.count, sizeof(Record), CompareRecords);
    RunSortTest(&sorted);

    printf("Таблица в обратном порядке:\n");
    Table reversed = original;
    qsort(reversed.items, reversed.count, sizeof(Record), CompareRecordsReverse);
    RunSortTest(&reversed);

    printf("Неупорядоченная таблица:\n");
    Table unsorted = original;
    RunSortTest(&unsorted);

    printf("Поиск в таблице:\n");
    RunInteractiveSearch(&unsorted);

    return 0;
}
