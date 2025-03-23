#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Definindo vetores de teste
int dataset_pequeno[] = {1, 5, 2, 8, 9, 3, 11, 4, 10, 6, 7, 12, 15, 13, 14, 16};
int dataset_grande[] = { /* Vetor de tamanho 2^16 com valores únicos */ };

int main(int argc, char *argv[]) {
    int meu_id, num_procs, i, tamanho_local, inicio, fim, chave_busca, indice_encontrado = -1;
    int *vetor_local, *vetor_principal;
    double tempo_inicial, tempo_final;
    int tamanho_vetor;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Definição do vetor e do valor a ser procurado
    if (argc > 1 && atoi(argv[1]) == 1) {
        vetor_principal = dataset_pequeno;
        tamanho_vetor = sizeof(dataset_pequeno) / sizeof(dataset_pequeno[0]);
        chave_busca = 11; // Chave para busca no vetor pequeno
    } else {
        // Usando o dataset maior (lembrar de substituir por um vetor de 2^16 elementos únicos)
        tamanho_vetor = 65536; // 2^16
        vetor_principal = (int*) malloc(tamanho_vetor * sizeof(int));
        if (vetor_principal == NULL) {
            printf("Erro: Falha ao alocar memória para o vetor.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Inicializando o vetor com valores únicos (adaptar para o caso geral)
        for (i = 0; i < tamanho_vetor; i++) {
            vetor_principal[i] = i + 1; // Exemplo: valores de 1 a 65536
        }

        chave_busca = 32768; // Chave para busca no vetor grande (meio do vetor)
    }

    tempo_inicial = MPI_Wtime();

    // Distribuindo o vetor entre os processos
    tamanho_local = tamanho_vetor / num_procs;
    inicio = meu_id * tamanho_local;
    fim = (meu_id == num_procs - 1) ? tamanho_vetor : inicio + tamanho_local; // Ajuste para o último processo

    vetor_local = (int*) malloc(tamanho_local * sizeof(int));

    MPI_Scatter(vetor_principal, tamanho_local, MPI_INT,
                vetor_local, tamanho_local, MPI_INT,
                0, MPI_COMM_WORLD);

    // Busca local no vetor distribuído
    for (i = 0; i < tamanho_local; i++) {
        if (vetor_local[i] == chave_busca) {
            indice_encontrado = inicio + i; // Calculando o índice global
            break;
        }
    }

    // Redução para encontrar o índice global
    int indice_global_encontrado = -1;
    MPI_Reduce(&indice_encontrado, &indice_global_encontrado, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    tempo_final = MPI_Wtime();

    // Imprimindo o resultado no processo mestre
    if (meu_id == 0) {
        if (indice_global_encontrado != -1) {
            printf("Elemento %d encontrado na posição %d\n", chave_busca, indice_global_encontrado);
        } else {
            printf("Elemento %d não encontrado no vetor.\n", chave_busca);
        }
        printf("Tempo de execução: %f segundos\n", tempo_final - tempo_inicial);
    }

    // Liberando a memória alocada
    free(vetor_local);
    if (meu_id == 0 && tamanho_vetor == 65536) {  // Liberando apenas se alocou (vetor grande)
        free(vetor_principal);
    }

    MPI_Finalize();
    return 0;
}
