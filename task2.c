#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

// СПИСКИ СМЕЖНОСТИ

typedef struct Node {
    int vertex;
    struct Node *next;
} Node;

static void appendNode(Node **head, int vertex)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) { fprintf(stderr, "Ошибка malloc\n"); exit(1); }
    newNode->vertex = vertex;
    newNode->next = NULL;

    if (*head == NULL) *head = newNode;
    else {
        Node *cur = *head;
        while (cur->next) cur = cur->next;
        cur->next = newNode;
    }
}

static void removeNode(Node **head, int vertex)
{
    Node *cur = *head;
    Node *prev = NULL;
    while (cur) {
        if (cur->vertex == vertex) {
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static int hasEdge(Node **adj, int u, int v)
{
    for (Node *cur = adj[u]; cur; cur = cur->next)
        if (cur->vertex == v) return 1;
    return 0;
}

static void addEdge(Node **adj, int u, int v)
{
    if (hasEdge(adj, u, v)) return;
    if (u == v) {  // петля
        appendNode(&adj[u], u);
        return;
    }
    appendNode(&adj[u], v);
    appendNode(&adj[v], u);
}

Node **createAdjlist(int n)
{
    Node **adj = (Node **)malloc(n * sizeof(Node *));
    if (!adj) { fprintf(stderr, "Ошибка malloc\n"); exit(1); }
    for (int i = 0; i < n; i++) adj[i] = NULL;
    return adj;
}

void freeAdjlist(int n, Node **adj)
{
    if (!adj) return;
    for (int i = 0; i < n; i++) {
        Node *cur = adj[i];
        while (cur) { Node *t = cur; cur = cur->next; free(t); }
    }
    free(adj);
}

Node **gen_undir(int n, int prob, int *outEdges)
{
    Node **adj = createAdjlist(n);
    int edges = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (rand() % 100 < prob) { addEdge(adj, i, j); edges++; }
    if (outEdges) *outEdges = edges;
    return adj;
}

void printAdjlist(const char *name, int n, Node **adj, const int *active)
{
    printf("\nСписок смежности графа %s:\n", name);
    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        printf("  %d: ", i);
        Node *cur = adj[i];
        int hasActiveNeighbors = 0;
        for (Node *c = cur; c; c = c->next) {
            if (active[c->vertex]) { hasActiveNeighbors = 1; break; }
        }
        if (!hasActiveNeighbors) { printf("- (Изолированный)\n"); continue; }
        
        int first = 1;
        for (Node *c = cur; c; c = c->next) {
            if (active[c->vertex]) {
                if (!first) printf(" -> ");
                printf("%d", c->vertex);
                first = 0;
            }
        }
        printf("\n");
    }
}

// МАТРИЦА СМЕЖНОСТИ

int **createMatrix(int n)
{
    int **M = (int **)malloc(n * sizeof(int *));
    if (!M) { fprintf(stderr, "Ошибка malloc\n"); exit(1); }
    for (int i = 0; i < n; i++) {
        M[i] = (int *)calloc(n, sizeof(int));
        if (!M[i]) { fprintf(stderr, "Ошибка calloc\n"); exit(1); }
    }
    return M;
}

void freeMatrix(int **M, int n)
{
    if (!M) return;
    for (int i = 0; i < n; i++) free(M[i]);
    free(M);
}

int **resizeMatrix(int **M, int oldN, int newN)
{
    int **newM = createMatrix(newN);
    for (int i = 0; i < oldN; i++) {
        for (int j = 0; j < oldN; j++) {
            newM[i][j] = M[i][j];
        }
    }
    freeMatrix(M, oldN);
    return newM;
}

int **adjToMatrix(int n, Node **adj, const int *active)
{
    int **M = createMatrix(n);
    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        for (Node *cur = adj[i]; cur; cur = cur->next) {
            if (active[cur->vertex])
                M[i][cur->vertex] = 1;
        }
    }
    return M;
}

Node **matrixToAdjlist(int n, int **M, const int *active)
{
    Node **adj = createAdjlist(n);
    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        if (M[i][i]) appendNode(&adj[i], i);
        for (int j = i + 1; j < n; j++) {
            if (active[j] && M[i][j]) {
                appendNode(&adj[i], j);
                appendNode(&adj[j], i);
            }
        }
    }
    return adj;
}

