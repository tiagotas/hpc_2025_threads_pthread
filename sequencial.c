#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERACOES 1000
#define RGB_COMPONENTES 3
#define BORDA 127

typedef struct {
    int x, y, r, g, b;
} PontoFixo;

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

void liberar_matriz(int ***matriz, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            free(matriz[i][j]);
        free(matriz[i]);
    }
    free(matriz);
}

void ler_arquivo(char* nome, int *n, int *num_fixos, PontoFixo **fixos) {
    FILE *fp = fopen(nome, "r");
    fscanf(fp, "%d %d", n, num_fixos);
    *fixos = malloc((*num_fixos) * sizeof(PontoFixo));
    for (int i = 0; i < *num_fixos; i++)
        fscanf(fp, "%d %d %d %d %d", &(*fixos)[i].x, &(*fixos)[i].y, &(*fixos)[i].r, &(*fixos)[i].g, &(*fixos)[i].b);
    fclose(fp);
}

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
    if (argc < 2) {
        printf("Uso: %s entrada.txt\n", argv[0]);
        return 1;
    }

    int n, num_fixos;
    PontoFixo *fixos;
    ler_arquivo(argv[1], &n, &num_fixos, &fixos);

    int ***matriz = alocar_matriz(n, 0);
    int ***buffer = alocar_matriz(n, 0);

    char **fixo_mask = malloc(n * sizeof(char*));
    for (int i = 0; i < n; i++) fixo_mask[i] = calloc(n, sizeof(char));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == 0 || j == 0 || i == n-1 || j == n-1)
                for (int k = 0; k < RGB_COMPONENTES; k++)
                    matriz[i][j][k] = BORDA;
        }
    }

    for (int i = 0; i < num_fixos; i++) {
        int x = fixos[i].x, y = fixos[i].y;
        matriz[x][y][0] = fixos[i].r;
        matriz[x][y][1] = fixos[i].g;
        matriz[x][y][2] = fixos[i].b;
        fixo_mask[x][y] = 1;
    }

    clock_t inicio = clock();

    for (int it = 0; it < ITERACOES; it++) {
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                if (fixo_mask[i][j]) continue;

                for (int k = 0; k < RGB_COMPONENTES; k++) {
                    buffer[i][j][k] = (
                        matriz[i][j][k] +
                        matriz[i-1][j][k] +
                        matriz[i+1][j][k] +
                        matriz[i][j-1][k] +
                        matriz[i][j+1][k]
                    ) / 5;
                }
            }
        }

        int ***tmp = matriz;
        matriz = buffer;
        buffer = tmp;
    }

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo: %.4fs\n", tempo);

    salvar_resultado("saida_seq.txt", matriz, n);

    liberar_matriz(matriz, n);
    liberar_matriz(buffer, n);
    for (int i = 0; i < n; i++) free(fixo_mask[i]);
    free(fixo_mask);
    free(fixos);

    return 0;
}
