# Importação das sete folhas originais

O pedido completo está em [REQUEST-MULTIPLAYER.md](REQUEST-MULTIPLAYER.md).
Os PNGs originais estão pendentes. O jogo existente continua funcional; ainda
não implementa a nova entrega cooperativa, câmera 2.5D ou servidor PostgreSQL.

## Instalação e detecção

Requer Python 3.10+ e Pillow:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r tools/requirements.txt
.venv/bin/python tools/sprite_pipeline.py inspect --output assets/animations/review.json
```

No Windows, use `.venv\Scripts\python.exe`.

| Categoria | Arquivo em assets/source |
|---|---|
| Baús | e2b20b93-8799-4092-a400-14b2df516e3a.png |
| Construções | 3e7f225e-5979-4f0f-b6cb-e4f7544e7836.png |
| Árvores | 4f1a5112-b608-47c7-aaf7-6be980fe74d9.png |
| Decoração | 591f28fd-06f3-46d7-bce3-11dfd7e2e488.png |
| Cobras | 15681c87-bdbc-446b-a3c2-c1d3853e999a.png |
| Raposas | bef336d9-407a-4ad2-a747-f4ff3616e2ba.png |
| Pintinhos | ca70ca01-1579-450b-9d26-5dd3eab5ecb5.png |

A detecção usa componentes conectados de alfa, com vizinhança de oito pixels.
Não presume uma grade. Componentes pequenos são omitidos da proposta, mas seus
pixels permanecem intactos no arquivo original e em qualquer recorte manual.
Caudas, sombras e efeitos separados podem aparecer como regiões distintas:
una-os manualmente em um retângulo antes de aprovar. Fundos opacos exigem
recortes manuais; não há remoção automática de fundo que possa apagar detalhes.

## Revisão obrigatória

O manifesto contém `reviewed: false` em cada folha. Confira as regiões no PNG
original; ajuste `rect: [x, y, largura, altura]` e dê IDs descritivos aos quadros.
Defina `origin: [x, y]` em pixels relativos ao recorte, usando o mesmo ponto dos
pés/tronco em toda sequência. A origem pode ficar fora do recorte para manter
alinhamento em saltos. Somente após verificar continuidade, defina animações:

```json
"animations": {
  "walk_right": {"frames": ["walk_01", "walk_02", "walk_03"], "fps": 8, "loop": true}
}
```

Não inferir animações a partir de poses diferentes. Não atribuir orientações
inexistentes. Uma ação sem sequência adequada pode usar um quadro estático
com transição temporizada, documentada no manifesto. Marque `reviewed: true`
após a revisão visual de cada folha.

## Recortes e atlas

```sh
.venv/bin/python tools/sprite_pipeline.py pack --manifest assets/animations/review.json
python3 -m unittest discover -s tests -p 'test_sprite_pipeline.py'
```

A publicação é bloqueada se faltarem categorias, referências, revisão, ou se a
fonte mudar de SHA-256. Todos os recortes são validados antes de gravar saídas.
O manifesto de revisão existente nunca é sobrescrito pela detecção.

Saídas: `assets/processed/<categoria>/<quadro>.png`,
`assets/atlases/<categoria>/atlas-N.png` e
`assets/animations/<categoria>.json`. O empacotamento usa múltiplas páginas
quando necessário, preserva RGBA sem redimensionamento e adiciona margem de
quatro pixels entre quadros com extrusão das bordas. PNG usa compressão sem
perdas; o carregador futuro deve utilizar as dimensões e origens do JSON.

A detecção não avalia artisticamente a continuidade nem separa objetos que se
tocam. Até receber e revisar os PNGs, nenhum atlas dessas sete fontes pode ser
considerado aprovado ou integrado ao jogo.
