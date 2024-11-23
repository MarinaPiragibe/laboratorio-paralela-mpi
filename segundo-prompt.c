#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 3 // Dimensão do sistema (exemplo: 3x3)

// Função para realizar troca de linhas
void swap_rows(double A[N][N+1], int row1, int row2) {
    for (int i = 0; i < N + 1; i++) {
        double temp = A[row1][i];
        A[row1][i] = A[row2][i];
        A[row2][i] = temp;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    float tempo_inicial, tempo_final;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tempo_inicial = MPI_Wtime();

    double A[N][N+1] = {
        {2, -1, 1, 3},
        {1, 3, 2, 12},
        {1, -1, 2, 5}
    };

    if (rank == 0) {
        printf("Matriz inicial (A|b):\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N + 1; j++) {
                printf("%6.2f ", A[i][j]);
            }
            printf("\n");
        }
    }

    // Broadcast inicial da matriz para todos os processos
    MPI_Bcast(&A[0][0], N * (N + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Etapa de eliminação
    for (int k = 0; k < N; k++) {
        // Processo 0 realiza o pivoteamento e transmite a linha pivô
        if (rank == 0) {
            // Encontrar a linha com maior elemento na coluna
            int max_row = k;
            for (int i = k + 1; i < N; i++) {
                if (fabs(A[i][k]) > fabs(A[max_row][k])) {
                    max_row = i;
                }
            }
            // Trocar linhas se necessário
            if (max_row != k) {
                swap_rows(A, k, max_row);
            }
        }

        // Broadcast da linha pivô para todos os processos
        MPI_Bcast(&A[k][0], N + 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Cada processo paraleliza a eliminação de linhas abaixo de k
        for (int i = k + 1 + rank; i < N; i += size) {
            double factor = A[i][k] / A[k][k];
            for (int j = k; j < N + 1; j++) {
                A[i][j] -= factor * A[k][j];
            }
        }

        // Reunir as linhas atualizadas em todos os processos
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, &A[0][0], N * (N + 1) / size, MPI_DOUBLE, MPI_COMM_WORLD);
    }

    // Substituição regressiva para resolver o sistema
    double x[N] = {0}; // Vetor solução
    if (rank == 0) {
        for (int i = N - 1; i >= 0; i--) {
            x[i] = A[i][N];
            for (int j = i + 1; j < N; j++) {
                x[i] -= A[i][j] * x[j];
            }
            x[i] /= A[i][i];
        }

        // Exibir a solução
        printf("\nSolução do sistema:\n");
        for (int i = 0; i < N; i++) {
            printf("x[%d] = %6.2f\n", i, x[i]);
        }
        // Exibir tempo
        tempo_final = MPI_Wtime();
        printf("\nTempo de Execução:\n");
        printf("%f\n", tempo_final - tempo_inicial);
    }

    // Compartilhar a solução com todos os processos
    MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
