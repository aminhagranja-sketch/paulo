# Arquitetura e expansão

## Limites entre módulos

O núcleo (`granja_core`) não depende de SFML. `Vec`, `Player`, `Loot` e `Enemy` são dados simples. `Simulation` transforma entradas abstratas em estado. `Renderer` lê esse estado e desenha com SFML, sem controlar regras do jogo. `Game` converte teclado/mouse em `Input`, controla menus, relógio e persistência.

Esse desenho modular é a alternativa escolhida para um ECS: não há hierarquias polimórficas de entidades nem regras de gameplay acopladas ao desenho. Uma migração futura para registry/component stores pode preservar as interfaces de entrada e apresentação.

## Mundo e streaming

- Tiles: 64 unidades; chunks: 16×16 tiles, 1024×1024 unidades.
- Seed padrão: `20261009`; hash de coordenadas estável e funções trigonométricas geram terreno e recompensas determinísticos, inclusive em coordenadas negativas.
- Só nove chunks em torno do jogador ficam carregados para simulação. Chunks distantes são descarregados e seus registros compactos ficam em memória: IDs dos itens coletados e snapshots dos inimigos.
- Ao voltar, o chunk é reconstruído pela seed e recebe o estado armazenado. A IA distante congela; não recebe atualizações em segundo plano.
- O save inclui áreas carregadas e descarregadas. Memória dos registros e tamanho do arquivo crescem com a exploração; não há streaming de saves para disco durante a partida.
- Coordenadas de movimento limitadas a ±999.000 unidades para manter precisão float. O leitor aceita até 20.000 registros de chunks e arquivos de até 32 MB; não se promete exploração infinita.
- O minimapa amostra o terreno em torno do jogador e mostra jogador, ninho e chefe. Não há neblina de exploração.

`World::terrain` define bloqueios estáticos; `World::blocked` faz círculo/círculo para árvores, pedras e colunas, e círculo/AABB para água e casa. `World::move` divide deslocamentos em passos de até oito unidades e resolve os eixos separadamente, evitando atravessar obstáculos com esquivas. Entidades vivas não bloqueiam umas às outras.

O roteiro do mundo tem três pontos fixos: ninho, arena e pomar secreto. Os testes verificam que os destinos e recursos da missão são alcançáveis na seed padrão.

## Loop e combate

`Game` acumula tempo e executa a simulação em passos de 1/60 s, limitando atraso acumulado por frame. Pulsos de esquiva, alimento e interação são consumidos em um único passo. Movimento, ataque segurado e mira são entradas contínuas.

- Movimento diagonal normalizado; câmera interpolada.
- Ataque: alcance 95 (112 para chefe), cone frontal, cooldown 0,34 s, custo 12 de energia.
- Esquiva: 0,20 s, velocidade 620, custo 30, invulnerabilidade 0,25 s.
- Dano sofrido concede 0,70 s de invulnerabilidade; respawn concede três segundos.
- Energia regenera 25/s, com máximo 100.
- Recompensas de morte e coleta têm marcadores para impedir duplicação.

IA: `Wander → Chase → Windup → Recover`; `Dead` ao zerar vida. Rivais se movem pelo entorno de sua origem, perseguem dentro de 320 unidades, anunciam golpes e respeitam descanso e invulnerabilidade do jogador. O chefe usa alcance, vida, dano e tempos próprios. Há leash de 650 unidades; o ninho é seguro. A perseguição usa movimento direto com colisão, sem A*: obstáculos podem interromper a perseguição.

## Persistência

`Save::write` serializa formato versão 1, grava `.tmp`, fecha o stream e substitui o destino. POSIX usa rename; Windows usa `MoveFileExW` com replace/write-through. `Save::read` constrói uma simulação candidata, valida tipos, intervalos, coordenadas, IDs e estados e só então troca a simulação atual. Timers de ataque, animações, efeitos e cooldowns não são persistidos.

Uma mudança nas regras de geração ou na identidade das entidades exige migrar o formato ou incrementar `version`: IDs locais são derivados da geração e não devem ser reutilizados sem migração.

## Adicionar conteúdo

- Novo item: acrescente `LootKind`, geração em `World`, recompensa em `Simulation::collect`, ícone em `Renderer::icon` e teste de persistência.
- Novo inimigo: acrescente dados de espécie e parâmetros ao modelo e geração. Preserve a marca `rewarded` e teste morte/recarga.
- Missões adicionais: extraia `readyForBoss()` para uma lista de objetivos, persistindo flags e recompensas concedidas.
- Arte por sprites: substitua `Renderer::chicken`, `tree` e `icon` por atlas/texturas; o núcleo continua igual.
- A*: acrescente um serviço de navegação sobre `terrain` e cacheie caminhos da IA por chunk.
- Áudio: um módulo de apresentação pode consumir eventos de gameplay sem contaminar o núcleo.

Os números de balanceamento estão hoje nos structs e em `Simulation.cpp`. Extrair uma configuração de conteúdo JSON é uma evolução possível, com validação explícita e versão própria.

## Toque e navegador (v1.1)

`TouchControls` é um módulo C++ independente de SFML, compartilhado pelo desktop e WebAssembly. Cada pointer tem proprietário próprio; levantar o dedo de ataque mantém o analógico ativo. Há zona morta de 12%, intensidade analógica, raio limitado entre 40 e 76 pixels e reset ao redimensionar/cancelar. Botões secundários geram pulsos de um passo.

`NativeTouch` adapta `WM_POINTER` em Windows, preserva o WndProc do SFML e evita promover os mesmos toques a cliques duplicados. Outros sistemas usam eventos SFML ou mouse. O backend Windows foi compilado, mas não validado em hardware touch.

`src/web/Bridge.cpp` exporta o núcleo em WebAssembly. `web/app.js` recebe snapshots do estado, desenha no Canvas, converte Pointer Events em chamadas ao controlador C++ e salva JSON em localStorage usando o mesmo serializador do desktop. O WASM inclui colisões, IA, combate, progressão e geração; JavaScript cuida da apresentação e do ciclo de vida do navegador. Não há backend ou credenciais. O bundle estático é pré-compilado e versionado; execute `scripts/build-web.sh` após mudanças no núcleo.

Os dois atlas têm grade 4×4. Caminhada: baixo, esquerda, cima, direita; ações: ataque, esquiva, dano, vitória. Ações laterais espelham para esquerda; caminhada usa cada direção. Os atlas derivados foram preparados por imagegen a partir da referência, e não recortados exatamente do original.
