# Validação — 9 de outubro de 2026

Executado no ambiente Debian x86_64:

- Release C++20 com GCC 14.2, CMake 3.31.6, SFML 3.0.2 e JSON 3.12.0; **32 cenários C++ passaram** no CTest.
- Transições 9→10 e 19→20, nível 40, troca de chunks e reload nas cinco fronteiras; progressão independente, drops 0/100%, interação obrigatória e recompensa única mesmo após save/reload durante abertura.
- **Oito testes do pipeline**: validação de fontes, recortes, isolamento de componentes, transparência, pivôs e preservação dos pixels de origem.
- WASM Emscripten 3.1.69 e Chromium/Playwright: dois dedos simultâneos, movimento/ataque, liberação, cancelamento, cura, oficina, Bolsa, pausa, save/reload, paisagem e retrato. Carregamento de oito categorias e desenho dos 189 quadros sem suavização. Sem erros JavaScript/WASM.
- **Multiplayer com dois contextos de navegador**: avatares visíveis, transições individuais 9→10/19→20, ausência de PvP, drops privados, interação simultânea, reconexão durante abertura, recompensa única e rejeição de alterações forjadas de nível/inventário.
- Smoke SFML/Xvfb: 130 frames com os atlas originais, coleta de alimento, nove chunks ativos e save/reload. Zero ovos no trajeto inicial é esperado.
- Windows x64 por MinGW: compilação cruzada e pacote ZIP gerados.

Capturas dos testes ficam em `build/validation/`; as capturas selecionadas em `docs/screenshots/`. O teste de cura usa o ninho para isolar dano de inimigos. O teste multiplayer força drops com configuração temporária e não modifica perfis reais.

## Reproduzir

```sh
source scripts/cloud-env.sh
cmake --build build/cloud --parallel 4
ctest --test-dir build/cloud --output-on-failure
python3 -m unittest discover -s tests -p 'test_sprite_pipeline.py'
./scripts/validate-graphics.sh
npm ci --ignore-scripts
# Com servidor estático de web/ ativo internamente na porta 8088:
npm run test:web
# Inicia/encerra seu próprio servidor e usa dados temporários:
npm run test:multiplayer
./scripts/build-windows-cross.sh
```

## Limites

Os testes usam touch emulado; Android/iPhone físico, Safari, tela Windows touch e execução do EXE em Windows real não foram testados. A CI prepara testes nativos MSVC. Xvfb não forneceu MSAA ou sincronização vertical; o contexto disponível passou no smoke.

Não houve benchmark com 20 jogadores ou garantia de FPS em aparelhos móveis. O servidor foi validado internamente com dois clientes, sem hospedagem pública. O GitHub Pages entrega somente o modo solo e o cliente; necessita serviço Node.js/WebSocket separado para multiplayer público. A checagem HTTPS da publicação verifica os arquivos servidos, sem teste de navegador público neste ambiente.
