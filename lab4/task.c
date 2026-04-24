#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_FILENAME_LENGTH 256
#define MAX_KEY_LENGTH 7
#define MAX_LINE_LENGTH 256


typedef struct Node {
    char key[MAX_KEY_LENGTH];
    double value;
    int height;
    struct Node *left_node;
    struct Node *right_node;
} Node;


int Height(const Node *node) {
    if (node == NULL) {
        return 0;
    }
    return node->height;
}


int Max(int a, int b) {
    return a > b ? a : b;
}


void UpdateHeight(Node *node) {
    node->height = 1 + Max(Height(node->left_node), Height(node->right_node));
}


int GetBalance(const Node *node) {
    if (node == NULL) {
        return 0;
    }
    return Height(node->left_node) - Height(node->right_node);
}


int IsValidKey(const char *key) {
    if (key == NULL || key[0] == '\0') {
        return 0;
    }
    size_t length = 0;
    for (size_t i = 0; key[i] != '\0'; ++i) {
        if (!isalpha((unsigned char)key[i])) {
            return 0;
        }
        ++length;
    }
    return length <= (size_t)(MAX_KEY_LENGTH - 1);
}


Node *CreateNode(const char *key, double value) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Ошибка. Память не выделена.\n");
        return new_node;
    }
    strncpy(new_node->key, key, MAX_KEY_LENGTH - 1);
    new_node->key[MAX_KEY_LENGTH - 1] = '\0';
    new_node->value = value;
    new_node->height = 1;
    new_node->left_node = NULL;
    new_node->right_node = NULL;
    return new_node;
}


Node *RotateRight(Node *y) {
    Node *x = y->left_node;
    Node *t = x->right_node;
    x->right_node = y;
    y->left_node = t;
    UpdateHeight(y);
    UpdateHeight(x);
    return x;
}


Node *RotateLeft(Node *x) {
    Node *y = x->right_node;
    Node *t = y->left_node;
    y->left_node = x;
    x->right_node = t;
    UpdateHeight(x);
    UpdateHeight(y);
    return y;
}


Node *Balance(Node *node) {
    UpdateHeight(node);
    int balance_factor = GetBalance(node);
    if (balance_factor > 1) {
        if (GetBalance(node->left_node) < 0) {
            node->left_node = RotateLeft(node->left_node);
        }
        return RotateRight(node);
    }
    if (balance_factor < -1) {
        if (GetBalance(node->right_node) > 0) {
            node->right_node = RotateRight(node->right_node);
        }
        return RotateLeft(node);
    }
    return node;
}


Node *Insert(Node *root, const char *key, double value, int *result) {
    if (root == NULL) {
        *result = 1;
        return CreateNode(key, value);
    }
    int cmp = strcmp(key, root->key);
    if (cmp < 0) {
        root->left_node = Insert(root->left_node, key, value, result);
    } else if (cmp > 0) {
        root->right_node = Insert(root->right_node, key, value, result);
    } else {
        *result = 0;
        return root;
    }
    return Balance(root);
}


Node *FindMin(Node *root) {
    while (root->left_node != NULL) {
        root = root->left_node;
    }
    return root;
}


Node *DeleteMin(Node *root) {
    if (root->left_node == NULL) {
        return root->right_node;
    }
    root->left_node = DeleteMin(root->left_node);
    return Balance(root);
}


Node *Delete(Node *root, const char *key, int *result) {
    if (root == NULL) {
        *result = 0;
        return NULL;
    }
    int cmp = strcmp(key, root->key);
    if (cmp < 0) {
        root->left_node = Delete(root->left_node, key, result);
    } else if (cmp > 0) {
        root->right_node = Delete(root->right_node, key, result);
    } else {
        *result = 1;
        Node *left_child = root->left_node;
        Node *right_child = root->right_node;
        free(root);
        if (right_child == NULL) {
            return left_child;
        }
        Node *min_node = FindMin(right_child);
        min_node->right_node = DeleteMin(right_child);
        min_node->left_node = left_child;
        return Balance(min_node);
    }
    return Balance(root);
}


Node *Find(const Node *root, const char *key) {
    if (root == NULL) {
        return NULL;
    }
    int cmp = strcmp(key, root->key);
    if (cmp < 0) {
        return Find(root->left_node, key);
    } else if (cmp > 0) {
        return Find(root->right_node, key);
    }
    return root;
}


void PrintTree(const Node *root, int depth, FILE *out) {
    if (root == NULL) {
        return;
    }
    PrintTree(root->right_node, depth + 1, out);
    for (int i = 0; i < depth; ++i) {
        fprintf(out, "|");
    }
    fprintf(out, "[%s: %.4f]\n", root->key, root->value);
    PrintTree(root->left_node, depth + 1, out);
}


void FreeTree(Node *root) {
    if (root != NULL) {
        FreeTree(root->left_node);
        FreeTree(root->right_node);
        free(root);
    }
}


