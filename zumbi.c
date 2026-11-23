#include "zumbi.h"
#include "raylib.h"
#include <stdlib.h>

#define RAIO_PERSEGUICAO 5 // Só persegue quando o jogador estiver a até 5 blocos de distância

// Vetores de deslocamento para as 4 direcoes:
// 0 = Cima, 1 = Baixo, 2 = Esquerda, 3 = Direita
static const int dLin[4] = {-1, 1, 0, 0};
static const int dCol[4] = {0, 0, -1, 1};

static int direcaoOposta(int dir) {
    switch (dir) {
        case 0: return 1;
        case 1: return 0;
        case 2: return 3;
        case 3: return 2;
        default: return -1;
    }
}

static bool celulaValidaParaZumbi(int lin, int col, char mapa[MAPA_LINHAS][MAPA_COLUNAS], bool portoesAbertos) {
    if (lin < 0 || lin >= MAPA_LINHAS || col < 0 || col >= MAPA_COLUNAS) {
        return false;
    }
    char c = mapa[lin][col];
    if (c == '#') {
        return false;
    }
    if (c == 'D' && !portoesAbertos) {
        return false;
    }
    return true;
}

static int distanciaManhattan(int l1, int c1, int l2, int c2) {
    int dl = l1 - l2;
    int dc = c1 - c2;
    if (dl < 0) dl = -dl;
    if (dc < 0) dc = -dc;
    return dl + dc;
}

// Verifica se algum outro zumbi (diferente de indiceAtual) já está na célula (lin, col)
static bool celulaOcupadaPorOutroZumbi(
    const Zumbi *zumbis,
    int totalZumbis,
    int indiceAtual,
    int lin,
    int col
) {
    for (int i = 0; i < totalZumbis; i++) {
        if (i == indiceAtual) continue;
        if (zumbis[i].pos_atual.linha == lin && zumbis[i].pos_atual.coluna == col) {
            return true;
        }
    }
    return false;
}

static int escolherDirecaoInteligente(
    const Zumbi *z,
    int indiceZumbi,
    const Zumbi *zumbis,
    int totalZumbis,
    Posicao alvoPlayer,
    char mapa[MAPA_LINHAS][MAPA_COLUNAS],
    bool portoesAbertos
) {
    int oposta = direcaoOposta(z->direcao);
    int distAtualPlayer = distanciaManhattan(
        z->pos_atual.linha, z->pos_atual.coluna,
        alvoPlayer.linha, alvoPlayer.coluna
    );
    bool devePerseguir = (distAtualPlayer <= RAIO_PERSEGUICAO);

    // Coleta direções válidas divididas entre:
    // 1) Livres de outro zumbi e sem voltar para trás
    // 2) Livres de outro zumbi (incluindo voltar para trás, se necessário)
    // 3) Qualquer célula válida (mesmo que outro zumbi esteja lá, caso todas estejam ocupadas)
    int livresSemVoltar[4], qtdLivresSemVoltar = 0;
    int livresTodas[4], qtdLivresTodas = 0;
    int todasValidas[4], qtdTodas = 0;

    for (int d = 0; d < 4; d++) {
        int nl = z->pos_atual.linha + dLin[d];
        int nc = z->pos_atual.coluna + dCol[d];

        if (!celulaValidaParaZumbi(nl, nc, mapa, portoesAbertos)) {
            continue;
        }

        todasValidas[qtdTodas++] = d;
        bool ocupado = celulaOcupadaPorOutroZumbi(zumbis, totalZumbis, indiceZumbi, nl, nc);

        if (!ocupado) {
            livresTodas[qtdLivresTodas++] = d;
            if (d != oposta) {
                livresSemVoltar[qtdLivresSemVoltar++] = d;
            }
        }
    }

    // Seleciona o conjunto de direções preferencial sem sobrepor outro zumbi
    const int *pool = NULL;
    int qtdPool = 0;

    if (qtdLivresSemVoltar > 0) {
        pool = livresSemVoltar;
        qtdPool = qtdLivresSemVoltar;
    } else if (qtdLivresTodas > 0) {
        pool = livresTodas;
        qtdPool = qtdLivresTodas;
    } else if (qtdTodas > 0) {
        pool = todasValidas;
        qtdPool = qtdTodas;
    } else {
        return -1; // Totalmente cercado
    }

    // Modo perseguição: escolhe entre as direções do pool que mais reduzem a distância ao jogador
    if (devePerseguir) {
        int melhorDist = 999999;
        int melhoresDirs[4];
        int qtdMelhores = 0;

        for (int i = 0; i < qtdPool; i++) {
            int d = pool[i];
            int nl = z->pos_atual.linha + dLin[d];
            int nc = z->pos_atual.coluna + dCol[d];
            int dist = distanciaManhattan(nl, nc, alvoPlayer.linha, alvoPlayer.coluna);

            if (dist < melhorDist) {
                melhorDist = dist;
                melhoresDirs[0] = d;
                qtdMelhores = 1;
            } else if (dist == melhorDist) {
                melhoresDirs[qtdMelhores++] = d;
            }
        }

        // Se houver empate na menor distância, sorteia entre elas para evitar loops repetitivos
        if (qtdMelhores > 0) {
            return melhoresDirs[GetRandomValue(0, qtdMelhores - 1)];
        }
    }

    // Modo patrulha/aleatório (quando o jogador está fora do raio de aproximação):
    // 65% de chance de manter a direção atual se ela estiver no pool, senão sorteia outra
    for (int i = 0; i < qtdPool; i++) {
        if (pool[i] == z->direcao && GetRandomValue(0, 99) < 65) {
            return z->direcao;
        }
    }

    return pool[GetRandomValue(0, qtdPool - 1)];
}

