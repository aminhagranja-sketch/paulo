# Navegador / celular

Cliente responsivo Canvas 2D com o mesmo C++20 compilado em WASM. Os 189 quadros são recortes dos oito PNGs originais do usuário, sem redesenho. Controles: analógico esquerdo, Bicar à direita, Usar, Esquiva, Comer e Bolsa (I).

Site solo: https://aminhagranja-sketch.github.io/paulo/. Hospede em HTTPS; não abra index.html como arquivo. Recompile com `scripts/build-web.sh` (Emscripten 3.1.69). Reimporte sprites com `scripts/build-game-art.sh` (Python/Pillow).

Inimigos: nível 1–9 pintinhos, 10–19 raposas, 20–40 cobras. Baús de inimigos exigem E/Usar. Save local v3 mantém drops e abertura; versões antigas preservam inventário.

Multiplayer opcional: `npm start` na raiz serve cliente e WebSocket. O cliente do Pages aceita `?server=wss://SEU_HOST/ws`. Consulte `server/README.md`; o Pages sozinho não executa o servidor.

Testes: `npm run test:web` com servidor estático interno em 127.0.0.1:8088; `npm run test:multiplayer` inicia e encerra seu próprio servidor, com perfis temporários.
