#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void generateRandomMatrix(double *A, double *b, int n)
{
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    for (int i = 0; i < n; i++)
    {
        double row_sum = 0.0;
        for (int j = 0; j < n; j++)
        {
            A[i * n + j] = ((double)rand() / RAND_MAX) * 20.0 - 10.0; // Valores entre -10 e 10
            if (i != j)
            {
                row_sum += fabs(A[i * n + j]);
            }
        }
        A[i * n + i] = row_sum + ((double)rand() / RAND_MAX) * 5.0 + 1.0; // Diagonal dominante
        b[i] = ((double)rand() / RAND_MAX) * 20.0 - 10.0;                 // Vetor independente
    }
}

void gauss_elimination(double *A, double *b, int n, int rank, int size)
{
    for (int k = 0; k < n; k++)
    {
        // Broadcast da linha pivô
        for (int j = k; j < n; j++)
        {
            MPI_Bcast(&A[k * n + j], 1, MPI_DOUBLE, k % size, MPI_COMM_WORLD);
        }
        MPI_Bcast(&b[k], 1, MPI_DOUBLE, k % size, MPI_COMM_WORLD);

        // Eliminação em paralelo
        for (int i = k + 1; i < n; i++)
        {
            if (i % size == rank)
            {
                double factor = A[i * n + k] / A[k * n + k];
                for (int j = k; j < n; j++)
                {
                    A[i * n + j] -= factor * A[k * n + j];
                }
                b[i] -= factor * b[k];
            }
        }
    }
}

void back_substitution(double *A, double *b, double *x, int n, int rank, int size)
{
    for (int i = n - 1; i >= 0; i--)
    {
        if (rank == 0)
        {
            x[i] = b[i];
            for (int j = i + 1; j < n; j++)
            {
                x[i] -= A[i * n + j] * x[j];
            }
            x[i] /= A[i * n + i];
        }
        MPI_Bcast(&x[i], 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    double tempo_inicial, tempo_final;
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tempo_inicial = MPI_Wtime();

    int n = 100; // Tamanho da matriz
    double *A = malloc(n * n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    double *x = malloc(n * sizeof(double));

    if (rank == 0)
    {
        generateRandomMatrix(A, b, n); // Apenas o processo 0 gera a matriz
    }

    // Transmissão inicial da matriz e vetor para todos os processos
    MPI_Bcast(A, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    gauss_elimination(A, b, n, rank, size);
    back_substitution(A, b, x, n, rank, size);

    if (rank == 0)
    {
        printf("Resultado:\n");
        for (int i = 0; i < n; i++)
        {
            printf("x[%d] = %f\n", i, x[i]);
        }
        tempo_final = MPI_Wtime();
        printf("Tempo de Execução: %f", tempo_final - tempo_inicial);
    }

    free(A);
    free(b);
    free(x);

    MPI_Finalize();
    return 0;
}
