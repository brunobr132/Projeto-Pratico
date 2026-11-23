#include "raylib.h"
#include <stdio.h>
#include <math.h>
// #include "pacman.h"
// #include "Fantasma.h"
// #include "menu2.h"
#include "mapa.h" 
#include "player.h"
#include "zumbi.h"

#define TILE 20
#define WIDTH 40
#define HEIGHT 20
#define SCREEN_WIDTH (WIDTH * 20)
#define SCREEN_HEIGHT (HEIGHT * 23)
#define MAP_ORIGIN_X ((SCREEN_WIDTH - WIDTH * TILE) / 2)
#define MAP_ORIGIN_Y 0
#define HUD_HEIGHT 60
#define TEMPO_PORTAO_ABERTO 4.0f
#define TEMPO_PORTAO_FECHADO 4.0f


typedef enum GameScreen{
    INICIAL,
    MENU,
    DESCRICAO,
    PLAYER,
    DIFICULDADE,
    SELECAO,
    SALVAR,
    CARREGAR,
    GAMEPLAY,
    FIM_DE_JOGO,
    REGRAS,
    PAUSA,
    VITORIA
} GameScreen;

typedef struct {
    Vector2 posicaoPlayer;
    Vector2 posicoesZombies[10];
    int numZombies;
    Vector2 Gates[4];
    int numGates;
    bool encontrouPlayer;
} InfoMapa;

GameScreen telaAtual = INICIAL;

const char initialmapa[HEIGHT][WIDTH] = {
    "########################################",
    "# P         a            b             #",
    "#                                   C  #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#      c           Z            a      #",
    "##D#########D###########D##########D####",
    "#     b                    Z           #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#   ######      #######      ######    #",
    "#           c                          #",
    "#                                      #",
    "#  A                   B               #",
    "#                                    S #",
    "########################################"
};

char mapa[HEIGHT][WIDTH];
bool portoesAbertos = false;
float inicioCicloPortoes = 0.0f;

void drawmapa(char mapa[HEIGHT][WIDTH], bool portoesAbertos);
void checarColisao(Vector2 pos, char mapa[HEIGHT][WIDTH], int *pontos, int *comeu, Sound sound);
// void updatePosition(Vector2 *pos, DIRECAO *direction, DIRECAO *buffer_direction, char mapa[HEIGHT][WIDTH], char Char);
// int avoidRotation(Vector2 *pos, DIRECAO *direction, DIRECAO *buffer_direction, char mapa[HEIGHT][WIDTH], char Char);
void desenharPlacar(int pontos);
void desenharVidas(int vidas);
InfoMapa processarMapa(char mapa[HEIGHT][WIDTH]);
void desenharTempo(int minutos, int segundos);
void desenharNivel(int mapaAtual);

float SPEED = 2.0f;

void atualizarPortoesPorTempo(void) {
    const float ciclo = TEMPO_PORTAO_ABERTO + TEMPO_PORTAO_FECHADO;
    const float tempoNoCiclo = fmodf((float)(GetTime() - inicioCicloPortoes), ciclo);
    portoesAbertos = tempoNoCiclo < TEMPO_PORTAO_ABERTO;
}

float tempoRestantePortoes(void) {
    const float ciclo = TEMPO_PORTAO_ABERTO + TEMPO_PORTAO_FECHADO;
    const float tempoNoCiclo = fmodf((float)(GetTime() - inicioCicloPortoes), ciclo);

    if (portoesAbertos) {
        return TEMPO_PORTAO_ABERTO - tempoNoCiclo;
    }

    return ciclo - tempoNoCiclo;
}

