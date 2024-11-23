#include <stdio.h>
#include <math.h>

#define N 3

void swap_rows(double A[N][N+1], int row1, int row2) {
    for (int i = 0; i < N + 1; i++) {
        double temp = A[row1][i];
        A[row1][i] = A[row2][i];
        A[row2][i] = temp;
    }
}

int main() {
    double A[N][N+1] = {
        {2, -1, 1, 3},
        {1, 3, 2, 12},
        {1, -1, 2, 5}
    };

    // Eliminação de Gauss
    for (int k = 0; k < N; k++) {
        // Pivoteamento parcial (sempre pegar p maior pivo)
        int max_row = k;
        for (int i = k + 1; i < N; i++) {
            if (fabs(A[i][k]) > fabs(A[max_row][k])) {
                max_row = i;
            }
        }
        if (max_row != k) {
            swap_rows(A, k, max_row);
        }

        // Eliminação
        for (int i = k + 1; i < N; i++) {
            double factor = A[i][k] / A[k][k]; // fator de multiplicação
            for (int j = k; j < N + 1; j++) {
                A[i][j] -= factor * A[k][j];
            }
        }
    }

    // Substituição regressiva
    double x[N];
    for (int i = N - 1; i >= 0; i--) {
        x[i] = A[i][N];
        for (int j = i + 1; j < N; j++) {
            x[i] -= A[i][j] * x[j];
        }
        x[i] /= A[i][i];
    }

    // Solução
    printf("Solução:\n");
    for (int i = 0; i < N; i++) {
        printf("x[%d] = %6.2f\n", i, x[i]);
    }

    return 0;
}
