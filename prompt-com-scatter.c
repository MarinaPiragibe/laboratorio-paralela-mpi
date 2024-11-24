#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define N 320 // Dimensão do sistema

void printMatrix(double *A, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            printf("%6.2f ", A[i * (n + 1) + j]);
        }
        printf("\n");
    }
}

void generateRandomMatrix(double *matrix, int n)
{
    srand(time(NULL));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            double random_value = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
            matrix[i * (n + 1) + j] = random_value;
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
                row_sum += fabs(matrix[i * (n + 1) + j]);
            }
        }
        matrix[i * (n + 1) + i] = row_sum + ((double)rand() / RAND_MAX) * 5.0 + 1.0;
    }
}

void gauss_elimination(double *local_matrix, double *pivot_row, int local_rows,
                       int n, int rank, int size)
{
    double *temp_row = (double *)malloc((n + 1) * sizeof(double));

    for (int k = 0; k < n - 1; k++)
    {
        // Processo que contém a linha pivô
        int pivot_proc = k / local_rows;

        // Se este processo tem a linha pivô, envie-a para todos
        if (rank == pivot_proc)
        {
            int local_pivot_row = k % local_rows;
            for (int j = 0; j <= n; j++)
            {
                pivot_row[j] = local_matrix[local_pivot_row * (n + 1) + j];
            }
        }

        // Broadcast da linha pivô para todos os processos
        MPI_Bcast(pivot_row, n + 1, MPI_DOUBLE, pivot_proc, MPI_COMM_WORLD);

        // Calcula o início das linhas locais em relação à matriz global
        int start_row = rank * local_rows;

        // Eliminação nas linhas locais
        for (int i = 0; i < local_rows; i++)
        {
            int global_row = start_row + i;
            if (global_row > k)
            { // Só elimina linhas abaixo do pivô
                double factor = local_matrix[i * (n + 1) + k] / pivot_row[k];
                for (int j = k; j <= n; j++)
                {
                    local_matrix[i * (n + 1) + j] -= factor * pivot_row[j];
                }
            }
        }
    }

    free(temp_row);
}

void back_substitution(double *matrix, double *x, int n)
{
    for (int i = n - 1; i >= 0; i--)
    {
        x[i] = matrix[i * (n + 1) + n];
        for (int j = i + 1; j < n; j++)
        {
            x[i] -= matrix[i * (n + 1) + j] * x[j];
        }
        x[i] /= matrix[i * (n + 1) + i];
    }
}

int main(int argc, char *argv[])
{
    int rank, size;
    double tempo_inicial, tempo_final;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    tempo_inicial = MPI_Wtime();

    // Calcula quantas linhas cada processo receberá
    int local_rows = N / size;
    if (N % size != 0)
    {
        if (rank == 0)
        {
            printf("Erro: N deve ser divisível pelo número de processos\n");
        }
        MPI_Finalize();
        return -1;
    }

    // Aloca memória para as partes locais da matriz
    double *local_matrix = (double *)malloc(local_rows * (N + 1) * sizeof(double));
    double *full_matrix = NULL;
    double *pivot_row = (double *)malloc((N + 1) * sizeof(double));
    double *x = (double *)malloc(N * sizeof(double));

    // Processo 0 gera a matriz inicial
    if (rank == 0)
    {
        full_matrix = (double *)malloc(N * (N + 1) * sizeof(double));
        generateRandomMatrix(full_matrix, N);
        printf("Matriz original:\n");
        printMatrix(full_matrix, N);
        printf("\n");
    }

    // Distribui as linhas da matriz entre os processos
    MPI_Scatter(full_matrix, local_rows * (N + 1), MPI_DOUBLE,
                local_matrix, local_rows * (N + 1), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Realiza a eliminação de Gauss
    gauss_elimination(local_matrix, pivot_row, local_rows, N, rank, size);

    // Reúne a matriz processada
    MPI_Gather(local_matrix, local_rows * (N + 1), MPI_DOUBLE,
               full_matrix, local_rows * (N + 1), MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // Processo 0 realiza a substituição reversa
    if (rank == 0)
    {
        printf("Matriz após eliminação:\n");
        printMatrix(full_matrix, N);
        printf("\n");

        back_substitution(full_matrix, x, N);

        printf("Solução:\n");
        for (int i = 0; i < N; i++)
        {
            printf("x[%d] = %f\n", i, x[i]);
        }

        tempo_final = MPI_Wtime();
        printf("\nTempo de Execução: %f", tempo_final - tempo_inicial);

        free(full_matrix);
    }

    // Libera memória
    free(local_matrix);
    free(pivot_row);
    free(x);

    MPI_Finalize();
    return 0;
}