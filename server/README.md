# Servidor multiplayer opcional

Requer Node.js 24, cliente WASM versionado em web/ e disco gravável. Na raiz:

```sh
npm ci --ignore-scripts
npm start
```

O servidor entrega o cliente e WebSocket `/ws`, com identificação automática. `/health` informa disponibilidade, jogadores e capacidade. Para validar dois navegadores sem alterar saves reais: `npm run test:multiplayer` (Chromium/Playwright).

## Configuração e hospedagem

| Variável | Padrão | Uso |
|---|---|---|
| PORT | 9090 | Porta HTTP/WebSocket |
| HOST | 0.0.0.0 | Interface de rede |
| MAX_PLAYERS | 20 | Limite de conexões, entre 1 e 100 |
| GRANJA_DATA_DIR | server-data | Diretório persistente de perfis |
| GRANJA_COMBAT_CONFIG | config/combat.json | Chances e raridades |
| ALLOWED_ORIGINS | localhost/127.0.0.1 e Pages | Origens permitidas, separadas por vírgula |

Escolha um serviço que execute Node.js continuamente, suporte WebSocket e forneça HTTPS, com **volume persistente** em GRANJA_DATA_DIR. Configure ALLOWED_ORIGINS com a origem HTTPS desse serviço e `https://aminhagranja-sketch.github.io`. Um serviço apenas estático não executa este servidor. O cliente publicado no Pages conecta usando `?server=wss://SEU_HOST/ws`; abrir o cliente servido pelo próprio Node já configura `/ws`.

Execute uma única instância por diretório de perfis; não há coordenação entre processos nem banco externo. Faça backups do diretório. Não coloque perfis em disco efêmero. O limite padrão de 20 é configurável, mas não representa um benchmark de carga. Esta entrega foi testada com dois jogadores e ainda não tem hospedagem pública contratada.

## Regras e persistência

Cada sessão tem uma instância do C++/WASM. O servidor calcula movimento, combate, nível, chance de drop e recompensa, aceitando apenas controles e ações validadas. Avatares próximos compartilham o terreno; inimigos e baús são individuais. Não há PvP nem inimigo compartilhado para combate cooperativo.

As faixas comuns são 1–9 pintinhos, 10–19 raposas e 20–40 cobras. O chefe da missão é separado. Chances padrão: 5/10/15%; raridades 70/23/6/1% (comum/raro/épico/lendário). O JSON permite ajustar essas probabilidades; valores inválidos são rejeitados. SFML e solo usam os mesmos padrões definidos em CombatConfig.hpp.

O navegador guarda um identificador secreto de sessão em localStorage. Reabrir no mesmo navegador e origem recupera o perfil; apagar esse dado ou iniciar nova aventura perde o acesso ao identificador anterior. Não há contas, senha ou recuperação entre dispositivos. O identificador não é exposto aos outros jogadores; trate-o como credencial.

JSON v3 mantém encontros, inventário, gerador aleatório e estados dos baús. Gravações críticas ocorrem antes do envio de estado, usando arquivo temporário, fsync e renomeação. Reconectar durante a abertura retoma o mesmo baú, sem nova recompensa. Mensagens que tentam escrever posição, nível, dano ou inventário são rejeitadas; limites de tamanho e frequência reduzem abuso. Esses testes não substituem uma auditoria de segurança.
