# Projeto-Pratico
# para compilar o arquivo 
"gcc main.c mapa.c player.c zumbi.c -o jogo.exe -lraylib -lwinmm -lgdi32 -lopengl32"
e para rodar ele "./jogo.exe"
 o menu.c seria para fazer as telas e os salvamento do jogo.

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

Reinicialização dinâmica de entidades entre fases/mapas (reiniciarEntidadesDoMapa e carregarFase): limpeza segura da memória alocada (liberarKits e liberarZumbis) e reconstrução automática do entregador, kits e zumbis ao carregar qualquer mapa (mapa1.txt, mapa2.txt, mapa3.txt), tanto na progressão normal quanto nos atalhos de teste (F1, F2, F3).

Preservação de progresso entre fases: manutenção da pontuação acumulada e das vidas restantes do jogador ao avançar de fase, com bônus de +100 pontos ao concluir cada etapa.

Validação da saída ('S'): verificação de que todos os kits do mapa foram entregues (kitsPendentes <= 0) e que a pilha LIFO está vazia (topo < 0) antes de liberar a saída, exibindo aviso na tela caso o jogador tente sair com entregas pendentes.

Integração com o cronômetro da fase e condições de derrota: encerramento da partida por tempo esgotado (00:00) ou por perda de todas as vidas, bloqueando a movimentação e exibindo o motivo específico no overlay de GAME OVER.

Telas de feedback de progressão: overlays visuais para Fase Concluída (avançando com ENTER ou ESPAÇO) e Todas as Fases Concluídas (exibindo a pontuação final).


## Em Desenvolvimento / Próximos Passos

Remoção de variáveis globais remanescentes e integração completa do fluxo de telas/menus com o restante do projeto.

