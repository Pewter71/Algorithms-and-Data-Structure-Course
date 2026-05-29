#include <stdio.h>

#define MAX_FILENAME_LENGTH 256
#define MAX_VERTICES 100

typedef struct {
    int matrix[MAX_VERTICES][MAX_VERTICES];
    int vertex_count;
} AdjacencyMatrix;

typedef struct {
    int data[MAX_VERTICES];
    int front;
    int back;
} Queue;


void InitQueue(Queue* queue) {
    queue->front = 0;
    queue->back = 0;
}

bool IsQueueEmpty(const Queue* queue) {
    return queue->front == queue->back;
}

void Enqueue(Queue* queue, int value) {
    queue->data[queue->back] = value;
    ++queue->back;
}

int Dequeue(Queue* queue) {
    int value = queue->data[queue->front];
    ++queue->front;
    return value;
}


void InitGraph(AdjacencyMatrix* graph, int vertex_count) {
    graph->vertex_count = vertex_count;
    for (int i = 0; i < vertex_count; ++i) {
        for (int j = 0; j < vertex_count; ++j) {
            graph->matrix[i][j] = 0;
        }
    }
}

void AddEdge(AdjacencyMatrix* graph, int from, int to) {
    graph->matrix[from][to] = 1;
}

void PrintMatrix(const AdjacencyMatrix* graph, const int* labels) {
    printf("Матрица смежности:\n");
    printf("    ");
    for (int j = 0; j < graph->vertex_count; ++j) {
        printf("%4d", labels[j]);
    }
    printf("\n");
    for (int i = 0; i < graph->vertex_count; ++i) {
        printf("%3d:", labels[i]);
        for (int j = 0; j < graph->vertex_count; ++j) {
            printf("%4d", graph->matrix[i][j]);
        }
        printf("\n");
    }
}

bool TopologicalSortKahn(const AdjacencyMatrix* graph, int* result, int* result_count) {
    int n = graph->vertex_count;
    int in_degree[MAX_VERTICES] = {0};

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (graph->matrix[i][j]) {
                ++in_degree[j];
            }
        }
    }

    Queue queue;
    InitQueue(&queue);

    for (int i = 0; i < n; ++i) {
        if (in_degree[i] == 0) {
            Enqueue(&queue, i);
        }
    }

    *result_count = 0;

    while (!IsQueueEmpty(&queue)) {
        int v = Dequeue(&queue);
        result[*result_count] = v;
        ++(*result_count);

        for (int u = 0; u < n; ++u) {
            if (graph->matrix[v][u]) {
                --in_degree[u];
                if (in_degree[u] == 0) {
                    Enqueue(&queue, u);
                }
            }
        }
    }

    return *result_count == n;
}

void PrintTopologicalOrder(const int* result, int result_count, const int* labels) {
    printf("алгоритм Кана:\n");
    for (int i = 0; i < result_count; ++i) {
        printf("%d", labels[result[i]]);
        if (i < result_count - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

bool ReadGraph(const char* filename, AdjacencyMatrix* graph, int* labels, int* vertex_count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Ошибка. Не удалось открыть файл '%s'.\n", filename);
        return false;
    }

    if (fscanf(file, "%d", vertex_count) != 1) {
        fprintf(stderr, "Ошибка. Не удалось прочитать количество вершин.\n");
        fclose(file);
        return false;
    }

    if (*vertex_count <= 0 || *vertex_count > MAX_VERTICES) {
        fprintf(stderr, "Ошибка. Некорректное количество вершин: %d.\n", *vertex_count);
        fclose(file);
        return false;
    }

    for (int i = 0; i < *vertex_count; ++i) {
        if (fscanf(file, "%d", &labels[i]) != 1) {
            fprintf(stderr, "Ошибка. Не удалось прочитать метку вершины %d.\n", i);
            fclose(file);
            return false;
        }
    }

    InitGraph(graph, *vertex_count);

    for (int i = 0; i < *vertex_count; ++i) {
        for (int j = 0; j < *vertex_count; ++j) {
            if (fscanf(file, "%d", &graph->matrix[i][j]) != 1) {
                fprintf(stderr, "Ошибка. Не удалось прочитать элемент матрицы.\n", i, j);
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);
    return true;
}

void RunFileMode(void) {
    char filename[MAX_FILENAME_LENGTH];

    printf("Введите имя файла\n");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "Ошибка. Не удалось прочитать имя файла.\n");
        return;
    }

    AdjacencyMatrix graph;
    int labels[MAX_VERTICES];
    int vertex_count = 0;

    if (!ReadGraph(filename, &graph, labels, &vertex_count)) {
        return;
    }

    PrintMatrix(&graph, labels);
    printf("\n");

    int result[MAX_VERTICES];
    int result_count = 0;

    bool is_acyclic = TopologicalSortKahn(&graph, result, &result_count);

    if (!is_acyclic) {
        printf("Граф содержит цикл, топологическая сортировка невозможна.\n");
        return;
    }

    PrintTopologicalOrder(result, result_count, labels);
}

int main(void) {
    RunFileMode();
    return 0;
}