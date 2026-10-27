#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

int **createMatrix(int n)
{
    int **M = (int **)malloc(n * sizeof(int *));
    if (!M) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        M[i] = (int *)calloc(n, sizeof(int));
        if (!M[i]) {
            fprintf(stderr, "Ошибка calloc\n");
            exit(1);
        }
    }
    return M;
}

void freeMatrix(int **M, int n)
{
    if (!M) return;
    for (int i = 0; i < n; i++)
        free(M[i]);
    free(M);
}

int **gen_undir(int n, int prob, int *outEdges)
{
    int **M = createMatrix(n);
    int edges = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (rand() % 100 < prob) {
                M[i][j] = 1;
                M[j][i] = 1;
                edges++;
            }
        }
    }
    if (outEdges) *outEdges = edges;
    return M;
}

static int edgeAt(int n, int **M, int i, int j)
{
    return (i < n && j < n) ? M[i][j] : 0;
}

int **graphUnion(int n1, int **M1, int n2, int **M2, int *outN)
{
    int N = n1 > n2 ? n1 : n2;
    int **R = createMatrix(N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            R[i][j] = edgeAt(n1, M1, i, j) || edgeAt(n2, M2, i, j);
    *outN = N;
    return R;
}

int **graphIntersection(int n1, int **M1, int n2, int **M2, int *outN)
{
    int N = n1 < n2 ? n1 : n2;
    int **R = createMatrix(N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            R[i][j] = M1[i][j] && M2[i][j];
    *outN = N;
    return R;
}

int **graphRingSum(int n1, int **M1, int n2, int **M2, int *outN)
{
    int N = n1 > n2 ? n1 : n2;
    int **R = createMatrix(N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            R[i][j] = edgeAt(n1, M1, i, j) ^ edgeAt(n2, M2, i, j);
    *outN = N;
    return R;
}

int countEdges(int n, int **M)
{
    int cnt = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (M[i][j]) cnt++;
    return cnt;
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

void printMatrixActive(const char *name, int n, int **M, const int *active)
{
    int cnt = 0;
    for (int i = 0; i < n; i++) if (active[i]) cnt++;

    printf("\nМатрица смежности %s (%d x %d):\n", name, cnt, cnt);
    if (cnt == 0) { printf("  граф пуст (все вершины изолированы и удалены)\n"); return; }

    printf("     ");
    for (int j = 0; j < n; j++) if (active[j]) printf("%3d", j);
    printf("\n");

    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        printf("%3d: ", i);
        for (int j = 0; j < n; j++)
            if (active[j]) printf("%3d", M[i][j]);
        printf("\n");
    }
}

// удаление изолированных вершин после кольцевой суммы
void printRingSum(int n, int **R)
{
    int *active = (int *)calloc(n, sizeof(int));
    if (!active) { fprintf(stderr, "Ошибка calloc\n"); exit(1); }

    int removed = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            if (R[i][j]) { active[i] = 1; break; }
        if (!active[i]) removed++;
    }

    if (removed) {
        printf("\nУдалены изолированные вершины:");
        for (int i = 0; i < n; i++) if (!active[i]) printf(" %d", i);
        printf("\n");
    }
    printMatrixActive("G1 (+) G2", n, R, active);
    free(active);
}

int readInt(const char *prompt, int lo, int hi)
{
    int x;
    char buf[100];
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) {
            fprintf(stderr, "Ошибка ввода\n");
            exit(1);
        }
        if (sscanf(buf, "%d", &x) == 1 && x >= lo && x <= hi)
            return x;
        printf("Некорректный ввод. Введите число от %d до %d.\n", lo, hi);
    }
}

int main(void)
{
    setlocale(LC_ALL, "Rus");
    srand((unsigned)time(NULL));

    int n1 = readInt("Введите количество вершин G1: ", 2, 1000);
    int p1 = readInt("Введите процент рёбер G1 (0..100): ", 0, 100);
    int n2 = readInt("Введите количество вершин G2: ", 2, 1000);
    int p2 = readInt("Введите процент рёбер G2 (0..100): ", 0, 100);

    int e1 = 0, e2 = 0;
    int **M1 = gen_undir(n1, p1, &e1);
    int **M2 = gen_undir(n2, p2, &e2);

    printf("\nГраф G1\n");
    printf("Вершин: %d, вероятность ребра: %d%%\n", n1, p1);
    printMatrix("M1", n1, M1);
    printf("Ребер в G1: %d\n", e1);

    printf("\nГраф G2\n");
    printf("Вершин: %d, вероятность ребра: %d%%\n", n2, p2);
    printMatrix("M2", n2, M2);
    printf("Ребер в G2: %d\n", e2);

    int choice;
    do {
        printf("\nОперации над графами\n");
        printf("1 - Объединение - G = G1 U G2\n");
        printf("2 - Пересечение - G = G1 /\\ G2\n");
        printf("3 - Кольцевая сумма - G = G1 (+) G2\n");
        printf("0 - Выход\n");
        choice = readInt("Выбор: ", 0, 3);

        switch (choice) {
        case 1: {
            int N;
            int **R = graphUnion(n1, M1, n2, M2, &N);
            printMatrix("G1 U G2", N, R);
            printf("Ребер в G1 U G2: %d\n", countEdges(N, R));
            freeMatrix(R, N);
            break;
        }
        case 2: {
            int N;
            int **R = graphIntersection(n1, M1, n2, M2, &N);
            printMatrix("G1 /\\ G2", N, R);
            printf("Ребер в G1 /\\ G2: %d\n", countEdges(N, R));
            freeMatrix(R, N);
            break;
        }
        case 3: {
            int N;
            int **R = graphRingSum(n1, M1, n2, M2, &N);
            printRingSum(N, R);
            printf("Ребер в G1 (+) G2: %d\n", countEdges(N, R));
            freeMatrix(R, N);
            break;
        }
        case 0:
            printf("Выход.\n");
            break;
        }
    } while (choice != 0);

    freeMatrix(M1, n1);
    freeMatrix(M2, n2);
    return 0;
}