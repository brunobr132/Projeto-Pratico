#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "mapa.h" // Já traz a struct Posicao { int linha; int coluna; }

#define CAPACIDADE_PILHA 3
#define MAPA_LINHAS 20
#define MAPA_COLUNAS 40

// Estados possíveis de cada kit no mapa
typedef enum {
    KIT_NO_CHAO = 0,
    KIT_TRANSPORTADO = 1,
    KIT_ENTREGUE = 2
} EstadoKit;

// Representa cada kit médico espalhado pelo mapa ('a', 'b', 'c')
typedef struct {
    int id;               // Identificador único (0, 1, 2...)
    char tipo;            // 'a', 'b' ou 'c'
    Posicao pos_original; // Onde ele nasceu (para devolver se o jogador morrer)
    EstadoKit estado;     // 0 = no chão, 1 = na mochila, 2 = entregue no abrigo
} Kit;

// Representa o entregador e sua mochila (pilha LIFO de até 3 kits)
typedef struct {
    Posicao pos_atual;
    Posicao pos_anterior; // Usada depois para detectar cruzamento com zumbis
    Posicao pos_inicial;  // Onde nasceu no mapa ('P')
    
    int vidas;            // Começa com 3
    int pontos;           // Começa com 0
    
    int pilha[CAPACIDADE_PILHA]; // Guarda os IDs dos kits carregados
    int topo;                    // Índice do topo da pilha (-1 = vazia, 0 a 2 = itens)
    
    float tempo_acumulado;       // Controla o intervalo entre passos
    char mensagem_status[64];    // Mensagem curta para mostrar na tela (ex: "Carga cheia!")
} Entregador;

// --- FUNÇÕES DO MÓDULO ---

// Inicializa o entregador na posição 'P' e aloca dinamicamente o vetor de kits
void inicializarPlayer(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], Kit **kits, int *totalKits);

// Libera a memória alocada para o vetor de kits
void liberarKits(Kit *kits);

// Lê o teclado (Setas ou WASD) e move o jogador respeitando paredes, portões e velocidade
void atualizarMovimentoPlayer(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], bool portoesAbertos, float deltaTime);

// Processa a tecla ESPAÇO para coletar kit no chão ou entregar no abrigo correspondente
void interagirComMapa(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], Kit *kits, int totalKits, int *kitsPendentes);

// Reinicia posição do jogador e devolve kits da pilha ao chão (quando sofre dano de zumbi)
void resetarPlayerAposDano(Entregador *player, Kit *kits);

// Desenha o entregador e os kits que ainda estão no chão
void desenharPlayerEKits(const Entregador *player, const Kit *kits, int totalKits, int tileSize, int originX, int originY);

#endif // PLAYER_H