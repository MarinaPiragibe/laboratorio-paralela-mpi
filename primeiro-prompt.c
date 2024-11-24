#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 3 // Dimensão do sistema (exemplo: 3x3)

//Resolva o problema de sistema linear de equações por eliminação de Gauss  na linguagem de programação C e MPI

// Função para realizar troca de linhas
void swap_rows(double A[N][N+1], int row1, int row2) {
    for (int i = 0; i < N + 1; i++) {
        double temp = A[row1][i];
        A[row1][i] = A[row2][i];
        A[row2][i] = temp;
    }
}

void printMatrix(double A[N][N+1]){
    for (int i = 0; i < N; i++) {
            for (int j = 0; j < N + 1; j++) {
                printf("%6.2f ", A[i][j]);
            }
            printf("\n");
        }
}

int main(int argc, char** argv) {
    int rank, size;
    float tempo_inicial, tempo_final;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tempo_inicial = MPI_Wtime();

    // Matriz aumentada (A|b)
    double A[N][N+1] = {
        {2, -1, 1, 3},
        {1, 3, 2, 12},
        {1, -1, 2, 5}
    };

    if (rank == 0) {
        printf("Matriz inicial (A|b):\n");
        printMatrix(A);
    }

    // Etapa de eliminação
    for (int k = 0; k < N; k++) {
        // Processo principal (rank 0) realiza pivoteamento
        if (rank == 0) {
            // Pivoteamento (encontrar maior elemento na coluna)
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

        // Broadcast da matriz atualizada
        MPI_Bcast(&A[0][0], N * (N + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);
        //printf("O processo com ranque %d recebeu o valor: %f\n",rank, A[0][0]);
        // Paralelizar a eliminação de linhas abaixo de k
        for (int i = k + 1; i < N; i++) {
            if (i % size == rank) { // Round Robin: só processa se for a vez do rank atual
                //printf("_______________\n");
                //printf("k = %d, rank = %d\n", k, rank);
                double factor = A[i][k] / A[k][k];
                //printf("factor %d: %lf", k, factor);
                for (int j = k; j < N + 1; j++) {
                    A[i][j] -= factor * A[k][j];
                    //printf("| A[%d][%d]: %lf", i, j, A[i][j]);
                }
                //printf("\n");
            }
        }
        // printMatrix(A);
        // printf("_______________\n");
        // Reunir a matriz em todos os processos
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, &A[0][0], N * (N + 1) / size, MPI_DOUBLE, MPI_COMM_WORLD);
    }

    // Resolver o sistema (substituição regressiva)
    double x[N];
    if (rank == 0) {
        //printMatrix(A);
        for (int i = N - 1; i >= 0; i--) {
            x[i] = A[i][N];
            for (int j = i + 1; j < N; j++) {
                x[i] -= A[i][j] * x[j];
            }
            x[i] /= A[i][i];
        }

        //Exibir solução
        printf("\nSolução do sistema:\n");
        for (int i = 0; i < N; i++) {
            printf("x[%d] = %6.2f\n", i, x[i]);
        }

        // Exibir tempo
        tempo_final = MPI_Wtime();
        printf("\nTempo de Execução: ");
        printf("%f\n", tempo_final - tempo_inicial);
        
    }

    MPI_Finalize();
    return 0;
}
