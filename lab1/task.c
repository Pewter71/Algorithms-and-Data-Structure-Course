#include <stdio.h>
#include <stdlib.h>


typedef struct Node {
    int data;
    struct Node *right_node;
    struct Node *left_node;
} Node;

Node *CreateNode(int value) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Ошибка. Память не выделена.\n");
        return newNode;
    }
    newNode->data = value;
    newNode->right_node = NULL;
    newNode->left_node = NULL;
    return newNode;
}

void InsertNode(int value, Node **root) {
    Node *newNode = CreateNode(value);
    if (*root == NULL) {
        *root = newNode;
        printf("Элемент добавлен.\n");
    } else {
        Node *current = *root;
        while (current != NULL) {
            if (current->data < value) {
                if (current->right_node == NULL) {
                    current->right_node = newNode;
                    printf("Элемент добавлен.\n");
                    break;
                } else {
                    current = current->right_node;
                }
            } else if (current->data > value) {
                if (current->left_node == NULL) {
                    current->left_node = newNode;
                    printf("Элемент добавлен.\n");
                    break;
                } else {
                    current = current->left_node;
                }
            } else {
                printf("Ошибка. Данный элемент уже содержится в дереве.\n");
                free(newNode);
                break;
            }
        }
    }
}

void PrintTree(Node *root, int depth) {
    if (root == NULL) {
        printf("Ошибка. Дерево пустое.\n");
    } else {
        if (root->right_node != NULL) {
            PrintTree(root->right_node, depth + 1);
        }
        for (int i = 0; i < depth; ++i) {
            printf("|");
        }
        printf("%d\n", root->data);
        if (root->left_node != NULL) {
            PrintTree(root->left_node, depth + 1);
        }
    }
}

void FreeTree(Node *root) {
    if (root != NULL) {
        FreeTree(root->right_node);
        FreeTree(root->left_node);
        free(root);
    }
}

Node *FindMin(Node* root) {
    while (root->left_node != NULL) {
        root = root->left_node;
    }
    return root;
}

Node *DeleteNode(Node* root, int value) {
    if (root == NULL) {
        return root;
    }
    if (value < root->data) {
        root->left_node = DeleteNode(root->left_node, value);
    } else if (value > root->data) {
        root->right_node = DeleteNode(root->right_node, value);
    } else {
        if (root->left_node == NULL) {
            Node* temp = root->right_node;
            free(root);
            return temp;
        } else if (root->right_node == NULL) {
            Node* temp = root->left_node;
            free(root);
            return temp;
        }
        Node* temp = FindMin(root->right_node);
        root->data = temp->data;
        root->right_node = DeleteNode(root->right_node, temp->data);
    }
    return root;
}

void Task(Node* root, int depth, int *max_depth, int *output) {
    if (root != NULL) {
        if (root->right_node != NULL || root->left_node != NULL) {
            if (depth > *max_depth) {
                *max_depth = depth;
                *output = root->data;
            }
        }
        Task(root->right_node, depth + 1, max_depth, output);
        Task(root->left_node, depth + 1, max_depth, output);
    }
}

int main() {
    Node *root = NULL;
    while(1) {
        printf("Введите цифру:\n"
               "0 - завершение программы.\n"
               "1 - добавление нового узла.\n"
               "2 - вывести дерево.\n"
               "3 - удаление узла.\n"
               "4 - вывести значение нетерминальной вершины с максимальной глубиной.\n"
               "5 - очистить дерево.\n");
        int input = -1;
        if (scanf("%d", &input) != 1) {
            while (getchar() != '\n') {}
            printf("Ошибка. Введите корректную цифру.\n");
            continue;
        }
        if (input == 0) {
            printf("Завершение программы.\n");
            break;
        }
        else if (input == 1) {
            printf("Введите значение узла.\n");
            int data = 0;
            if (scanf("%d", &data) != 1) {
                while (getchar() != '\n') {}
                printf("Ошибка. Введите число.\n");
                continue;
            }
            InsertNode(data, &root);
        }
        else if (input == 2) {
            PrintTree(root, 0);
        }
        else if (input == 3) {
            printf("Введите значение удаляемого узла.\n");
            int data = 0;
            if (scanf("%d", &data) != 1) {
                while (getchar() != '\n') {}
                printf("Ошибка. Введите число.\n");
                continue;
            }
            root = DeleteNode(root, data);
        }
        else if (input == 4) {
            if (root == NULL) {
                printf("Ошибка. Дерево пустое.\n");
                continue;
            }
            int max_depth = -1;
            int output = 0;
            Task(root, 0, &max_depth, &output);
            if (max_depth == -1) {
                printf("Ошибка. Корень является листом.\n");
                continue;
            }
            printf("%d\n", output);
        }
        else if (input == 5) {
            FreeTree(root);
            root = NULL;
            printf("Все узлы были удалены.\n");
        }
        else {
            printf("Введите цифру из списка.\n");
            continue;
        }
    }
    FreeTree(root);
    return 0;
}
