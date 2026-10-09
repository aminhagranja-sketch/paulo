# Validação da versão inicial

Validação realizada no ambiente Debian 13 x86_64 deste projeto em 9 de outubro de 2026.

## Executado com sucesso

- Compilação Release de `meu_galinheiro` e `granja_tests` com GCC 14.2, CMake 3.31.6, SFML 3.0.2 e JSON 3.12.0.
- **19 testes de gameplay**, executados por `granja_tests` e registrados como um alvo no CTest:
  - ownership multitouch independente para analógico e ataque;
  - cancelamento, zona morta, pulsos e resize do controlador;
  - velocidade proporcional à intensidade do analógico;
  - seed, terreno e recompensas determinísticos;
  - coordenadas negativas e exatamente nove chunks ativos;
  - descarregamento e restauração dos itens coletados;
  - colisão com água sem atravessar obstáculos;
  - caminhos acessíveis até a arena, pomar e recursos necessários à missão;
  - movimento diagonal normalizado;
  - coleta sem duplicar recompensa;
  - cone de ataque, cooldown, morte, XP e moedas;
  - golpe anunciado de NPC e invulnerabilidade de esquiva;
  - respawn e penalidade de moedas;
  - alimentos e cura sem desperdício;
  - níveis, economia e restrição de melhorias ao ninho;
  - bloqueio do chefe, vitória e recompensa única;
  - save/load JSON e substituição de um save existente;
  - persistência de chunks já descarregados;
  - rejeição transacional de JSON inválido e vida fora de intervalo.
- **Smoke gráfico** com Xvfb e OpenGL Mesa por software: janela SFML, fonte, terreno, personagens e HUD renderizados em 130 frames; movimento, coleta de um ovo, salvamento e recarga bem-sucedidos. Captura em `docs/screenshots/gameplay.png`.
- **Compilação cruzada Windows x64** usando MinGW GCC 14-posix: executável PE gerado com SFML e FreeType estáticos. Inspeção de imports confirmou apenas DLLs do sistema Windows: ADVAPI32, GDI32, KERNEL32, msvcrt, OPENGL32, USER32 e WINMM.

## Limites da evidência

A execução do `.exe` em uma máquina Windows real e o build nativo MSVC não foram realizados neste ambiente Linux. A CI incluída prepara esse build e executa os testes em Windows, mas ainda precisa rodar no GitHub. A compilação cruzada não substitui testes gráficos no sistema de destino.

O Xvfb não forneceu antialiasing MSAA nem controle de sincronização vertical; SFML usou o contexto disponível e o smoke passou. O limite de 60 FPS não é uma promessa de desempenho em todos os computadores. Não houve benchmark em GPU dedicada nem teste de exploração de milhares de chunks.

## Reproduzir

```bash
./scripts/bootstrap-cloud.sh
source scripts/cloud-env.sh
ctest --test-dir build/cloud --output-on-failure
./scripts/validate-graphics.sh
./scripts/build-windows-cross.sh
```

No Windows com Visual Studio 2022:

```powershell
./scripts/build-windows.ps1 -Run
```

Os testes gráficos usam saves temporários separados do progresso real. O runner sai com código diferente de zero se movimento, recompensa, recarga ou captura falharem.

## Validação móvel (v1.1)

Núcleo compilado com Emscripten 3.1.69, com exceções C++ e filesystem em memória. Teste Chromium via Playwright com touch emulado, viewports 844×390 e 390×844: movimento e ataque com dois dedos simultâneos, liberação independente, cancelamento, pausa, recarga do save local, rotação, cura pelo botão Comer e interação/compra na oficina sem teclado passaram. Sem erros JavaScript/WASM registrados. Capturas em `docs/screenshots/mobile-landscape.png` e `mobile-portrait.png`.

Não foram realizados testes em telefone Android/iPhone físico, Safari móvel ou tela Windows touch. O backend nativo `WM_POINTER` compila no alvo Windows x64, mas ainda precisa ser exercitado nesses dispositivos. Não há APK/iOS nativo nem promessa de FPS em todo aparelho. A hospedagem estática depende de ativar o GitHub Pages no repositório.
