# testador-nema

Interface para testar os motores NEMA com redução do nemabot. Um ESP32 controla um NEMA 17
(driver TB6600, TMC2209 ou MKS SERVO42D no modo de pulso) e recebe os comandos pelo Wi-Fi. A interface roda
no computador: dá para mover o motor, rodar rotinas de teste, ver o movimento num gráfico ao vivo e atualizar
o firmware sem cabo (OTA).

O ESP32 é só o **cérebro**: recebe os comandos (WebSocket na porta 81, HTTP/JSON na 80) e transforma em passos.
A interface não fica mais na flash: o nemabot.local baixa ela do GitHub. Mudar a interface é só dar push.

## Estrutura

```
firmware/teste_nema17_wifi/   sketch do Arduino (abra o .ino na IDE)
web/src/                      fontes da interface: index.html, style.css, js/*.js
web/mini/index.html           o que fica no ESP32: carregador da interface + página de reserva, ~2 KB em gzip
web/interface.html            a interface montada (gerada pelo build_web.py, versionada: o nemabot.local baixa ela)
web/sim/                      motor simulado em JS, usado na prévia e nos testes
tools/serve.py                serve a interface no computador, montada de web/src a cada recarga
tools/build_web.py            gera dist/ e o firmware/.../web_page.h
tests/                        testes do motor (JS e C++) e da interface (Playwright)
```

## Fluxo

**Usar:** abra `http://nemabot.local`. O ESP32 entrega um carregador pequeno (`web/mini/index.html`, ~2 KB),
que baixa a interface do GitHub (`web/interface.html`, branch `main`) e a abre ali mesmo, com o endereço
`nemabot.local`. A última cópia fica salva no navegador: sem internet (por exemplo, na rede própria "nemabot"),
ela é usada. Sem internet e sem cópia, aparece a página de reserva (parar, status, Wi-Fi).
`http://nemabot.local/?reserva` abre a reserva direto.

**Mudar a interface** (sem gravar o ESP32):
1. Edite `web/src/`. Para ver na hora, `python3 tools/serve.py` abre a interface no computador
   (http://localhost:8000, falando com o nemabot.local; `/previa` tem o motor simulado).
2. Rode `python3 tools/build_web.py`: ele gera o `web/interface.html`.
3. Commite e dê push no `main`. O nemabot.local pega a versão nova na próxima recarga
   (o GitHub pode levar até ~5 min para atualizar o cache).

**Firmware:** abra `firmware/teste_nema17_wifi/teste_nema17_wifi.ino` no Arduino IDE e grave (cabo ou OTA).
Os outros `.cpp`/`.h` da pasta entram juntos. O `web_page.h` é gerado pelo `build_web.py` (não edite à mão) e leva só o carregador.
Para levar a interface completa na flash de novo, rode `python3 tools/build_web.py --embutir` e grave.

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

Por isso a interface não vai para o GitHub Pages: uma página em `https` não consegue abrir `ws://` nem
`http://` para o ESP32 (o navegador bloqueia conteúdo misto). O carregador resolve isso: a página é `http://nemabot.local`
e só o texto da interface vem do GitHub.

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
