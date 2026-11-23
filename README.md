# testador-nema

Interface para testar os motores NEMA com redução do nemabot. Um ESP32 controla um NEMA 17
(driver TB6600 ou TMC2209) e serve uma página web pelo Wi-Fi. Pela página dá para mover o motor,
rodar rotinas de teste, ver o movimento num gráfico ao vivo e atualizar o firmware sem cabo (OTA).

## Estrutura

```
firmware/teste_nema17_wifi/   sketch do Arduino (abra o .ino na IDE)
web/src/                      fontes da interface: index.html, style.css, js/*.js
web/sim/                      motor simulado em JS, usado na prévia e nos testes
tools/build_web.py            monta a página e gera firmware/.../web_page.h
tests/                        testes do motor (JS e C++) e da interface (Playwright)
```

## Fluxo

1. Edite a interface em `web/src/`.
2. Rode `python3 tools/build_web.py`. Ele gera:
   - `firmware/teste_nema17_wifi/web_page.h` (não edite à mão)
   - `dist/previa.html`, que abre direto no navegador com o motor simulado
3. Compile e grave pela IDE do Arduino (cabo ou OTA).

O firmware (`.ino`, `.cpp`, `.h`) é editado direto na pasta `firmware/`.

## Hardware

| Sinal | GPIO |
|---|---|
| STEP | 25 |
| DIR  | 26 |
| ENA  | 27 |

Senhas de Wi-Fi e OTA ficam na memória do ESP32 (NVS), configuradas pela página.
Nenhuma senha vai no código.

## Testes

```sh
node tests/test_engine.js                     # motor simulado em JS
F=firmware/teste_nema17_wifi                  # motor do firmware em C++, num relógio simulado
g++ -std=gnu++17 -w -Itests/harness/stub -I$F tests/harness/harness.cpp $F/motion.cpp $F/log.cpp -o /tmp/harness && /tmp/harness
python3 tests/ui_test.py                      # interface (precisa de playwright e do build)
```
