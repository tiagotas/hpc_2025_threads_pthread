// servidor.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORTA 8080
#define MAX_CLIENTES 4
#define TAM 16
#define ITER 10

typedef struct {
    unsigned char R, G, B;
} Pixel;

Pixel matriz[TAM][TAM];
int linhasPorCliente;

void inicializa_matriz() {
    for (int i = 0; i < TAM; i++)
        for (int j = 0; j < TAM; j++)
            matriz[i][j] = (Pixel){0, 0, 0};
}

void envia_bloco(int sock, int ini, int fim) {
    int linhas = fim - ini + 1;
    send(sock, &linhas, sizeof(int), 0);
    send(sock, &matriz[ini][0], linhas * TAM * sizeof(Pixel), 0);
}

void recebe_bloco(int sock, int ini, int fim) {
    int linhas = fim - ini + 1;
    recv(sock, &matriz[ini][0], linhas * TAM * sizeof(Pixel), 0);
}

int main() {
    int server_fd, clientes[MAX_CLIENTES];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORTA);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, MAX_CLIENTES);

    inicializa_matriz();
    linhasPorCliente = TAM / MAX_CLIENTES;

    printf("Aguardando %d clientes...\n", MAX_CLIENTES);
    for (int i = 0; i < MAX_CLIENTES; i++)
        clientes[i] = accept(server_fd, (struct sockaddr*)&addr, &addrlen);

    // Iterações
    for (int it = 0; it < ITER; it++) {
        for (int i = 0; i < MAX_CLIENTES; i++) {
            int ini = i * linhasPorCliente;
            int fim = ini + linhasPorCliente - 1;
            envia_bloco(clientes[i], ini, fim);
        }
        for (int i = 0; i < MAX_CLIENTES; i++) {
            int ini = i * linhasPorCliente;
            int fim = ini + linhasPorCliente - 1;
            recebe_bloco(clientes[i], ini, fim);
        }
    }

    // Encerra conexões
    for (int i = 0; i < MAX_CLIENTES; i++)
        close(clientes[i]);
    close(server_fd);
    printf("Processamento finalizado!\n");
    return 0;
}
