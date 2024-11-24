#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define N 100 // Dimensão do sistema (exemplo: 3x3)

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

void generateRandomMatrix(double *matrix, int n)
{
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    // Gera os coeficientes da matriz e o vetor de termos independentes
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            // Gera números aleatórios entre -10 e 10 com 2 casas decimais
            double random_value = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
            matrix[i * (n + 1) + j] = random_value;
        }
    }

    // Garante que a diagonal principal tenha valores dominantes
    // para melhorar a estabilidade numérica
    for (int i = 0; i < n; i++)
    {
        double row_sum = 0;
        for (int j = 0; j < n; j++)
        {
            if (i != j)
            {
                row_sum += fabs(matrix[i * (n + 1) + j]);
            }
        }
        // Faz o elemento da diagonal ser maior que a soma dos outros elementos
        matrix[i * (n + 1) + i] = row_sum + ((double)rand() / RAND_MAX) * 5.0 + 1.0;
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

    double augmented[N][N + 1];
    double x[N] = {0};
    if (rank == 0)
    {
        generateRandomMatrix(&augmented[0][0], N);
        printf("Matriz gerada:\n");
        printMatrix(&augmented[0][0]);
        printf("\n");
    }
    // double augmented[N][N + 1]= {
    //     {2, -1, 1, 3},
    //     {1, 3, 2, 12},
    //     {1, -1, 2, 5}
    // };
    // Transmissão inicial da matriz aumentada
    MPI_Bcast(augmented, N * (N + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);

    gauss_elimination(&augmented[0][0], N, rank, size);
    back_substitution(&augmented[0][0], x, N, rank, size);

    if (rank == 0)
    {
        printf("Resultado:\n");
        for (int i = 0; i < N; i++)
        {
            printf("x[%d] = %f\n", i, x[i]);
        }
        // Exibir tempo
        tempo_final = MPI_Wtime();
        printf("\nTempo de Execução: %f\n", tempo_final - tempo_inicial);
    }

    MPI_Finalize();
    return 0;
}
