# Projeto-Pratico
# para compilar o arquivo 
"gcc main.c mapa.c -o jogo.exe -lraylib -lwinmm -lgdi32 -lopengl32"
e para rodar ele "./jogo"
# e para compilar mais arquivos 
tipo player.c ou zumbi.c adicionar entre o gcc e o -o
exemplo "gcc main.c mapa.c player.c zumbi.c -o jogo.exe -lraylib -lwinmm -lgdi32 -lopengl32"
# o menu.c seria para fazer as telas e os salvamento do jogo.

# MAPA

 para trocar o mapa o  atalho e F1-F3 ou Ctrl+Shitf+1-3


## João:

## Funcionalidades Implementadas
Entregador, Pilha LIFO e Kits (`player.h`, `player.c`):
Movimentação discreta por célula (`WASD` ou setas) com intervalo de passo ajustado pela quantidade de kits carregados (até 3).
Coleta (`a`, `b`, `c`) e entrega nos abrigos correspondentes (`A`, `B`, `C`) usando a tecla `ESPAÇO` e pilha LIFO (apenas o kit do topo é avaliado na entrega).
Os kits transportados são devolvidos para o chão na posição inicial e reset do jogador ao sofrer dano.

Zumbis e Colisões (`zumbi.h`, `zumbi.c`):
Movimentação discreta a cada `0.5s` (`2 blocos/s`), respeitando paredes e portões fechados sem alterar os marcadores estáticos do terreno.
Patrulha aleatória fora do raio de aproximação, perseguição por distância de Manhattan dentro do raio definido e prevenção de sobreposição entre zumbis.
Detecção de colisão (mesma célula ou cruzamento), aplicando penalidade de vida/pontos e verificando condição de Game Over.

## Em Desenvolvimento / Próximos Passos
Condições de vitória na saída (`S`) após a entrega dos kits pendentes e controle de tempo limite da fase.
Remoção de variáveis globais remanescentes e integração completa do fluxo de telas/menus com o restante do projeto.

# para compilar o arquivo 
"gcc main.c mapa.c player.c zumbi.c -o jogo.exe -lraylib -lwinmm -lgdi32 -lopengl32"
e para rodar ele "./jogo.exe"