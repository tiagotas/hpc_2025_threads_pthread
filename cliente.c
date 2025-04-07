// cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TAM 16

typedef struct {
    unsigned char R, G, B;
} Pixel;

Pixel bloco[4][TAM];  // Assume 4 linhas por cliente

void processa_bloco(Pixel bloco[][TAM], int linhas) {
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < TAM; j++) {
            bloco[i][j].R = (bloco[i][j].R + 5) % 256;
            bloco[i][j].G = (bloco[i][j].G + 5) % 256;
            bloco[i][j].B = (bloco[i][j].B + 5) % 256;
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    connect(sock, (struct sockaddr*)&server, sizeof(server));

    for (int i = 0; i < 10; i++) {
        int linhas;
        recv(sock, &linhas, sizeof(int), 0);
        recv(sock, &bloco[0][0], linhas * TAM * sizeof(Pixel), 0);
        processa_bloco(bloco, linhas);
        send(sock, &bloco[0][0], linhas * TAM * sizeof(Pixel), 0);
    }

    close(sock);
    return 0;
}
