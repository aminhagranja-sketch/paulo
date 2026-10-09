# MEU GALINHEIRO ADVENTURE — DESENVOLVIMENTO COMPLETO COM SPRITES

Você é um desenvolvedor sênior especializado em jogos RPG multiplayer, C++, WebAssembly, animações 2D/2.5D, inteligência artificial e desenvolvimento de jogos para navegadores.

Sua missão é desenvolver o jogo **Meu Galinheiro Adventure**, utilizando obrigatoriamente os sete arquivos PNG fornecidos.

Não quero apenas uma demonstração visual. Quero um jogo funcional, organizado, expansível e preparado para publicação na Web.

## 1. ARQUIVOS FORNECIDOS

Os arquivos estão disponíveis em `assets/source/`.

Identifique-os pelo conteúdo visual:

1. `e2b20b93-8799-4092-a400-14b2df516e3a.png` — Baús animados de diferentes raridades.
2. `3e7f225e-5979-4f0f-b6cb-e4f7544e7836.png` — Casas, celeiros e lojas com portas em diferentes estados.
3. `4f1a5112-b608-47c7-aaf7-6be980fe74d9.png` — Árvores com variações de movimento.
4. `591f28fd-06f3-46d7-bce3-11dfd7e2e488.png` — Pedras, flores e objetos decorativos.
5. `15681c87-bdbc-446b-a3c2-c1d3853e999a.png` — Cobra com animações de movimentação, ataque, veneno e derrota.
6. `bef336d9-407a-4ad2-a747-f4ff3616e2ba.png` — Raposa com animações de corrida, ataque e derrota.
7. `ca70ca01-1579-450b-9d26-5dd3eab5ecb5.png` — Pintinhos com animações de movimento e diferentes ações.

Preserve a identidade visual cartoon HD dos arquivos.

## 2. PROCESSAMENTO AUTOMÁTICO DOS SPRITES

Criar um sistema de processamento de sprite sheets.

Requisitos:

- Ler todos os PNGs mantendo o canal alfa.
- Identificar os quadros visuais.
- Recortar os quadros em imagens individuais.
- Preservar transparência e detalhes.
- Remover resíduos visuais indesejados somente quando necessário.
- Evitar cortar partes dos personagens.
- Padronizar o ponto de origem de cada animação.
- Gerar atlas de texturas otimizados.
- Gerar metadados JSON de cada animação.
- Organizar os arquivos por categoria.

ATENÇÃO: as imagens não possuem necessariamente uma grade perfeitamente uniforme. Não dividir cegamente por linhas e colunas.

Permitir definir manualmente as regiões de recorte quando a detecção automática não for confiável.

Não considerar diferenças de pose como animações válidas sem verificar a continuidade visual.

Organização esperada:

assets/
  source/
  processed/
    chests/
    buildings/
    trees/
    environment/
    enemies/
      snake/
      fox/
    characters/
      chicks/
  atlases/
  animations/

## 3. TECNOLOGIA

O jogo deverá ser executado diretamente pelo navegador.

Arquitetura recomendada:

- C++20 para lógica de gameplay e sistemas que se beneficiem de código nativo.
- Emscripten para compilar módulos C++ em WebAssembly.
- Babylon.js com TypeScript para renderização e integração Web.
- WebSocket para multiplayer.
- Servidor autoritativo para combate, drops e economia.
- PostgreSQL para persistência.

O cliente deverá funcionar em computadores e celulares.

Não tentar executar diretamente C++ nativo no navegador.

## 4. MUNDO DO JOGO

Criar um mundo aberto cartoon com:

- Vila inicial.
- Fazendas.
- Florestas.
- Campos de milho.
- Caminhos de pedra.
- Jardins.
- Casas.
- Celeiros.
- Lojas.
- Áreas de combate.
- Regiões perigosas.
- Arena do chefão final.

Usar os sprites fornecidos para compor o ambiente.

Como são imagens 2D, implementar inicialmente um mundo 2.5D, utilizando planos transparentes, ordenação de profundidade e câmera inclinada em terceira pessoa.

A câmera deverá acompanhar a galinha por trás, dentro das limitações visuais dos sprites.

Não inventar visões traseiras ou laterais que não existam nos arquivos. Se forem necessárias para uma câmera livre, preparar uma etapa posterior de criação de modelos 3D reais.

## 5. PERSONAGEM PRINCIPAL

Criar uma galinha jogável, utilizando inicialmente um personagem provisório até que sejam fornecidos os sprites definitivos da galinha adulta.

Movimentos:

- Parado.
- Caminhar.
- Correr.
- Virar.
- Pular.
- Atacar.
- Receber dano.
- Morrer.
- Coletar.
- Interagir.

