# Navegador / celular

Site estático. Hospede esta pasta em HTTPS; não abra o index.html diretamente pelo sistema de arquivos. WebAssembly usa o mesmo núcleo C++20 do desktop. Atlas preparados a partir da referência do usuário por imagegen.

Recompilar: `scripts/build-web.sh` (Emscripten 3.1.69). Testar: `npm run test:web` com servidor em 127.0.0.1:8088. Licenças do projeto e de dependências estão no repositório principal.

GitHub Pages: Settings → Pages → Deploy from a branch → `gh-pages` / `/ (root)` → Save. Endereço esperado: https://aminhagranja-sketch.github.io/paulo/.

Inventário: botão Bolsa ou tecla I. Alimentos podem ser usados pela Bolsa.
Ovos aparecem somente após a derrota de inimigos; o save v2 mantém drops
pendentes e coletados. Saves v1 preservam o inventário e migram o estado do mapa.
A versão ainda é individual; cooperação e persistência de servidor estão pendentes.

As oito folhas originais do RAR são usadas na Web por atlas e metadados.
Para reimportar: `./scripts/build-game-art.sh` na raiz, com Python/Pillow.
A loja da vila aceita as melhorias existentes; portas mudam por proximidade.
Não há interiores nem multiplayer nesta versão.