void drawmapa(char mapa[HEIGHT][WIDTH], bool portoesAbertos) {
    const Color fundo = {225, 229, 231, 255};
    const Color parede = {48, 69, 90, 255};
    const Color borda = {36, 55, 75, 255};
    const Color gradeFundo = {207, 213, 216, 255};
    const Color gradeParede = {59, 79, 99, 255};
    const int larguraMapa = WIDTH * TILE;
    const int alturaMapa = HEIGHT * TILE;

    DrawRectangle(MAP_ORIGIN_X - TILE, MAP_ORIGIN_Y - TILE,
                  larguraMapa + 2 * TILE, alturaMapa + 2 * TILE, borda);

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            const int x = MAP_ORIGIN_X + j * TILE;
            const int y = MAP_ORIGIN_Y + i * TILE;
            const bool paredeCelula = mapa[i][j] == '#';

            DrawRectangle(x, y, TILE, TILE, paredeCelula ? parede : fundo);
            DrawRectangleLines(x, y, TILE, TILE,
                               paredeCelula ? gradeParede : gradeFundo);
        }
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            const int x = MAP_ORIGIN_X + j * TILE;
            const int y = MAP_ORIGIN_Y + i * TILE;
            const char celula = mapa[i][j];
            Color corMarcador;

            switch (celula) {
                case 'P':
                    corMarcador = YELLOW;
                    break;
                case 'Z':
                    DrawCircle(x + TILE / 2, y + TILE / 2, TILE / 2 - 2, LIME);
                    DrawText("Z", x + 5, y + 2, 10, BLACK);
                    continue;
                case 'a':
                    corMarcador = (Color){105, 190, 211, 255};
                    break;
                case 'b':
                case 'B':
                    corMarcador = (Color){242, 165, 55, 255};
                    break;
                case 'c':
                    corMarcador = (Color){188, 151, 213, 255};
                    break;
                case 'C':
                    corMarcador = (Color){171, 126, 201, 255};
                    break;
                case 'D':
                    corMarcador = portoesAbertos
                        ? (Color){70, 190, 90, 255}
                        : (Color){255, 0, 0, 255};
                    break;
                case 'A':
                    corMarcador = (Color){112, 199, 186, 255};
                    break;
                case 'S':
                    corMarcador = (Color){181, 184, 186, 255};
                    break;
                default:
                    continue;
            }

            DrawRectangle(x + 2, y + 2, TILE - 4, TILE - 4, corMarcador);
            DrawText(TextFormat("%c", celula), x + 4, y + 2, 10, BLACK);
        }
    }
}

InfoMapa processarMapa(char mapa[HEIGHT][WIDTH]) {
    InfoMapa info = {0};

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            const Vector2 posicao = {
                (float)(MAP_ORIGIN_X + j * TILE + TILE / 2),
                (float)(MAP_ORIGIN_Y + i * TILE + TILE / 2)
            };

            if (mapa[i][j] == 'P') {
                info.posicaoPlayer = posicao;
                info.encontrouPlayer = true;
            } else if (mapa[i][j] == 'Z' && info.numZombies < 10) {
                info.posicoesZombies[info.numZombies++] = posicao;
            } else if (mapa[i][j] == 'D' && info.numGates < 4) {
                info.Gates[info.numGates++] = posicao;
            }
        }
    }

    return info;
}

void desenharHUD(
    int vidas, int pontos, int fase,
    int kitsPendentes, const char *carga, int cargaAtual, int capacidade,
    int portoesAbertos, int totalPortoes,
    int minutos, int segundos
) {
    const int largura = GetScreenWidth();
    const int y = GetScreenHeight() - HUD_HEIGHT;
    const Color fundo = {24, 40, 52, 255};
    const Color textoSecundario = {175, 190, 198, 255};

    DrawRectangle(0, y, largura, HUD_HEIGHT, fundo);

    DrawText(TextFormat("VIDAS %d", vidas), 4, y + 4, 11, RAYWHITE);
    DrawText(TextFormat("PONTOS %06d", pontos),
             (int)(largura * 0.10f), y + 4, 11, RAYWHITE);
    DrawText(TextFormat("FASE %d", fase),
             (int)(largura * 0.265f), y + 4, 11, RAYWHITE);
    DrawText(TextFormat("TEMPO %02d:%02d", minutos, segundos),
             (int)(largura * 0.35f), y + 4, 11, RAYWHITE);
    DrawText(TextFormat("KITS PENDENTES %d", kitsPendentes),
             (int)(largura * 0.49f), y + 4, 11, RAYWHITE);

    DrawText(TextFormat("CARGA: %s (%d/%d)", carga, cargaAtual, capacidade),
             4, y + 28, 9, textoSecundario);
    DrawText(TextFormat("PORTÕES: abertos %d/%d",
                        portoesAbertos, totalPortoes),
             (int)(largura * 0.17f), y + 28, 9, textoSecundario);
    DrawText("TAB: menu", (int)(largura * 0.39f), y + 28, 9, textoSecundario);
    DrawText("ESPAÇO: coletar / entregar",
             (int)(largura * 0.49f), y + 28, 9, textoSecundario);
}

