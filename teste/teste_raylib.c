#include "raylib.h"
#include <string.h>
#define LARGURA 800
#define ALTURA 800
#define TAMANHO_CELULA 20
#define QUANTIDADE_CELULAS 40
// comando para rodar :gcc teste_raylib.c -o teste_raylib -lraylib -lwinmm -lgdi32 -lopengl32
// ./teste_raylib.exe


int main(void){
char texto[50] = "Use as setas para mover";
//Inicializações
InitWindow(LARGURA, ALTURA, "Teste"); //Inicializa janela (tamanho e título)
SetTargetFPS(60);// Ajusta a execução do jogo para 60 frames por segundo
Vector2 squarePosition = { (float)LARGURA/2, (float)ALTURA/2 };

int mapa[QUANTIDADE_CELULAS][QUANTIDADE_CELULAS];
for (int linha = 0; linha < QUANTIDADE_CELULAS; linha++){
    for (int coluna = 0; coluna < QUANTIDADE_CELULAS; coluna++){
        mapa[linha][coluna] = GetRandomValue(0, 3) == 0 ? 1 : 0;
    }
}

int celulaInicialX = (int)(squarePosition.x / TAMANHO_CELULA);
int celulaInicialY = (int)(squarePosition.y / TAMANHO_CELULA);
mapa[celulaInicialY][celulaInicialX] = 0;

//Laço principal do jogo
while (!WindowShouldClose()) // Detecta botão de fechamento da janela ou ESC
{
    int proximaColuna = (int)(squarePosition.x / TAMANHO_CELULA);
    int proximaLinha = (int)(squarePosition.y / TAMANHO_CELULA);
    bool tentouMover = false;

    if (IsKeyPressed(KEY_RIGHT)){
        proximaColuna++;
        tentouMover = true;
    } else if (IsKeyPressed(KEY_LEFT)){
        proximaColuna--;
        tentouMover = true;
    } else if (IsKeyPressed(KEY_UP)){
        proximaLinha--;
        tentouMover = true;
    } else if (IsKeyPressed(KEY_DOWN)){
        proximaLinha++;
        tentouMover = true;
    }

    if (tentouMover){
        if (proximaColuna < 0 || proximaColuna >= QUANTIDADE_CELULAS ||
            proximaLinha < 0 || proximaLinha >= QUANTIDADE_CELULAS){
            strcpy(texto, "Limite do mapa");
        } else if (mapa[proximaLinha][proximaColuna] == 1){
            strcpy(texto, "Obstaculo: movimento bloqueado");
        } else {
            squarePosition.x = (float)(proximaColuna * TAMANHO_CELULA);
            squarePosition.y = (float)(proximaLinha * TAMANHO_CELULA);
            strcpy(texto, "Use as setas para mover");
        }
    }

    // Atualiza a representação visual a partir do estado do jogo
    BeginDrawing(); //Inicia o ambiente de desenho na tela
    ClearBackground(RAYWHITE); //Limpa a tela e define cor de fundo

    for (int linha = 0; linha < QUANTIDADE_CELULAS; linha++){
        for (int coluna = 0; coluna < QUANTIDADE_CELULAS; coluna++){
            if (mapa[linha][coluna] == 1){
                DrawRectangle(
                    coluna * TAMANHO_CELULA,
                    linha * TAMANHO_CELULA,
                    TAMANHO_CELULA,
                    TAMANHO_CELULA,
                    LIGHTGRAY
                );
            }
        }
    }

    for (int posicao = 0; posicao <= QUANTIDADE_CELULAS; posicao++){
        int coordenada = posicao * TAMANHO_CELULA;
        DrawLine(coordenada, 0, coordenada, ALTURA, GRAY);
        DrawLine(0, coordenada, LARGURA, coordenada, GRAY);
    }

    DrawCircle(
        (int)squarePosition.x + TAMANHO_CELULA / 2,
        (int)squarePosition.y + TAMANHO_CELULA / 2,
        TAMANHO_CELULA / 2 - 2,
        RED
    );
    DrawText(texto, 10, 10, 20, DARKBLUE);
    EndDrawing(); //Finaliza o ambiente de desenho na tela
    }
    
    CloseWindow(); // Fecha a janela e o contexto OpenGL
    return 0;
}