#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void gauss_elimination(double *A, double *b, int n, int rank, int size) {
    for (int k = 0; k < n; k++) {
        // Broadcast da linha pivô
        for (int j = k; j < n; j++) {
            MPI_Bcast(&A[k * n + j], 1, MPI_DOUBLE, k % size, MPI_COMM_WORLD);
        }
        MPI_Bcast(&b[k], 1, MPI_DOUBLE, k % size, MPI_COMM_WORLD);

        // Eliminação em paralelo
        for (int i = k + 1; i < n; i++) {
            if (i % size == rank) {
                double factor = A[i * n + k] / A[k * n + k];
                for (int j = k; j < n; j++) {
                    A[i * n + j] -= factor * A[k * n + j];
                }
                b[i] -= factor * b[k];
            }
        }
    }
}

void back_substitution(double *A, double *b, double *x, int n, int rank, int size) {
    for (int i = n - 1; i >= 0; i--) {
        if (rank == 0) {
            x[i] = b[i];
            for (int j = i + 1; j < n; j++) {
                x[i] -= A[i * n + j] * x[j];
            }
            x[i] /= A[i * n + i];
        }
        MPI_Bcast(&x[i], 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    double tempo_inicial, tempo_final;
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tempo_inicial = MPI_Wtime();

    int n = 3; // Tamanho fixo para esta matriz
    double A[3][3] = {
        {2, -1, 1},
        {1, 3, 2},
        {1, -1, 2}
    };
    double b[3] = {3, 12, 5};
    double x[3] = {0};

    // if (rank == 0) {
    //     for (int i = 0; i < n; i++) {
    //         b[i] = rand() % 100;
    //         for (int j = 0; j < n; j++) {
    //             A[i * n + j] = rand() % 100;
    //         }
    //     }
    // }

    // Transmissão inicial da matriz e vetor
    MPI_Bcast(A, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    gauss_elimination(&A[0][0], b, n, rank, size);
    back_substitution(&A[0][0], b, x, n, rank, size);

    if (rank == 0) {
        printf("Resultado:\n");
        for (int i = 0; i < n; i++) {
            printf("x[%d] = %f\n", i, x[i]);
        }
        tempo_final = MPI_Wtime();
        printf("Tempo de Execução: %f", tempo_final - tempo_inicial);
    }

    MPI_Finalize();
    return 0;
}