int main(void){
    Mapa *mapaCarregado = carregarMapa("mapa1.txt");
    
    if (!mapaCarregado) {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                mapa[i][j] = initialmapa[i][j];
            }
        }
    } else {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                mapa[i][j] = mapaCarregado->celulas[i][j];
            }
        }
        liberarMapa(mapaCarregado);
    }
    
    InfoMapa infoMapa = processarMapa(mapa);
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RESIDENT EVIL");
    SetTargetFPS(60);
    InitAudioDevice();
    inicioCicloPortoes = (float)GetTime();
    SetRandomSeed((unsigned int)GetTime());

    Entregador player;
    Kit *kits = NULL;
    int totalKits = 0;
    inicializarPlayer(&player, mapa, &kits, &totalKits);
    int kitsPendentes = totalKits;
    Zumbi *zumbis = NULL;
    int totalZumbis = 0;
    inicializarZumbis(&zumbis, &totalZumbis, mapa);
    bool gameOver = false;
    float tempoRestanteFase = 240.0f;

    while(!WindowShouldClose()){
        if (IsKeyPressed(KEY_F1) ||
            (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_ONE))) {
            Mapa *novoMapa = carregarMapa("mapa1.txt");
            if (novoMapa) {
                for (int i = 0; i < HEIGHT; i++) {
                    for (int j = 0; j < WIDTH; j++) {
                        mapa[i][j] = novoMapa->celulas[i][j];
                    }
                }
                liberarMapa(novoMapa);
                infoMapa = processarMapa(mapa);
                inicioCicloPortoes = (float)GetTime();
            }
        }

        if (IsKeyPressed(KEY_F2) ||
            (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_TWO))) {
            Mapa *novoMapa = carregarMapa("mapa2.txt");
            if (novoMapa) {
                for (int i = 0; i < HEIGHT; i++) {
                    for (int j = 0; j < WIDTH; j++) {
                        mapa[i][j] = novoMapa->celulas[i][j];
                    }
                }
                liberarMapa(novoMapa);
                infoMapa = processarMapa(mapa);
                inicioCicloPortoes = (float)GetTime();
            }
        }
        if (IsKeyPressed(KEY_F3) ||
            (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_THREE))) {
            Mapa *novoMapa = carregarMapa("mapa3.txt");
            if (novoMapa) {
                for (int i = 0; i < HEIGHT; i++) {
                    for (int j = 0; j < WIDTH; j++) {
                        mapa[i][j] = novoMapa->celulas[i][j];
                    }
                }
                liberarMapa(novoMapa);
                infoMapa = processarMapa(mapa);
                inicioCicloPortoes = (float)GetTime();
            }
        }

        atualizarPortoesPorTempo();
        BeginDrawing();
        ClearBackground(WHITE);

        float dt = GetFrameTime();

        if (!gameOver && tempoRestanteFase > 0.0f) {
            tempoRestanteFase -= dt;
            if (tempoRestanteFase < 0.0f) {
                tempoRestanteFase = 0.0f;
            }
        }

        if (!gameOver) {
            atualizarMovimentoPlayer(&player, mapa, portoesAbertos, dt);
            atualizarZumbis(zumbis, totalZumbis, mapa, portoesAbertos, player.pos_atual, dt);
            gameOver = verificarColisaoZumbis(&player, kits, zumbis, totalZumbis);
        if (!gameOver) {
                    interagirComMapa(&player, mapa, kits, totalKits, &kitsPendentes);
                }
        }
        drawmapa(mapa, portoesAbertos);
        desenharPlayerEKits(&player, kits, totalKits, TILE, MAP_ORIGIN_X, MAP_ORIGIN_Y);
        desenharZumbis(zumbis, totalZumbis, TILE, MAP_ORIGIN_X, MAP_ORIGIN_Y);

        // Descobre o texto da carga atual para mostrar no HUD
        char textoCarga[20] = "vazia";
        if (player.topo >= 0) {
        int idTopo = player.pilha[player.topo];
        sprintf(textoCarga, "topo: %c", kits[idTopo].tipo);
        }

        if (gameOver) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 170});
        DrawText("GAME OVER - Sem vidas!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 20, 24, RED);
        DrawText("Pressione ESC para sair", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 15, 16, RAYWHITE);
    }   
        int minutos = (int)tempoRestanteFase / 60;
        int segundos = (int)tempoRestanteFase % 60;
        
        desenharHUD(player.vidas, player.pontos, 1, kitsPendentes, textoCarga,
                player.topo + 1, 3, portoesAbertos ? 4 : 0, 4,
                minutos, segundos);
        
        DrawText(
            TextFormat("PORTAO D: %s (%.1fs)",
                       portoesAbertos ? "ABERTO" : "FECHADO",
                       tempoRestantePortoes()),
            10,
            GetScreenHeight() - HUD_HEIGHT - 20,
            12,
            portoesAbertos ? GREEN : RED
        );
        EndDrawing();
    }
    
    liberarKits(kits);
    liberarZumbis(zumbis);
    CloseWindow();
    return 0;
}