void ProcessLine(Node **root, const char *line, FILE *out) {
    int op = 0;
    char raw_key[MAX_LINE_LENGTH];
    double value = 0.0;

    if (sscanf(line, "%d", &op) != 1) {
        fprintf(out, "Ошибка. Некорректный формат строки.\n");
        return;
    }

    if (op == 1) {
        if (sscanf(line, "%d %255s %lf", &op, raw_key, &value) != 3) {
            fprintf(out, "Ошибка. Команда 1 требует ключ и числовое значение.\n");
            return;
        }
        if (!IsValidKey(raw_key)) {
            fprintf(out, "Ошибка. Ключ должен состоять только из латинских букв (не более 6).\n");
            return;
        }
        int result = 0;
        *root = Insert(*root, raw_key, value, &result);
        if (result == 1) {
            fprintf(out, "Элемент добавлен.\n");
        } else {
            fprintf(out, "Ошибка. Элемент с таким ключом уже существует.\n");
        }
    } else if (op == 2) {
        if (sscanf(line, "%d %255s", &op, raw_key) != 2) {
            fprintf(out, "Ошибка. Введите ключ.\n");
            return;
        }
        if (!IsValidKey(raw_key)) {
            fprintf(out, "Ошибка. Некорректный ключ.\n");
            return;
        }
        int result = 0;
        *root = Delete(*root, raw_key, &result);
        if (result == 1) {
            fprintf(out, "Элемент удалён.\n");
        } else {
            fprintf(out, "Ошибка. Элемент с таким ключом не найден.\n");
        }
    } else if (op == 3) {
        if (*root == NULL) {
            fprintf(out, "Дерево пустое.\n");
        } else {
            PrintTree(*root, 0, out);
        }
    } else if (op == 4) {
        if (sscanf(line, "%d %255s", &op, raw_key) != 2) {
            fprintf(out, "Ошибка. Введите ключ.\n");
            return;
        }
        if (!IsValidKey(raw_key)) {
            fprintf(out, "Ошибка. Некорректный ключ.\n");
            return;
        }
        const Node *found = Find(*root, raw_key);
        if (found != NULL) {
            fprintf(out, "%.4f\n", found->value);
        } else {
            fprintf(out, "Ошибка. Элемент с таким ключом не найден.\n");
        }
    } else {
        fprintf(out, "Ошибка. Введите число из списка.\n");
    }
}


void RunInteractive(Node **root) {
    char line[MAX_LINE_LENGTH];
    while (1) {
        printf("\nКоманды:\n"
               "0 - выход\n"
               "1 - добавить узел\n"
               "2 - удалить узел\n"
               "3 - вывести дерево\n"
               "4 - найти значение\n");

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }


        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0') {
            continue;
        }

        int choice = 0;
        if (sscanf(line, "%d", &choice) != 1) {
            printf("Ошибка. Некорректный ввод.\n");
            continue;
        }

        if (choice == 0) {
            printf("Завершение программы.\n");
            break;
        }

        ProcessLine(root, line, stdout);
    }
}


void RunFileMode(Node **root) {
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];

    printf("Введите имя входного файла\n");
    if (scanf("%255s", input_filename) != 1) {
        fprintf(stderr, "Ошибка. Не удалось прочитать имя файла.\n");
        return;
    }

    printf("Введите имя выходного файла\n");
    if (scanf("%255s", output_filename) != 1) {
        fprintf(stderr, "Ошибка. Не удалось прочитать имя файла.\n");
        return;
    }

    FILE *in = fopen(input_filename, "r");
    if (in == NULL) {
        fprintf(stderr, "Ошибка. Не удалось открыть файл.\n");
        return;
    }

    FILE *out = fopen(output_filename, "w");
    if (out == NULL) {
        fprintf(stderr, "Ошибка. Не удалось создать файл.\n");
        fclose(in);
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), in) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (line[0] == '\0') {
            continue;
        }
        fprintf(out, "%s\n", line);
        ProcessLine(root, line, out);
        fprintf(out, "\n");
    }

    fclose(in);
    fclose(out);
    printf("Результаты записаны в файл '%s'.\n", output_filename);
}


int main() {
    Node *root = NULL;

    printf("Выберите режим работы:\n"
           "1 - интерактивный режим\n"
           "2 - работа с файлами\n");

    char line[MAX_LINE_LENGTH];
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 1;
    }

    int mode = 0;
    if (sscanf(line, "%d", &mode) != 1) {
        fprintf(stderr, "Ошибка. некорректный ввод.\n");
        return 1;
    }

    if (mode == 1) {
        RunInteractive(&root);
    } else if (mode == 2) {
        RunFileMode(&root);
    } else {
        fprintf(stderr, "Ошибка. некорректный режим.\n");
        FreeTree(root);
        return 1;
    }

    FreeTree(root);
    return 0;
}
