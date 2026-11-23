#ifndef ZUMBI_H
#define ZUMBI_H

#include <stdbool.h>
#include "mapa.h"   // Posicao { int linha; int coluna; }
#include "player.h" // Entregador, Kit, MAPA_LINHAS, MAPA_COLUNAS

#define INTERVALO_PASSO_ZUMBI 0.50f // 2 blocos por segundo (0.5s por celula)

typedef struct {
    Posicao pos_atual;
    Posicao pos_anterior;
    Posicao pos_inicial;
    int direcao;          // 0: cima, 1: baixo, 2: esquerda, 3: direita
    float tempo_acumulado;
} Zumbi;

// Aloca dinamicamente os zumbis encontrados com 'Z' no mapa e limpa 'Z' da matriz do terreno
void inicializarZumbis(Zumbi **zumbis, int *totalZumbis, char mapa[MAPA_LINHAS][MAPA_COLUNAS]);

// Libera o vetor dinâmico de zumbis
void liberarZumbis(Zumbi *zumbis);

// Atualiza o movimento dos zumbis perseguindo alvoPlayer a cada 0.5s
void atualizarZumbis(
    Zumbi *zumbis,
    int totalZumbis,
    char mapa[MAPA_LINHAS][MAPA_COLUNAS],
    bool portoesAbertos,
    Posicao alvoPlayer,
    float deltaTime
);

// Verifica colisao (mesma celula ou cruzamento) entre o jogador e qualquer zumbi.
// Se houver colisao, aplica dano (-1 vida), devolve kits da pilha ao chao e reseta posicoes.
// Retorna true se o jogador perdeu todas as vidas (Game Over).
bool verificarColisaoZumbis(Entregador *player, Kit *kits, Zumbi *zumbis, int totalZumbis);

// Reseta todos os zumbis para pos_inicial e zera o temporizador de 0.5s
void resetarZumbisAposDano(Zumbi *zumbis, int totalZumbis);

// Desenha os zumbis por cima do mapa
void desenharZumbis(const Zumbi *zumbis, int totalZumbis, int tileSize, int originX, int originY);

#endif // ZUMBI_H