void printMatrixDirect(const char *name, int n, int **M, const int *active)
{
    int cnt = 0;
    for (int i = 0; i < n; i++) if (active[i]) cnt++;
    printf("\nМатрица смежности %s (%d x %d):\n", name, cnt, cnt);
    printf("     ");
    for (int j = 0; j < n; j++) {
        if (active[j]) printf("%3d", j);
    }
    printf("\n");
    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        printf("%3d: ", i);
        for (int j = 0; j < n; j++) {
            if (active[j]) printf("%3d", M[i][j]);
        }
        printf("\n");
    }
}

void printMatrixFromAdj(const char *name, int n, Node **adj, const int *active)
{
    int **M = adjToMatrix(n, adj, active);
    printMatrixDirect(name, n, M, active);
    freeMatrix(M, n);
}

void printAdjFromMatrix(const char *name, int n, int **M, const int *active)
{
    Node **adj = matrixToAdjlist(n, M, active);
    printAdjlist(name, n, adj, active);
    freeAdjlist(n, adj);
}

int **gen_undir_matrix(int n, int prob, int *outEdges)
{
    int **M = createMatrix(n);
    int edges = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (rand() % 100 < prob) {
                M[i][j] = 1; M[j][i] = 1; edges++;
            }
    if (outEdges) *outEdges = edges;
    return M;
}

// ОПЕРАЦИИ НАД МАТРИЦЕЙ

void mergeVerticesMatrix(int n, int **M, int *active, int u, int v)
{
    if (u == v || !active[u] || !active[v]) return;

    int Loop = M[u][v] || M[v][v];   // смежные вершины -> петля

    for (int j = 0; j < n; j++) {
        if (j == u || j == v || !active[j]) continue;
        if (M[v][j]) {
            M[u][j] = 1;
            M[j][u] = 1;
        }
    }
    
    for (int j = 0; j < n; j++) {
        M[v][j] = 0;
        M[j][v] = 0;
    }
    if (Loop) M[u][u] = 1;
    active[v] = 0;
}

void contractEdgeMatrix(int n, int **M, int *active, int u, int v)
{
    if (u == v || !active[u] || !active[v]) {
        printf("Некорректные вершины.\n");
        return;
    }
    if (!M[u][v]) {
        printf("Ребра между %d и %d нет — стягивание невозможно.\n", u, v);
        return;
    }
    M[u][v] = 0;   // удаляем ребро до слияния, петли нет
    M[v][u] = 0;
    mergeVerticesMatrix(n, M, active, u, v);
}

int **splitVertexMatrix(int *n_ptr, int **M, int **active_ptr, int v, int *newN)
{
    int n = *n_ptr;
    int N = n + 1;
    
    int **R = resizeMatrix(M, n, N);
    int *active = (int *)realloc(*active_ptr, N * sizeof(int));
    if (!active) { fprintf(stderr, "Ошибка realloc\n"); exit(1); }
    
    active[N - 1] = 1;
    int w = N - 1;   // клон вершины v с теми же связями

    for (int i = 0; i < n; i++) {
        if (!active[i] || i == v) continue;
        if (R[v][i]) {
            R[w][i] = 1;
            R[i][w] = 1;
        }
    }
    if (R[v][v]) R[w][w] = 1;

    R[v][N - 1] = 1;
    R[N - 1][v] = 1;
    
    *n_ptr = N;
    *active_ptr = active;
    *newN = N;
    return R;
}

// ОПЕРАЦИИ НАД СПИСКАМИ

void mergeVerticesList(int n, Node **adj, int *active, int u, int v)
{
    if (u == v || !active[u] || !active[v]) return;

    int needLoop = hasEdge(adj, u, v) || hasEdge(adj, v, v);   // смежные вершины -> петля

    for (Node *cur = adj[v]; cur; cur = cur->next) {
        int neighbor = cur->vertex;
        if (neighbor == u || neighbor == v || !active[neighbor]) continue;
        if (!hasEdge(adj, u, neighbor)) {
            addEdge(adj, u, neighbor);
        }
        removeNode(&adj[neighbor], v);
    }
    
    Node *cur = adj[v];
    while (cur) {
        Node *t = cur;
        cur = cur->next;
        free(t);
    }
    adj[v] = NULL;
    
    for (int i = 0; i < n; i++) {
        if (active[i] && i != v) {
            removeNode(&adj[i], v);
        }
    }
    if (needLoop) addEdge(adj, u, u);
    active[v] = 0;
}

