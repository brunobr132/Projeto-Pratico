#include "player.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Função interna para descobrir o tempo de espera conforme a carga
static float obterIntervaloPasso(int quantidadeKits) {
    switch (quantidadeKits) {
        case 0: return 0.25f; // Vazio: 4 blocos/s
        case 1: return 0.33f; // 1 kit: ~3 blocos/s
        case 2: return 0.50f; // 2 kits: 2 blocos/s
        case 3: return 1.00f; // 3 kits (cheio): 1 bloco/s
        default: return 0.25f;
    }
}

void inicializarPlayer(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], Kit **kits, int *totalKits) {
    // 1. Valores iniciais padrão do jogador
    player->vidas = 3;
    player->pontos = 0;
    player->topo = -1; // Pilha vazia
    player->tempo_acumulado = 0.0f;
    strcpy(player->mensagem_status, "Use WASD ou Setas para andar");

    // 2. Primeiro contamos quantos kits existem no mapa para fazer malloc exato
    int contagemKits = 0;
    for (int i = 0; i < MAPA_LINHAS; i++) {
        for (int j = 0; j < MAPA_COLUNAS; j++) {
            char c = mapa[i][j];
            if (c == 'a' || c == 'b' || c == 'c') {
                contagemKits++;
            } else if (c == 'P') {
                player->pos_inicial = (Posicao){i, j};
                player->pos_atual = (Posicao){i, j};
                player->pos_anterior = (Posicao){i, j};
                // IMPORTANTE: limpa o 'P' do terreno para não deixar rastro fixo
                mapa[i][j] = ' ';
            }
        }
    }

    *totalKits = contagemKits;

    // 3. Aloca dinamicamente o vetor de kits
    if (contagemKits > 0) {
        *kits = (Kit *)malloc(contagemKits * sizeof(Kit));
        int idx = 0;
        for (int i = 0; i < MAPA_LINHAS; i++) {
            for (int j = 0; j < MAPA_COLUNAS; j++) {
                char c = mapa[i][j];
                if (c == 'a' || c == 'b' || c == 'c') {
                    (*kits)[idx].id = idx;
                    (*kits)[idx].tipo = c;
                    (*kits)[idx].pos_original = (Posicao){i, j};
                    (*kits)[idx].estado = KIT_NO_CHAO;
                    idx++;
                    // Também tiramos a letra do terreno; ela será desenhada pelo vetor dinâmico!
                    mapa[i][j] = ' ';
                }
            }
        }
    } else {
        *kits = NULL;
    }
}

void liberarKits(Kit *kits) {
    if (kits != NULL) {
        free(kits);
    }
}

void atualizarMovimentoPlayer(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], bool portoesAbertos, float deltaTime) {
    // Acumula o tempo decorrido desde o último frame
    player->tempo_acumulado += deltaTime;

    // Descobre quantos kits estão na mochila (topo + 1) para calcular a velocidade
    int qtdKits = player->topo + 1;
    float intervaloNecessario = obterIntervaloPasso(qtdKits);

    // Verifica qual direção o jogador quer ir (sem diagonal, prioridade: Cima > Baixo > Esquerda > Direita)
    int dLinha = 0;
    int dColuna = 0;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        dLinha = -1;
    } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        dLinha = 1;
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        dColuna = -1;
    } else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        dColuna = 1;
    }

    // Se nenhuma tecla de direção está pressionada, permite andar imediatamente no próximo toque
    if (dLinha == 0 && dColuna == 0) {
        if (player->tempo_acumulado > intervaloNecessario) {
            player->tempo_acumulado = intervaloNecessario;
        }
        return;
    }

    // Só efetua o passo se já passou o tempo necessário da carga atual
    if (player->tempo_acumulado >= intervaloNecessario) {
        int novaLinha = player->pos_atual.linha + dLinha;
        int novaColuna = player->pos_atual.coluna + dColuna;

        // 1. Checa limites do mapa
        if (novaLinha < 0 || novaLinha >= MAPA_LINHAS || novaColuna < 0 || novaColuna >= MAPA_COLUNAS) {
            return;
        }

        char celulaDestino = mapa[novaLinha][novaColuna];

        // 2. Checa colisão com parede '#'
        if (celulaDestino == '#') {
            return;
        }

        // 3. Checa colisão com portão 'D' fechado
        // (Nota: se o jogador já estiver EM CIMA do portão que fechou, ele consegue sair para uma célula livre)
        if (celulaDestino == 'D' && !portoesAbertos) {
            strcpy(player->mensagem_status, "Portao fechado!");
            return;
        }

        // 4. Movimento válido! Atualiza posição anterior e atual
        player->pos_anterior = player->pos_atual;
        player->pos_atual.linha = novaLinha;
        player->pos_atual.coluna = novaColuna;

        // Consome o tempo do passo
        player->tempo_acumulado = 0.0f;
    }
}

