#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

int **createMatrix(int n)
{
    int **M = malloc(n * sizeof(int *));
    if (!M) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        M[i] = calloc(n, sizeof(int));
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


void printCartesianMatrix(const char *name, int n1, int n2, int **M)
{
    int N = n1 * n2;
    printf("\nМатрица смежности %s (%d x %d):\n", name, N, N);

    printf("        ");
    for (int u = 0; u < n1; u++) {
        for (int v = 0; v < n2; v++) {
            printf(" (%d,%d)", u, v);
        }
    }
    printf("\n");

    for (int u1 = 0; u1 < n1; u1++) {
        for (int v1 = 0; v1 < n2; v1++) {
            int idx1 = u1 * n2 + v1;
            printf("(%d,%d): ", u1, v1);
            for (int u2 = 0; u2 < n1; u2++) {
                for (int v2 = 0; v2 < n2; v2++) {
                    int idx2 = u2 * n2 + v2;
                    printf("%6d", M[idx1][idx2]);
                }
            }
            printf("\n");
        }
    }
}

int **cartesianProduct(int n1, int **M1, int n2, int **M2, int *newN)
{
    int N = n1 * n2;
    int **R = createMatrix(N);

    for (int u1 = 0; u1 < n1; u1++) {
        for (int v1 = 0; v1 < n2; v1++) {
            int idx1 = u1 * n2 + v1;

            /* Рёбра из G2 при фиксированной u1 */
            for (int v2 = 0; v2 < n2; v2++) {
                if (M2[v1][v2]) {
                    int idx2 = u1 * n2 + v2;
                    R[idx1][idx2] = 1;
                    R[idx2][idx1] = 1;
                }
            }

            /* Рёбра из G1 при фиксированной v1 */
            for (int u2 = 0; u2 < n1; u2++) {
                if (M1[u1][u2]) {
                    int idx2 = u2 * n2 + v1;
                    R[idx1][idx2] = 1;
                    R[idx2][idx1] = 1;
                }
            }
        }
    }

    *newN = N;
    return R;
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
    setlocale(LC_ALL, "");
    srand((unsigned)time(NULL));

    int n1 = readInt("Введите количество вершин графа G1: ", 2, 1000);
    int p1 = readInt("Введите процент рёбер G1 (0..100): ", 0, 100);

    int n2 = readInt("Введите количество вершин графа G2: ", 2, 1000);
    int p2 = readInt("Введите процент рёбер G2 (0..100): ", 0, 100);
    int e1 = 0, e2 = 0;
    int **M1 = gen_undir(n1, p1, &e1);
    int **M2 = gen_undir(n2, p2, &e2);

    printf("\nГраф G1: %d вершин, %d рёбер (prob = %d%%)\n", n1, e1, p1);
    printMatrix("G1", n1, M1);

    printf("\nГраф G2: %d вершин, %d рёбер (prob = %d%%)\n", n2, e2, p2);
    printMatrix("G2", n2, M2);

    int choice;
    do {
        printf("\nМеню\n");
        printf("1 - Декартово произведение G = G1 x G2\n");
        printf("0 - Выход\n");
        choice = readInt("Выбор: ", 0, 1);

        switch (choice) {
        case 1: {
            int newN = 0;
            int **R = cartesianProduct(n1, M1, n2, M2, &newN);

            printCartesianMatrix("G1 x G2", n1, n2, R);
            printf("\nВершин в G1 x G2: %d (= %d * %d)\n", newN, n1, n2);
            printf("Рёбер в G1 x G2: %d\n", countEdges(newN, R));

            freeMatrix(R, newN);
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