void contractEdgeList(int n, Node **adj, int *active, int u, int v)
{
    if (u == v || !active[u] || !active[v]) {
        printf("Некорректные вершины.\n");
        return;
    }
    if (!hasEdge(adj, u, v)) {
        printf("Ребра между %d и %d нет — стягивание невозможно.\n", u, v);
        return;
    }
    removeNode(&adj[u], v);   // удаляем ребро до слияния, петли нет
    removeNode(&adj[v], u);
    mergeVerticesList(n, adj, active, u, v);
}

Node **splitVertexList(int *n_ptr, Node **adj_ptr, int **active_ptr, int v, int *newN)
{
    int n = *n_ptr;
    int N = n + 1;
    int w = N - 1;   // клон вершины v с теми же связями

    Node **adj = (Node **)realloc(adj_ptr, N * sizeof(Node *));
    if (!adj) { fprintf(stderr, "Ошибка realloc\n"); exit(1); }
    adj[w] = NULL;

    int *active = (int *)realloc(*active_ptr, N * sizeof(int));
    if (!active) { fprintf(stderr, "Ошибка realloc\n"); exit(1); }
    active[w] = 1;

    int hasLoop = 0;
    for (Node *cur = adj[v]; cur; cur = cur->next) {
        int neighbor = cur->vertex;
        if (!active[neighbor]) continue;
        if (neighbor == v) { hasLoop = 1; continue; }
        addEdge(adj, w, neighbor);
    }
    if (hasLoop) addEdge(adj, w, w);

    addEdge(adj, v, w);

    *n_ptr = N;
    *active_ptr = active;
    *newN = N;
    return adj;
}

int readInt(const char *prompt, int lo, int hi)
{
    int x;
    char buf[100];
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) {
            fprintf(stderr, "Ошибка ввода\n"); exit(1);
        }
        if (sscanf(buf, "%d", &x) == 1 && x >= lo && x <= hi) return x;
        printf("Некорректный ввод. Введите число от %d до %d.\n", lo, hi);
    }
}

void MatrixMode(void)
{
    int n = readInt("Введите количество вершин графа: ", 2, 1000);
    int p = readInt("Введите процент рёбер (0..100): ", 0, 100);

    int e = 0;
    int **M = gen_undir_matrix(n, p, &e);
    
    int *active = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) active[i] = 1;

    printf("\nСгенерирован граф (матричная форма): %d вершин, %d рёбер (prob = %d%%)\n",
           n, e, p);
    printMatrixDirect("исходной", n, M, active);
    printAdjFromMatrix("исходного", n, M, active);

    int choice;
    do {
        printf("\nМеню (матричная форма)\n");
        printf("1 - Отождествление вершин\n");
        printf("2 - Стягивание ребра\n");
        printf("3 - Расщепление вершины\n");
        printf("0 - Выход в главное меню\n");
        choice = readInt("Выбор: ", 0, 3);

        switch (choice) {
        case 1: {
            int u = readInt("Введите первую вершину: ", 0, n - 1);
            int v = readInt("Введите вторую вершину: ", 0, n - 1);
            if (!active[u] || !active[v]) { printf("Одна из вершин уже удалена/неактивна!\n"); break; }
            if (u == v) { printf("Вершины должны быть разными!\n"); break; }
            mergeVerticesMatrix(n, M, active, u, v);
            printMatrixDirect("после отождествления", n, M, active);
            printAdjFromMatrix("после отождествления", n, M, active);
            break;
        }
        case 2: {
            int u = readInt("Введите первую вершину ребра: ", 0, n - 1);
            int v = readInt("Введите вторую вершину ребра: ", 0, n - 1);
            if (!active[u] || !active[v]) { printf("Одна из вершин уже удалена/неактивна!\n"); break; }
            if (u == v) { printf("Вершины должны быть разными!\n"); break; }
            contractEdgeMatrix(n, M, active, u, v);
            printMatrixDirect("после стягивания ребра", n, M, active);
            printAdjFromMatrix("после стягивания ребра", n, M, active);
            break;
        }
        case 3: {
            int v = readInt("Введите расщепляемую вершину: ", 0, n - 1);
            if (!active[v]) { printf("Вершина неактивна!\n"); break; }

            printf("Соседи вершины %d: ", v);
            int cnt = 0;
            for (int j = 0; j < n; j++)
                if (active[j] && j != v && M[v][j]) { printf("%d ", j); cnt++; }
            if (cnt == 0) printf("нет");
            printf("\n");

            int newN;
            M = splitVertexMatrix(&n, M, &active, v, &newN);
            printf("Создана вершина-клон %d с теми же связями и ребро %d - %d.\n",
                   n - 1, v, n - 1);

            printMatrixDirect("после расщепления", n, M, active);
            printAdjFromMatrix("после расщепления", n, M, active);
            break;
        }
        case 0:
            break;
        }
    } while (choice != 0);

    freeMatrix(M, n);
    free(active);
}

