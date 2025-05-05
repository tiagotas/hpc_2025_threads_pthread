#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 16
#define ITERACOES 1000
#define RGB_COMPONENTES 3
#define BORDA 127

typedef struct {
    int x, y, r, g, b;
} PontoFixo;

typedef struct {
    int id, n, n_threads;
    int ***matriz, ***buffer;
    char **fixos;
} ThreadArgs;

// Variável de barreira global
pthread_barrier_t barreira;

void* espalhar_cor(void* arg) {
    ThreadArgs *args = (ThreadArgs*) arg;
    int n = args->n, id = args->id, n_threads = args->n_threads;
    int ini = id * n / n_threads;
    int fim = (id + 1) * n / n_threads;

    // Evita que a thread processe a borda
    if (ini == 0) ini = 1;
    if (fim == n) fim = n - 1;

    for (int it = 0; it < ITERACOES; it++) {
        for (int i = ini; i < fim; i++) {
            for (int j = 1; j < n - 1; j++) {
                if (args->fixos[i][j]) continue;

                for (int k = 0; k < RGB_COMPONENTES; k++) {
                    args->buffer[i][j][k] = (
                        args->matriz[i][j][k] +
                        args->matriz[i-1][j][k] +
                        args->matriz[i+1][j][k] +
                        args->matriz[i][j-1][k] +
                        args->matriz[i][j+1][k]
                    ) / 5;
                }
            }
        }

        pthread_barrier_wait(&barreira);

        // Troca os ponteiros das matrizes
        int ***tmp = args->matriz;
        args->matriz = args->buffer;
        args->buffer = tmp;

        pthread_barrier_wait(&barreira);
    }

    pthread_exit(NULL);
}

// Função de alocação
int*** alocar_matriz(int n, int valor_inicial) {
    int ***matriz = malloc(n * sizeof(int**));
    for (int i = 0; i < n; i++) {
        matriz[i] = malloc(n * sizeof(int*));
        for (int j = 0; j < n; j++) {
            matriz[i][j] = malloc(RGB_COMPONENTES * sizeof(int));
            for (int k = 0; k < RGB_COMPONENTES; k++)
                matriz[i][j][k] = valor_inicial;
        }
    }
    return matriz;
}

// Liberação de memória
void liberar_matriz(int ***matriz, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            free(matriz[i][j]);
        free(matriz[i]);
    }
    free(matriz);
}

// Leitura do arquivo de entrada
void ler_arquivo(char* nome, int *n, int *num_fixos, PontoFixo **fixos) {
    FILE *fp = fopen(nome, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    fscanf(fp, "%d %d", n, num_fixos);
    *fixos = malloc((*num_fixos) * sizeof(PontoFixo));
    for (int i = 0; i < *num_fixos; i++)
        fscanf(fp, "%d %d %d %d %d", &(*fixos)[i].x, &(*fixos)[i].y, &(*fixos)[i].r, &(*fixos)[i].g, &(*fixos)[i].b);
    fclose(fp);
}

// Escrita do resultado
void salvar_resultado(char* nome, int ***matriz, int n) {
    FILE *fp = fopen(nome, "w");
    for (int i = 0; i < n; i++) {
        for (int j = 64; j < 128; j++)
            fprintf(fp, "%d %d %d ", matriz[i][j][0], matriz[i][j][1], matriz[i][j][2]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s entrada.txt num_threads\n", argv[0]);
        return 1;
    }

    int n, num_fixos;
    PontoFixo *fixos;
    ler_arquivo(argv[1], &n, &num_fixos, &fixos);

    int ***matriz = alocar_matriz(n, 0);
    int ***buffer = alocar_matriz(n, 0);

    // Inicializar bordas com valor fixo
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == 0 || j == 0 || i == n-1 || j == n-1)
                for (int k = 0; k < RGB_COMPONENTES; k++)
                    matriz[i][j][k] = buffer[i][j][k] = BORDA;
        }
    }

    // Aplicar pontos fixos
    char **fixo_mask = malloc(n * sizeof(char*));
    for (int i = 0; i < n; i++) fixo_mask[i] = calloc(n, sizeof(char));
    for (int i = 0; i < num_fixos; i++) {
        int x = fixos[i].x, y = fixos[i].y;
        matriz[x][y][0] = fixos[i].r;
        matriz[x][y][1] = fixos[i].g;
        matriz[x][y][2] = fixos[i].b;
        fixo_mask[x][y] = 1;
    }

    int num_threads = atoi(argv[2]);
    if (num_threads < 1 || num_threads > MAX_THREADS) {
        fprintf(stderr, "Número de threads inválido (1 a %d)\n", MAX_THREADS);
        return 1;
    }

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    pthread_barrier_init(&barreira, NULL, num_threads);

    clock_t inicio = clock();

    for (int i = 0; i < num_threads; i++) {
        args[i] = (ThreadArgs){i, n, num_threads, matriz, buffer, fixo_mask};
        pthread_create(&threads[i], NULL, espalhar_cor, &args[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo: %.4fs\n", tempo);

    salvar_resultado("saida.txt", args[0].matriz, n);

    liberar_matriz(matriz, n);
    liberar_matriz(buffer, n);
    for (int i = 0; i < n; i++) free(fixo_mask[i]);
    free(fixo_mask);
    free(fixos);

    pthread_barrier_destroy(&barreira);

    return 0;
}