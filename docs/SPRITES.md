# Artes originais integradas à Web

O RAR fornecido contém as sete folhas do [pedido](REQUEST-MULTIPLAYER.md) e uma
folha adicional da galinha adulta. Todas foram extraídas e preservadas em
`assets/source/`, sem recriar personagens ou gerar poses.

| Categoria | PNG original | Quadros usados |
|---|---|---:|
| Baús | e2b20b93-8799-4092-a400-14b2df516e3a.png | 42 |
| Construções | 3e7f225e-5979-4f0f-b6cb-e4f7544e7836.png | 15 |
| Árvores | 4f1a5112-b608-47c7-aaf7-6be980fe74d9.png | 21 |
| Decoração | 591f28fd-06f3-46d7-bce3-11dfd7e2e488.png | 12 |
| Cobras | 15681c87-bdbc-446b-a3c2-c1d3853e999a.png | 25 |
| Raposas | bef336d9-407a-4ad2-a747-f4ff3616e2ba.png | 15 |
| Pintinhos | ca70ca01-1579-450b-9d26-5dd3eab5ecb5.png | 17 |
| Galinha adulta | 5462a363-df7e-4da6-b2ce-1a715d75e117.png | 40 |

## Reproduzir

Requer Python 3.10+ e Pillow 12.3.0. Instale em um ambiente virtual se necessário:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r tools/requirements.txt
```

Use Python com Pillow no PATH e execute:

```sh
./scripts/build-game-art.sh
python3 -m unittest discover -s tests -p 'test_sprite_pipeline.py'
```

O script usa `assets/animations/review.json`, gera recortes individuais em
`assets/processed/`, dez páginas de atlas em `assets/atlases/` e metadados JSON
em `assets/animations/`. Também copia atlas e metadados para `web/assets/`.
`web/sprites.js` carrega as oito categorias e usa coordenadas, origens, FPS e
sequências desse JSON. O navegador não baixa as folhas originais ou recortes
individuais: usa somente os atlas.

## Recortes e transparência

As regiões irregulares foram conferidas manualmente nas fontes e no jogo.
Não foi imposta uma grade universal. Cada quadro tem `rect` e `origin` próprios.
As sequências usam quadros existentes; ações sem orientações alternativas
utilizam a pose disponível ou espelhamento horizontal.

As sete folhas RGBA tinham resíduos de fundo: a opção `cleanBorderAlpha: 150`
remove somente pixels com alfa inferior a 150 conectados à borda do recorte.
Detalhes internos com alfa baixo permanecem intactos. A galinha adulta é RGB,
com fundo preto: `removeBorderBlack: true` remove preto próximo conectado à
borda, preservando olhos e detalhes pretos internos. Nenhuma fonte é alterada.
Essas operações são aplicadas somente aos recortes derivados.

A margem entre quadros e a extrusão das bordas reduzem vazamento de texturas.
O empacotamento usa múltiplas páginas sem redimensionar os pixels de origem.
Os SHA-256 do manifesto impedem importar silenciosamente fontes diferentes.
Todos os inputs são validados antes da geração das saídas.

Para novas fontes, `python3 tools/sprite_pipeline.py inspect --output NOVO.json`
gera propostas por componentes conectados de alfa. A revisão manual permanece
obrigatória; não sobrescreve o manifesto aprovado.

## Comportamento integrado e limites

- A galinha Web usa a folha adulta original em quatro direções, ataque, esquiva,
  dano e vitória. Os atlas gerados anteriormente continuam na versão SFML.
- Árvores usam quadros de vento e diminuem opacidade quando encobrem o jogador.
- Pedras, flores e colunas usam decoração original; o mundo mantém suas colisões.
- Casa, celeiro e loja mudam de estado de porta por proximidade. A loja permite
  comprar as melhorias existentes com validação de distância e saldo no C++.
- Pintinhos passeiam em trajetos simples na vila. Sem interação ou companheiros.
- Cobras e raposas usam animações de movimentação, ataque, dano e derrota;
  raposas perseguem mais rápido que cobras. Veneno, investidas e esquiva de
  inimigos ainda não fazem parte da IA atual.
- Baús têm seis estilos e sequência de abertura após a coleta validada pela
  simulação local. A coleta atual ocorre por proximidade, sem servidor.
- Construções adicionais são visuais; não há interiores ou transição de área.
- A apresentação continua top-down Canvas 2D. Multiplayer, PostgreSQL e câmera
  Babylon 2.5D permanecem pendentes.