Controles:

Computador:
- WASD para movimentação.
- Mouse para câmera.
- Espaço para pular.
- Clique esquerdo para atacar.
- E para interagir.
- I para inventário.

Celular:
- Joystick virtual.
- Botão de ataque.
- Botão de pulo.
- Botão de interação.
- Botão de inventário.

Criar controles responsivos.

## 6. ANIMAÇÃO DAS COBRAS

Utilizar os quadros da cobra fornecida.

Estados:

IDLE: parada, observando.
MOVE: rastejando.
CHASE: perseguindo o jogador.
ATTACK: mordida.
POISON: ataque venenoso.
HIT: recebendo dano.
DEATH: derrota.
RESPAWN: reaparecimento.

A cobra deverá:

- Detectar galinhas próximas.
- Perseguir o jogador.
- Atacar quando estiver ao alcance.
- Aplicar veneno temporário.
- Recuar ocasionalmente.
- Ser derrotada ao perder toda a vida.

Usar máquina de estados para controlar sua IA.

## 7. ANIMAÇÃO DAS RAPOSAS

Utilizar os sprites da raposa.

Estados:

IDLE.
WALK.
RUN.
CHASE.
BITE.
SPECIAL_ATTACK.
HIT.
DEATH.

Comportamentos:

- Patrulhar florestas.
- Identificar galinhas próximas.
- Correr atrás do jogador.
- Atacar com mordidas.
- Realizar investidas.
- Esquivar de determinados ataques.
- Recuar quando estiver com pouca vida.

As raposas deverão ser mais rápidas que as cobras.

## 8. SISTEMA DE PINTINHOS

Utilizar os sprites de pintinhos para criar personagens secundários.

Animações:

- Parado.
- Caminhando.
- Correndo.
- Pulando.
- Bicando.
- Comendo.
- Dormindo.
- Feliz.
- Assustado.
- Saindo do ovo.

Os pintinhos poderão circular pela vila.

Criar comportamentos simples de IA.

Futuramente, permitir que determinados pintinhos sejam companheiros do jogador.

## 9. BAÚS ANIMADOS

Utilizar os sprites de baús fornecidos.

Raridades:

- Comum.
- Raro.
- Épico.
- Lendário.
- Natural.
- Infernal.

Cada baú terá:

- Animação fechado.
- Animação começando a abrir.
- Animação abrindo.
- Efeito luminoso.
- Exibição da recompensa.
- Animação final.

Ao interagir com um baú, o servidor deverá validar sua disponibilidade e gerar a recompensa.

Recompensas possíveis:

- Grãos.
- Ração.
- Ovos.
- Equipamentos.
- Materiais especiais.

Baús comuns poderão aparecer em locais predefinidos do mapa.

Impedir que o mesmo baú seja resgatado repetidamente de maneira indevida.

## 10. CASAS, CELEIROS E LOJAS

Utilizar os sprites fornecidos para criar construções interativas.

CASAS:
- Porta fechada.
- Porta parcialmente aberta.
- Porta totalmente aberta.
- Porta fechando.

CELEIROS:
- Portões fechados.
- Portões abrindo.
- Portões abertos.
- Portões fechando.

LOJA:
- Fachada fechada.
- Fachada aberta.
- Interior iluminado.
- Interface de compra.

Quando o jogador se aproximar, exibir a opção "Entrar".

Ao interagir:
1. Reproduzir animação da porta.
2. Aguardar a abertura.
3. Permitir entrada ou transição de ambiente.
4. Fechar a porta quando apropriado.

Usar os quadros existentes, mantendo as construções alinhadas durante a animação.

## 11. ÁRVORES ANIMADAS

Utilizar os sprites das árvores.

Criar:
- Árvores verdes.
- Árvores floridas.
- Árvores de outono.
- Palmeiras.
- Pinheiros.

Animações:
- Vento fraco.
- Vento moderado.
- Vento forte.
- Folhas caindo.
- Árvore sendo derrubada, quando houver quadros adequados.

Utilizar animações suaves.

As árvores devem possuir colisão no tronco, mas não bloquear completamente a câmera com suas copas.

## 12. PEDRAS E FLORES

Utilizar os sprites de decoração para construir:

- Caminhos.
- Cercas.
- Muros.
- Jardins.
- Pedras.
- Arcos.
- Escadas.
- Vasos.
- Flores.
- Arbustos.
- Lanternas.

Adicionar pequenas animações de movimento nas flores.

Usar os elementos como decoração e obstáculos.

Evitar excesso de objetos para manter bom desempenho.

## 13. SISTEMA DE COMBATE

O jogador poderá enfrentar:

