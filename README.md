# testador-nema

Interface para testar os motores NEMA com redução do nemabot. Um ESP32 controla um NEMA 17
(driver TB6600, TMC2209 ou MKS SERVO42D no modo de pulso) e serve uma página web pelo Wi-Fi. Pela página dá para mover o motor,
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

No MKS SERVO42D (modo de pulso, CR_vFOC) os mesmos GPIOs vão em STP, DIR e EN, e o COM
(anodo comum das entradas) vai no 3V3 do ESP32. A aba Ligações mostra o diagrama e o menu da placa.

Senhas de Wi-Fi e OTA ficam na memória do ESP32 (NVS), configuradas pela página.
Nenhuma senha vai no código.

## Conexão

A página (porta 80) vem comprimida em gzip, ~38 KB em vez de ~126 KB. Depois de carregada,
ela abre um WebSocket na porta 81: o ESP32 empurra o status e as amostras do gráfico a cada
100 ms e recebe os comandos pela mesma conexão. Se o WebSocket cair, a página volta sozinha
a perguntar por HTTP (`/status`) e reconecta. O tooltip do indicador "Conectado" mostra qual
dos dois está em uso.

## Testes

```sh
node tests/test_engine.js                     # motor simulado em JS
F=firmware/teste_nema17_wifi                  # motor do firmware em C++, num relógio simulado
g++ -std=gnu++17 -w -Itests/harness/stub -I$F tests/harness/harness.cpp $F/motion.cpp $F/log.cpp -o /tmp/harness && /tmp/harness
python3 tests/net_test.py                     # firmware no PC com sockets de verdade + Chromium no WebSocket
python3 tools/build_web.py                    # os dois abaixo usam o dist/previa.html
python3 tests/ui_test.py                      # interface
python3 tests/ws_ui_test.py                   # WebSocket da página: push, comandos, queda e reconexão
```

Os testes em Python precisam de `playwright` (com o Chromium instalado).