void inicializarZumbis(Zumbi **zumbis, int *totalZumbis, char mapa[MAPA_LINHAS][MAPA_COLUNAS]) {
    int count = 0;
    for (int i = 0; i < MAPA_LINHAS; i++) {
        for (int j = 0; j < MAPA_COLUNAS; j++) {
            if (mapa[i][j] == 'Z') {
                count++;
            }
        }
    }

    *totalZumbis = count;
    if (count <= 0) {
        *zumbis = NULL;
        return;
    }

    *zumbis = (Zumbi *)malloc(count * sizeof(Zumbi));
    if (!(*zumbis)) {
        *totalZumbis = 0;
        return;
    }

    int idx = 0;
    for (int i = 0; i < MAPA_LINHAS; i++) {
        for (int j = 0; j < MAPA_COLUNAS; j++) {
            if (mapa[i][j] == 'Z') {
                (*zumbis)[idx].pos_inicial = (Posicao){i, j};
                (*zumbis)[idx].pos_atual = (Posicao){i, j};
                (*zumbis)[idx].pos_anterior = (Posicao){i, j};
                (*zumbis)[idx].direcao = GetRandomValue(0, 3);
                (*zumbis)[idx].tempo_acumulado = 0.0f;
                idx++;

                // Limpa 'Z' da matriz do mapa para que os zumbis andem livremente sem deixar rastro
                mapa[i][j] = ' ';
            }
        }
    }
}

void liberarZumbis(Zumbi *zumbis) {
    if (zumbis != NULL) {
        free(zumbis);
    }
}

void atualizarZumbis(
    Zumbi *zumbis,
    int totalZumbis,
    char mapa[MAPA_LINHAS][MAPA_COLUNAS],
    bool portoesAbertos,
    Posicao alvoPlayer,
    float deltaTime
) {
    if (!zumbis || totalZumbis <= 0) return;

    for (int i = 0; i < totalZumbis; i++) {
        zumbis[i].tempo_acumulado += deltaTime;

        if (zumbis[i].tempo_acumulado >= INTERVALO_PASSO_ZUMBI) {
            int dir = escolherDirecaoInteligente(
                &zumbis[i], i, zumbis, totalZumbis, alvoPlayer, mapa, portoesAbertos
            );
            zumbis[i].direcao = dir;

            if (dir != -1) {
                zumbis[i].pos_anterior = zumbis[i].pos_atual;
                zumbis[i].pos_atual.linha += dLin[dir];
                zumbis[i].pos_atual.coluna += dCol[dir];
            } else {
                zumbis[i].pos_anterior = zumbis[i].pos_atual;
            }

            zumbis[i].tempo_acumulado = 0.0f;
        }
    }
}

void resetarZumbisAposDano(Zumbi *zumbis, int totalZumbis) {
    if (!zumbis || totalZumbis <= 0) return;

    for (int i = 0; i < totalZumbis; i++) {
        zumbis[i].pos_atual = zumbis[i].pos_inicial;
        zumbis[i].pos_anterior = zumbis[i].pos_inicial;
        zumbis[i].direcao = GetRandomValue(0, 3);
        zumbis[i].tempo_acumulado = 0.0f;
    }
}

bool verificarColisaoZumbis(Entregador *player, Kit *kits, Zumbi *zumbis, int totalZumbis) {
    if (!player || !zumbis || totalZumbis <= 0) return false;

    for (int i = 0; i < totalZumbis; i++) {
        bool mesmaCelula = (player->pos_atual.linha == zumbis[i].pos_atual.linha &&
                           player->pos_atual.coluna == zumbis[i].pos_atual.coluna);

        bool cruzaram = (player->pos_atual.linha == zumbis[i].pos_anterior.linha &&
                         player->pos_atual.coluna == zumbis[i].pos_anterior.coluna &&
                         player->pos_anterior.linha == zumbis[i].pos_atual.linha &&
                         player->pos_anterior.coluna == zumbis[i].pos_atual.coluna);

        if (mesmaCelula || cruzaram) {
            player->vidas -= 1;
            player->pontos -= 50;
            if (player->pontos < 0) {
                player->pontos = 0;
            }

            if (player->vidas <= 0) {
                player->vidas = 0;
                return true; // Game Over
            }

            // Ainda tem vidas: devolve kits da pilha ao chao e reseta jogador e zumbis
            resetarPlayerAposDano(player, kits);
            resetarZumbisAposDano(zumbis, totalZumbis);
            return false;
        }
    }

    return false;
}

void desenharZumbis(const Zumbi *zumbis, int totalZumbis, int tileSize, int originX, int originY) {
    if (!zumbis || totalZumbis <= 0) return;

    for (int i = 0; i < totalZumbis; i++) {
        int x = originX + zumbis[i].pos_atual.coluna * tileSize;
        int y = originY + zumbis[i].pos_atual.linha * tileSize;

        DrawCircle(x + tileSize / 2, y + tileSize / 2, tileSize / 2 - 2, LIME);
        DrawText("Z", x + 6, y + 4, 12, BLACK);
    }
}