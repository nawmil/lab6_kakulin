#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

typedef struct Node {
    int vertex;        
    struct Node *next;  
} Node;

static void appendNode(Node **head, int vertex)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(1);
    }
    newNode->vertex = vertex;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        Node *cur = *head;
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = newNode;
    }
}

int **gen_undir(int n, int prob)
{
    int **adj = (int **)malloc(n * sizeof(int *));
    if (!adj) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        adj[i] = (int *)calloc(n, sizeof(int));
        if (!adj[i]) {
            fprintf(stderr, "Ошибка calloc\n");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (rand() % 100 < prob) {
                adj[i][j] = 1;
                adj[j][i] = 1;
            }

    return adj;
}

void freeMatrix(int **M, int n)
{
    if (!M) return;
    for (int i = 0; i < n; i++)
        free(M[i]);
    free(M);
}

Node **buildAdjlist(int n, int **M)
{
    Node **adj = (Node **)malloc(n * sizeof(Node *));
    if (!adj) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        adj[i] = NULL;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            if (M[i][j]) {
                appendNode(&adj[i], j);
            }
        }
    }
    return adj;
}

void printAdjlist(const char *name, int n, Node **adj)
{
    printf("\nСписок смежности графа %s:\n", name);
    for (int i = 0; i < n; i++) {
        printf("  %d: ", i);
        if (adj[i] == NULL) {
            printf("- (Изолированный)\n");
        } else {
            Node *cur = adj[i];
            while (cur != NULL) {
                printf("%d", cur->vertex);
                if (cur->next != NULL) printf(" -> ");
                cur = cur->next;
            }
            printf("\n");
        }
    }
}

void freeAdjlist(int n, Node **adj)
{
    if (!adj) return;
    for (int i = 0; i < n; i++) {
        Node *cur = adj[i];
        while (cur != NULL) {
            Node *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(adj);
}

void printMatrix(const char *name, int n, int **M)
{
    printf("\nМатрица смежности %s (%d x %d):\n", name, n, n);

    printf("     ");
    for (int j = 0; j < n; j++) printf("%3d", j);
    printf("\n");

    for (int i = 0; i < n; i++) {
        printf("%3d: ", i);
        for (int j = 0; j < n; j++)
            printf("%3d", M[i][j]);
        printf("\n");
    }
}

int countEdges(int n, int **M)
{
    int edges = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (M[i][j]) edges++;
    return edges;
}

int readN(const char *prompt)
{
    int n;
    char buf[100];
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) {
            fprintf(stderr, "Ошибка ввода\n");
            exit(1);
        }
        if (sscanf(buf, "%d", &n) == 1 && n >= 2)
            return n;
        printf("Некорректный ввод. Введите целое число >= 2.\n");
    }
}

int readProb(const char *prompt)
{
    int p;
    char buf[100];
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) {
            fprintf(stderr, "Ошибка ввода\n");
            exit(1);
        }
        if (sscanf(buf, "%d", &p) == 1 && p >= 0 && p <= 100)
            return p;
        printf("Некорректный ввод. Введите целое число от 0 до 100.\n");
    }
}

int main(void)
{
    setlocale(LC_ALL, "Rus");
    srand((unsigned)time(NULL));

    printf("=== Граф G1 ===\n");
    int n1 = readN("Введите количество вершин G1: ");
    int p1 = readProb("Введите процент рёбер G1 (0..100): ");

    printf("\n=== Граф G2 ===\n");
    int n2 = readN("Введите количество вершин G2: ");
    int p2 = readProb("Введите процент рёбер G2 (0..100): ");

    int **M1 = gen_undir(n1, p1);
    printf("\nГраф G1\n");
    printf("Вершин: %d, вероятность ребра: %d%%\n", n1, p1);
    printMatrix("M1", n1, M1);
    printf("Ребер в G1: %d\n", countEdges(n1, M1));
    Node **adj1 = buildAdjlist(n1, M1);

    int **M2 = gen_undir(n2, p2);
    printf("\nГраф G2\n");
    printf("Вершин: %d, вероятность ребра: %d%%\n", n2, p2);
    printMatrix("M2", n2, M2);
    printf("Ребер в G2: %d\n", countEdges(n2, M2));
    Node **adj2 = buildAdjlist(n2, M2);

    printAdjlist("G1", n1, adj1);
    printAdjlist("G2", n2, adj2);

    freeAdjlist(n1, adj1);
    freeAdjlist(n2, adj2);
    freeMatrix(M1, n1);
    freeMatrix(M2, n2);

    return 0;
}