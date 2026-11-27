# testador-nema

Interface para testar os motores NEMA com redução do nemabot. Um ESP32 controla um NEMA 17
(driver TB6600, TMC2209 ou MKS SERVO42D no modo de pulso) e recebe os comandos pelo Wi-Fi. A interface roda
no computador: dá para mover o motor, rodar rotinas de teste, ver o movimento num gráfico ao vivo e atualizar
o firmware sem cabo (OTA).

O ESP32 é só o **cérebro**: recebe os comandos (WebSocket na porta 81, HTTP/JSON na 80) e transforma em passos.
A página não fica mais na flash, então mudar a interface não pede gravar o firmware.

## Estrutura

```
firmware/teste_nema17_wifi/   sketch do Arduino (abra o .ino na IDE)
web/src/                      fontes da interface: index.html, style.css, js/*.js
web/mini/index.html           página de reserva que fica no ESP32 (parar, status, Wi-Fi), ~1,3 KB em gzip
web/sim/                      motor simulado em JS, usado na prévia e nos testes
tools/serve.py                serve a interface no computador, montada de web/src a cada recarga
tools/build_web.py            gera dist/ e o firmware/.../web_page.h
tests/                        testes do motor (JS e C++) e da interface (Playwright)
```

## Fluxo

**Interface** (não precisa gravar o ESP32):

```sh
python3 tools/serve.py                     # abre http://localhost:8000 falando com nemabot.local
python3 tools/serve.py --esp 192.168.4.1   # ESP32 na rede própria "nemabot"
```

Edite `web/src/`, salve e recarregue o navegador. Em `http://localhost:8000/previa` fica a mesma
interface com o motor simulado. O endereço do ESP32 também muda em Sistema → Robô (ou com `?esp=` na URL)
e fica salvo no navegador. Sem o `serve.py`, dá para abrir o `dist/index.html` direto (gerado pelo `build_web.py`).

**Firmware:** o `.ino`, `.cpp` e `.h` são editados direto em `firmware/`. Compile e grave pela IDE do Arduino (cabo ou OTA).
O `web_page.h` é gerado pelo `python3 tools/build_web.py` (não edite à mão) e por padrão leva só a página de reserva.
Para levar a interface completa na flash de novo (por exemplo, para usar só pelo celular), rode `python3 tools/build_web.py --embutir`
e grave.

## Hardware

| Sinal | GPIO |
|---|---|
| STEP | 25 |
| DIR  | 26 |
| ENA  | 27 |

No MKS SERVO42D (modo de pulso, CR_vFOC) os mesmos GPIOs vão em STP, DIR e EN, e o COM
(anodo comum das entradas) vai no 3V3 do ESP32. A aba Ligações mostra o diagrama e o menu da placa.

A opção **Braço** da aba Ligações mostra todas as ligações do nemabot: alimentação, ESP32, CAN com as SERVO42D,
TMC2209 do punho, tela da base e flange, com filtro por sistema e a escolha do motor do J1.

Senhas de Wi-Fi e OTA ficam na memória do ESP32 (NVS), configuradas pela página.
Nenhuma senha vai no código.

## Conexão

A interface abre um WebSocket em `ws://<esp>:81/`: o ESP32 empurra o status e as amostras do gráfico
a cada 100 ms e recebe os comandos pela mesma conexão (`<id>|c=goto&v=90`, resposta `{"re":<id>,"ok":true}`).
Se o WebSocket cair, a página volta sozinha a perguntar por HTTP (`http://<esp>/status`) e reconecta.
O tooltip do indicador "Conectado" mostra o endereço e qual dos dois está em uso.

As respostas JSON do ESP32 levam `Access-Control-Allow-Origin: *`, porque a página vem de outra origem
(localhost ou arquivo). O mesmo protocolo serve para o programa em Python do PC.

A interface precisa ser servida por `http` (localhost ou arquivo): uma página em `https`, como o GitHub Pages,
não consegue abrir `ws://` nem `http://` para o ESP32 (o navegador bloqueia conteúdo misto).

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
