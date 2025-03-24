#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int meu_id, num_procs, i, tamanho_local, inicio, fim, chave_busca, indice_encontrado = -1;
    int *vetor_local, *vetor_principal;
    double tempo_inicial, tempo_final;
    int tamanho_vetor;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc > 1 && atoi(argv[1]) == 1) {
        printf("Erro: dataset_pequeno não definido. Certifique-se de declarar ou inicializar corretamente.");
        MPI_Abort(MPI_COMM_WORLD, 1);
    } else {
        tamanho_vetor = 65536; 
        vetor_principal = (int*) malloc(tamanho_vetor * sizeof(int));
        if (vetor_principal == NULL) {
            printf("Erro: Falha ao alocar memória para o vetor.");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (i = 0; i < tamanho_vetor; i++) {
            vetor_principal[i] = i + 1; 
        }
        chave_busca = 32768; 
    }

    tempo_inicial = MPI_Wtime();

    
    tamanho_local = (meu_id == num_procs - 1) ? (tamanho_vetor - (num_procs - 1) * (tamanho_vetor / num_procs)) : (tamanho_vetor / num_procs);
    inicio = meu_id * (tamanho_vetor / num_procs);

    vetor_local = (int*) malloc(tamanho_local * sizeof(int));
    if (vetor_local == NULL) {
        printf("Erro: Falha ao alocar memória para vetor_local.");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(vetor_principal, tamanho_vetor / num_procs, MPI_INT,
                vetor_local, tamanho_vetor / num_procs, MPI_INT,
                0, MPI_COMM_WORLD);

    
    for (i = 0; i < tamanho_local; i++) {
        if (vetor_local[i] == chave_busca) {
            indice_encontrado = inicio + i; 
            break;
        }
    }

    
    int indice_global_encontrado = -1;
    MPI_Allreduce(&indice_encontrado, &indice_global_encontrado, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    tempo_final = MPI_Wtime();

    if (meu_id == 0) {
        if (indice_global_encontrado != -1) {
            printf("Elemento %d encontrado na posição %d\n", chave_busca, indice_global_encontrado);
        } else {
            printf("Elemento %d não encontrado no vetor.\n", chave_busca);
        }
        printf("Tempo de execução: %f segundos\n", tempo_final - tempo_inicial);
    }

    free(vetor_local);
    if (meu_id == 0 && tamanho_vetor == 65536) {
        free(vetor_principal);
    }

    MPI_Finalize();
    return 0;
}