void ListMode(void)
{
    int n = readInt("Введите количество вершин графа: ", 2, 1000);
    int p = readInt("Введите процент рёбер (0..100): ", 0, 100);

    int e = 0;
    Node **adj = gen_undir(n, p, &e);
    
    int *active = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) active[i] = 1;

    printf("\nСгенерирован граф (списки смежности): %d вершин, %d рёбер (prob = %d%%)\n",
           n, e, p);
    printAdjlist("исходного", n, adj, active);
    printMatrixFromAdj("исходной", n, adj, active);

    int choice;
    do {
        printf("\nМеню (списковая форма)\n");
        printf("1 - Отождествление вершин\n");
        printf("2 - Стягивание ребра\n");
        printf("3 - Расщепление вершины\n");
        printf("0 - Выход в главное меню\n");
        choice = readInt("Выбор: ", 0, 3);

        switch (choice) {
        case 1: {
            int u = readInt("Введите первую вершину: ", 0, n - 1);
            int v = readInt("Введите вторую вершину: ", 0, n - 1);
            if (!active[u] || !active[v]) { printf("Одна из вершин неактивна!\n"); break; }
            if (u == v) { printf("Вершины должны быть разными!\n"); break; }
            mergeVerticesList(n, adj, active, u, v);
            printAdjlist("после отождествления", n, adj, active);
            printMatrixFromAdj("после отождествления", n, adj, active);
            break;
        }
        case 2: {
            int u = readInt("Введите первую вершину ребра: ", 0, n - 1);
            int v = readInt("Введите вторую вершину ребра: ", 0, n - 1);
            if (!active[u] || !active[v]) { printf("Одна из вершин неактивна!\n"); break; }
            if (u == v) { printf("Вершины должны быть разными!\n"); break; }
            contractEdgeList(n, adj, active, u, v);
            printAdjlist("после стягивания ребра", n, adj, active);
            printMatrixFromAdj("после стягивания ребра", n, adj, active);
            break;
        }
        case 3: {
            int v = readInt("Введите расщепляемую вершину: ", 0, n - 1);
            if (!active[v]) { printf("Вершина неактивна!\n"); break; }

            printf("Соседи вершины %d: ", v);
            int cnt = 0;
            for (Node *cur = adj[v]; cur; cur = cur->next)
                if (active[cur->vertex] && cur->vertex != v) { printf("%d ", cur->vertex); cnt++; }
            if (cnt == 0) printf("нет");
            printf("\n");

            int newN;
            adj = splitVertexList(&n, adj, &active, v, &newN);
            printf("Создана вершина-клон %d с теми же связями и ребро %d - %d.\n",
                   n - 1, v, n - 1);

            printAdjlist("после расщепления", n, adj, active);
            printMatrixFromAdj("после расщепления", n, adj, active);
            break;
        }
        case 0:
            break;
        }
    } while (choice != 0);

    freeAdjlist(n, adj);
    free(active);
}

int main(void)
{
    setlocale(LC_ALL, "Rus");
    srand((unsigned)time(NULL));

    int choice;
    do {
        printf("\nГлавное меню\n");
        printf("1 - Матричная форма представления графа\n");
        printf("2 - Списки смежности\n");
        printf("0 - Выход\n");
        choice = readInt("Выбор: ", 0, 2);

        switch (choice) {
        case 1: MatrixMode(); break;
        case 2: ListMode(); break;
        case 0: printf("Выход.\n"); break;
        }
    } while (choice != 0);

    return 0;
}