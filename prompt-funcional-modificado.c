#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 3 // Dimensão do sistema (exemplo: 3x3)

void printMatrix(double *A)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j <= N; j++)
        {
            printf("%6.2f ", A[i * (N + 1) + j]);
        }
        printf("\n");
    }
}

void gauss_elimination(double *augmented, int n, int rank, int size)
{
    for (int k = 0; k < n; k++)
    {
        // Broadcast da linha pivô
        MPI_Bcast(&augmented[k * (n + 1) + k], n + 1 - k, MPI_DOUBLE, k % size, MPI_COMM_WORLD);
        // Eliminação em paralelo
        for (int i = k + 1; i < n; i++)
        {
            if (i % size == rank)
            {
                double factor = augmented[i * (n + 1) + k] / augmented[k * (n + 1) + k];
                for (int j = k; j <= n; j++)
                {
                    augmented[i * (n + 1) + j] -= factor * augmented[k * (n + 1) + j];
                }
            }
        }
        // printf("rank = %d, augmented[rank * (n + 1)] = %lf, (n + 1) = %d, augmented = %lf\n", rank, augmented[rank * (n + 1)], (n + 1), augmented[0]);
        // printMatrix(augmented);
        // printf("\n");
        MPI_Gather(&augmented[rank * (n + 1)], (n + 1), MPI_DOUBLE, augmented, (n + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

void back_substitution(double *augmented, double *x, int n, int rank, int size)
{
    for (int i = n - 1; i >= 0; i--)
    {
        if (rank == 0)
        {
            // printMatrix(augmented);
            x[i] = augmented[i * (n + 1) + n];
            for (int j = i + 1; j < n; j++)
            {
                x[i] -= augmented[i * (n + 1) + j] * x[j];
            }
            x[i] /= augmented[i * (n + 1) + i];
        }
        MPI_Bcast(&x[i], 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    float tempo_inicial, tempo_final;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tempo_inicial = MPI_Wtime();

    if (size > N)
    {
        if (rank == 0)
        {
            printf("Erro: o número de processos é maior que o número de linhas da matriz.\n");
        }
        MPI_Finalize();
        return -1;
    }

    int n = 3; // Tamanho fixo para esta matriz
    double augmented[N][N + 1] = {
        {2, -1, 1, 3},
        {1, 3, 2, 12},
        {1, -1, 2, 5}};
    double x[3] = {0};

    // Transmissão inicial da matriz aumentada
    MPI_Bcast(augmented, n * (n + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);

    gauss_elimination(&augmented[0][0], n, rank, size);
    back_substitution(&augmented[0][0], x, n, rank, size);

    if (rank == 0)
    {
        printf("Resultado:\n");
        for (int i = 0; i < n; i++)
        {
            printf("x[%d] = %f\n", i, x[i]);
        }
        // Exibir tempo
        tempo_final = MPI_Wtime();
        printf("\nTempo de Execução: ");
        printf("%f\n", tempo_final - tempo_inicial);
    }

    MPI_Finalize();
    return 0;
}
