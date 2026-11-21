#include "raylib.h"
#include <stdio.h>
#include <math.h>
// #include "pacman.h"
// #include "Fantasma.h"
// #include "menu2.h"
#include "mapa.h" 

#define TILE 20
#define WIDTH 40
#define HEIGHT 20
#define SCREEN_WIDTH (WIDTH * 20)
#define SCREEN_HEIGHT (HEIGHT * 23)
#define MAP_ORIGIN_X ((SCREEN_WIDTH - WIDTH * TILE) / 2)
#define MAP_ORIGIN_Y 0
#define HUD_HEIGHT 60

float temporest = 120.0f;


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

void drawmapa(char mapa[HEIGHT][WIDTH]);
void checarColisao(Vector2 pos, char mapa[HEIGHT][WIDTH], int *pontos, int *comeu, Sound sound);
// void updatePosition(Vector2 *pos, DIRECAO *direction, DIRECAO *buffer_direction, char mapa[HEIGHT][WIDTH], char Char);
// int avoidRotation(Vector2 *pos, DIRECAO *direction, DIRECAO *buffer_direction, char mapa[HEIGHT][WIDTH], char Char);
void desenharPlacar(int pontos);
void desenharVidas(int vidas);
InfoMapa processarMapa(char mapa[HEIGHT][WIDTH]);
void desenharTempo(int minutos, int segundos);
void desenharNivel(int mapaAtual);

float SPEED = 2.0f;

void drawmapa(char mapa[HEIGHT][WIDTH]) {
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
    int vidas, int pontos, int fase, int minutos, int segundos,
    int kitsPendentes, const char *carga, int cargaAtual, int capacidade,
    int portoesAbertos, int totalPortoes
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
    Mapa *mapaCarregado = carregarMapa("mapa.txt");

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
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PAC - MAN");
    SetTargetFPS(60);
    InitAudioDevice();

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(WHITE);

        drawmapa(mapa);
        desenharHUD(3, 0, 1, 4, 0, 6, "vazia", 0, 3, 0, 3);

        EndDrawing();
    }
}