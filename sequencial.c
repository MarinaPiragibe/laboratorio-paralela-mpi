#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h> // Para srand e rand

#define N 512

void swap_rows(double A[N][N + 1], int row1, int row2)
{
    for (int i = 0; i < N + 1; i++)
    {
        double temp = A[row1][i];
        A[row1][i] = A[row2][i];
        A[row2][i] = temp;
    }
}

void generateRandomMatrix(double A[N][N + 1], int n)
{
    srand(time(NULL));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            double random_value = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
            A[i][j] = random_value;
        }
    }

    // Garante diagonal dominante
    for (int i = 0; i < n; i++)
    {
        double row_sum = 0;
        for (int j = 0; j < n; j++)
        {
            if (i != j)
            {
                row_sum += fabs(A[i][j]);
            }
        }
        A[i][i] = row_sum + ((double)rand() / RAND_MAX) * 5.0 + 1.0;
    }
}

int main(int argc, char **argv)
{
    clock_t tempo_inicial, tempo_final;
    tempo_inicial = clock();

    double A[N][N + 1];
    generateRandomMatrix(A, N); // Gera matriz aleatória

    // Exibe a matriz gerada
    printf("Matriz aumentada gerada:\n");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j <= N; j++)
        {
            printf("%6.2f ", A[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // Eliminação de Gauss
    for (int k = 0; k < N; k++)
    {
        // Pivoteamento parcial
        int max_row = k;
        for (int i = k + 1; i < N; i++)
        {
            if (fabs(A[i][k]) > fabs(A[max_row][k]))
            {
                max_row = i;
            }
        }
        if (max_row != k)
        {
            swap_rows(A, k, max_row);
        }

        // Eliminação
        for (int i = k + 1; i < N; i++)
        {
            double factor = A[i][k] / A[k][k];
            for (int j = k; j < N + 1; j++)
            {
                A[i][j] -= factor * A[k][j];
            }
        }
    }

    // Substituição regressiva
    double x[N];
    for (int i = N - 1; i >= 0; i--)
    {
        x[i] = A[i][N];
        for (int j = i + 1; j < N; j++)
        {
            x[i] -= A[i][j] * x[j];
        }
        x[i] /= A[i][i];
    }

    // Solução
    printf("Solução:\n");
    for (int i = 0; i < N; i++)
    {
        printf("x[%d] = %6.2f\n", i, x[i]);
    }

    // Exibir tempo de execução
    tempo_final = clock();
    double tempo_execucao = ((double)(tempo_final - tempo_inicial)) / CLOCKS_PER_SEC;
    printf("\nTempo de Execução: %f segundos\n", tempo_execucao);

    return 0;
}