void interagirComMapa(Entregador *player, char mapa[MAPA_LINHAS][MAPA_COLUNAS], Kit *kits, int totalKits, int *kitsPendentes) {
    // Só executa uma vez por aperto da tecla ESPAÇO
    if (!IsKeyPressed(KEY_SPACE)) {
        return;
    }

    int lin = player->pos_atual.linha;
    int col = player->pos_atual.coluna;

    // --- CASO 1: Verificar se existe um kit NO CHÃO na célula atual ---
    for (int i = 0; i < totalKits; i++) {
        if (kits[i].estado == KIT_NO_CHAO &&
            kits[i].pos_original.linha == lin &&
            kits[i].pos_original.coluna == col) {

            // Verifica se a pilha ainda tem espaço (máximo 3: índices 0, 1, 2)
            if (player->topo < CAPACIDADE_PILHA - 1) {
                player->topo++;
                player->pilha[player->topo] = kits[i].id; // Push na pilha!
                kits[i].estado = KIT_TRANSPORTADO;
                strcpy(player->mensagem_status, TextFormat("Kit '%c' coletado!", kits[i].tipo));
            } else {
                strcpy(player->mensagem_status, "Carga cheia (3/3)!");
            }
            return; // Já interagiu nesta tecla
        }
    }

    // --- CASO 2: Verificar se o jogador está sobre um ABRIGO ('A', 'B' ou 'C') ---
    char celulaAtual = mapa[lin][col];
    if (celulaAtual == 'A' || celulaAtual == 'B' || celulaAtual == 'C') {
        if (player->topo == -1) {
            strcpy(player->mensagem_status, "Mochila vazia!");
            return;
        }

        // Olha apenas o kit no TOPO da pilha
        int idKitTopo = player->pilha[player->topo];
        char tipoEsperado = (char)tolower(celulaAtual); // 'A' -> 'a', 'B' -> 'b', 'C' -> 'c'

        if (kits[idKitTopo].tipo == tipoEsperado) {
            // Entrega correta! Faz Pop da pilha
            player->topo--;
            kits[idKitTopo].estado = KIT_ENTREGUE;
            player->pontos += 100;
            if (*kitsPendentes > 0) {
                (*kitsPendentes)--;
            }
            strcpy(player->mensagem_status, TextFormat("Kit '%c' entregue! +100 pts", tipoEsperado));
        } else {
            // Tipo diferente do topo: não perde vida nem pontos
            strcpy(player->mensagem_status, TextFormat("Topo eh '%c', abrigo pede '%c'", kits[idKitTopo].tipo, tipoEsperado));
        }
    }
}

void resetarPlayerAposDano(Entregador *player, Kit *kits) {
    // Volta para posição inicial
    player->pos_atual = player->pos_inicial;
    player->pos_anterior = player->pos_inicial;
    player->tempo_acumulado = 0.0f;

    // Esvazia a pilha devolvendo os kits transportados para suas posições originais no chão
    while (player->topo >= 0) {
        int idKit = player->pilha[player->topo];
        kits[idKit].estado = KIT_NO_CHAO;
        player->topo--;
    }
    strcpy(player->mensagem_status, "Atingido! Kits devolvidos.");
}

void desenharPlayerEKits(const Entregador *player, const Kit *kits, int totalKits, int tileSize, int originX, int originY) {
    // 1. Desenha todos os kits que ainda estão no chão
    for (int i = 0; i < totalKits; i++) {
        if (kits[i].estado == KIT_NO_CHAO) {
            int x = originX + kits[i].pos_original.coluna * tileSize;
            int y = originY + kits[i].pos_original.linha * tileSize;

            Color corKit = WHITE;
            if (kits[i].tipo == 'a') corKit = (Color){105, 190, 211, 255};
            else if (kits[i].tipo == 'b') corKit = (Color){242, 165, 55, 255};
            else if (kits[i].tipo == 'c') corKit = (Color){188, 151, 213, 255};

            DrawRectangle(x + 2, y + 2, tileSize - 4, tileSize - 4, corKit);
            DrawText(TextFormat("%c", kits[i].tipo), x + 6, y + 4, 12, BLACK);
        }
    }

    // 2. Desenha o Entregador (Player) por cima do mapa e dos kits
    int px = originX + player->pos_atual.coluna * tileSize;
    int py = originY + player->pos_atual.linha * tileSize;

    DrawRectangle(px + 2, py + 2, tileSize - 4, tileSize - 4, YELLOW);
    DrawRectangleLines(px + 2, py + 2, tileSize - 4, tileSize - 4, ORANGE);
    DrawText("P", px + 6, py + 4, 12, BLACK);
}