- Cobras.
- Raposas.
- Escorpiões, quando os sprites forem adicionados.
- Galinhas selvagens.
- Galos inimigos.
- Chefões.

Criar:

- Ataque básico.
- Ataque especial.
- Defesa.
- Esquiva.
- Barra de vida.
- Dano crítico.
- Experiência.
- Efeitos de status.

Cada inimigo deverá possuir vida, ataque, defesa, velocidade e nível.

## 14. SISTEMA DE OVOS

Os ovos serão a moeda principal da progressão.

IMPORTANTE:

Ovos não aparecem espontaneamente no chão.

Somente inimigos derrotados poderão dropar ovos.

Quando um inimigo morrer:
1. Executar animação de derrota.
2. Calcular recompensa no servidor.
3. Gerar o drop.
4. Permitir coleta pelo jogador elegível.
5. Atualizar inventário.
6. Salvar a transação.

Ovos comuns terão valor de referência de R$ 0,002 por unidade, correspondente a 0,2 centavo.

O ovo de ouro terá valor de referência de R$ 10,00.

Esses valores serão apenas parâmetros de economia do jogo nesta implementação, sem criar automaticamente saques ou conversões para dinheiro real.

O Grande Ovo será exclusivo da missão final.

## 15. LOJA DE EQUIPAMENTOS

Criar loja para comprar:

- Capacetes.
- Armaduras.
- Esporas.
- Botas.
- Amuletos.
- Itens especiais.

Cada equipamento deverá modificar atributos da galinha.

Atributos:
- Vida.
- Ataque.
- Defesa.
- Velocidade.
- Crítico.
- Esquiva.
- Resistência a veneno.

Criar interface visual de equipamentos.

## 16. PROGRESSÃO DE 30 DIAS

O jogo deverá ser balanceado para aproximadamente 30 horas de gameplay ativo, equivalentes a cerca de 30 dias para um jogador regular.

Criar regiões progressivas.

Inimigos iniciais:
- Cobras comuns.
- Galinhas selvagens.

Inimigos intermediários:
- Raposas.
- Cobras venenosas.
- Escorpiões.

Inimigos avançados:
- Raposas sombrias.
- Cobras-rei.
- Escorpiões imperiais.
- Guardiões.

Chefão final:
REI DOS GALOS.

O Grande Ovo só poderá ser obtido após derrotar o chefão final e cumprir os requisitos da missão.

## 17. MULTIPLAYER COOPERATIVO

Os jogadores entrarão simultaneamente no mesmo mundo.

Regras obrigatórias:

- Todos podem se enxergar.
- Todos podem explorar.
- Todos podem enfrentar inimigos.
- Todos podem participar de batalhas cooperativas.
- Jogadores não podem causar dano uns aos outros.
- PvP totalmente desativado.
- Recompensas individuais.
- Inventários separados.
- Progressão individual.
- Persistência no servidor.

Criar uma instância inicial para até 20 jogadores, com capacidade configurável.

Implementar sincronização em tempo real.

## 18. OTIMIZAÇÃO

Priorizar:
- 60 FPS em computadores compatíveis.
- Meta de 30 FPS em celulares intermediários.
- Atlas de sprites.
- Carregamento por região.
- Texturas comprimidas.
- Animações eficientes.
- Limitação de objetos simultâneos.
- Sincronização por proximidade.
- Redução de processamento de IA fora do alcance dos jogadores.

## 19. PRIMEIRA ENTREGA FUNCIONAL

Desenvolver primeiro uma versão jogável contendo:

1. Mapa inicial com árvores, pedras e flores.
2. Casas, celeiro e loja.
3. Portas animadas.
4. Galinha controlável.
5. Pintinhos circulando.
6. Cobras com IA.
7. Raposas com IA.
8. Sistema de combate.
9. Ovos dropados por inimigos.
10. Baús animados.
11. Inventário.
12. Loja funcional.
13. Dois jogadores simultâneos.
14. PvP desativado.
15. Salvamento de progresso.

Testar em duas janelas independentes do navegador.

## 20. INSTRUÇÕES FINAIS

Não substitua os sprites fornecidos por emojis, quadrados ou desenhos genéricos.

Não apresente apenas um mockup.

Implemente os sistemas reais e reutilize as artes fornecidas.

Se algum sprite não permitir determinada animação, documente a limitação e implemente uma transição provisória sem inventar quadros inexistentes.

Organize o projeto para receber novas artes posteriormente.

Crie scripts de inicialização, documentação e testes.

Execute a compilação e corrija os erros encontrados.

Priorize a primeira versão jogável antes de implementar funcionalidades avançadas.

O resultado deverá ser um RPG multiplayer cartoon, visualmente bonito, funcional e preparado para integrar o site Meu Galinheiro.