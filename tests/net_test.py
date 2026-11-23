"""Teste de ponta a ponta do WebSocket do firmware: compila motor + web + ws_server para o PC
(tests/net/), sobe numa porta local e conecta o Chromium de verdade e um cliente TCP cru."""
import asyncio, base64, hashlib, http.server, os, pathlib, socket, struct, subprocess, threading, time
from playwright.async_api import async_playwright

ROOT = pathlib.Path(__file__).resolve().parent.parent
F = ROOT / 'firmware' / 'teste_nema17_wifi'
BIN = '/tmp/nemabot_net_test'
PORT = 8181
fails = 0


def ok(c, m):
    global fails
    print(('ok   ' if c else 'FALHOU ') + m)
    fails += not c


def build():
    srcs = [ROOT / 'tests/net/net_test.cpp'] + [F / f for f in
            ['motion.cpp', 'log.cpp', 'samples.cpp', 'web.cpp', 'ws_server.cpp', 'wifi_net.cpp', 'ota.cpp']]
    cmd = ['g++', '-std=gnu++17', '-O1', '-Wall', '-Wextra', '-Werror', f'-I{ROOT}/tests/net/stub',
           f'-I{ROOT}/tests/harness/stub', f'-I{F}', *map(str, srcs), '-o', BIN]
    subprocess.run(cmd, check=True)


def raw_handshake():
    s = socket.create_connection(('127.0.0.1', PORT))
    key = base64.b64encode(os.urandom(16)).decode()
    s.sendall((f'GET / HTTP/1.1\r\nHost: x\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n'
               f'sec-websocket-key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n').encode())
    resp = b''
    while b'\r\n\r\n' not in resp:
        resp += s.recv(1024)
    want = base64.b64encode(hashlib.sha1((key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').encode()).digest()).decode()
    return s, resp.decode(), want


def frame(op, payload, mask=True):
    h = bytes([0x80 | op])
    n = len(payload)
    h += bytes([(0x80 if mask else 0) | (n if n < 126 else 126)]) + (struct.pack('>H', n) if n >= 126 else b'')
    if not mask:
        return h + payload
    m = os.urandom(4)
    return h + m + bytes(b ^ m[i % 4] for i, b in enumerate(payload))


def read_frame(s, timeout=2):
    s.settimeout(timeout)
    def rd(n):
        b = b''
        while len(b) < n:
            c = s.recv(n - len(b))
            if not c:
                raise ConnectionError('fechou')
            b += c
        return b
    h = rd(2)
    n = h[1] & 0x7F
    if n == 126: n = struct.unpack('>H', rd(2))[0]
    elif n == 127: n = struct.unpack('>Q', rd(8))[0]
    return h[0] & 0x0F, rd(n)


JS = '''async (port) => {
  const out = {msgs: [], replies: {}, opened: false};
  const ws = new WebSocket('ws://127.0.0.1:' + port + '/');
  window.__t = ws;
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; setTimeout(rej, 3000); });
  out.opened = true;
  ws.onmessage = e => { const j = JSON.parse(e.data); if (j.re !== undefined) out.replies[j.re] = j; else out.msgs.push({t: performance.now(), j}); };
  ws.send('s|since=0&lsince=0');
  window.__out = out;
  return true;
}'''


async def main():
    build()
    proc = subprocess.Popen([BIN, str(PORT), '40'], stdout=subprocess.PIPE, text=True)
    lines = [proc.stdout.readline().strip() for _ in range(9)]
    ok(lines[0] == 'ACCEPT s3pPLMBiTxaQ9kYGzzhZRbK+xOo=', 'chave do handshake igual ao exemplo da RFC 6455')
    want = {'SHA1000': hashlib.sha1(b'a' * 1000).hexdigest(),
            **{f'SHA{n}': hashlib.sha1(b'x' * n).hexdigest() for n in [0, 55, 56, 63, 64, 65]}}
    got = dict(l.split() for l in lines[1:8])
    ok(got == want, 'SHA-1 confere com o hashlib em 7 tamanhos (inclui as bordas de bloco)')
    ok(lines[8] == 'PRONTO', 'servidor no ar')
    try:
        async with async_playwright() as p:
            b = await p.chromium.launch(args=['--no-proxy-server'])   # conexão local direta
            pg = await b.new_page()
            # página servida por HTTP local (o Chromium recusa WebSocket a partir de about:blank)
            hs = http.server.ThreadingHTTPServer(('127.0.0.1', 0), http.server.SimpleHTTPRequestHandler)
            threading.Thread(target=hs.serve_forever, daemon=True).start()
            await pg.goto(f'http://127.0.0.1:{hs.server_address[1]}/README.md')
            ok(await pg.evaluate(JS, PORT), 'Chromium conecta (handshake aceito)')
            await pg.wait_for_timeout(1500)
            out = await pg.evaluate('window.__out')
            msgs = out['msgs']
            ok(10 <= len(msgs) <= 18, f'status empurrado a ~10/s ({len(msgs)} em 1,5 s)')
            first = msgs[0]['j']
            ok(len(first['samples']) > 50, f'primeiro push traz o histórico ({len(first["samples"])} amostras)')
            ok(any('[ws] página conectada' in l[1] for m in msgs for l in m['j']['log']), 'log avisa a conexão')
            seqs = [t for m in msgs for t in (s[0] for s in m['j']['samples'])]
            ok(all(b > a for a, b in zip(seqs, seqs[1:])), 'amostras sem repetição entre pushes')
            ok(first['ws'] == 1, 'status informa 1 conexão')

            await pg.evaluate("window.__t.send('1|c=rpm&v=30')")
            await pg.evaluate("window.__t.send('2|c=goto&v=90')")
            await pg.evaluate("window.__t.send('3|c=voar&v=1')")
            await pg.wait_for_timeout(400)
            rep = await pg.evaluate('window.__out.replies')
            ok(rep.get('2', {}).get('ok') and rep['2'].get('c') == 'goto', f'comando responde pelo WebSocket {rep.get("2")}')
            ok(rep.get('3', {}).get('ok') is False and 'desconhecido' in rep['3'].get('error', ''), f'erro de comando volta {rep.get("3")}')
            t0 = time.time()
            pos = None
            while time.time() - t0 < 15:
                await pg.wait_for_timeout(200)
                last = await pg.evaluate('window.__out.msgs[window.__out.msgs.length-1].j')
                pos = last['pos']
                if abs(pos - 90) < 0.01 and not last['moving']:
                    break
            ok(pos is not None and abs(pos - 90) < 0.01, f'motor do firmware chegou a 90° ({pos:.2f}°, {time.time() - t0:.1f} s)')

            # jog renovado pelo WebSocket e parada sozinha
            await pg.evaluate('''()=>{let k=10;window.__jt=setInterval(()=>window.__t.send((k++)+'|c=jog&v=1&p2=1'),150);}''')
            await pg.wait_for_timeout(800)
            m = await pg.evaluate('window.__out.msgs[window.__out.msgs.length-1].j.mode')
            await pg.evaluate('clearInterval(window.__jt)')
            ok(m == 'jog', f'jog roda com renovações pelo WebSocket ({m})')
            await pg.wait_for_timeout(1500)
            m = await pg.evaluate('window.__out.msgs[window.__out.msgs.length-1].j.mode')
            ok(m != 'jog', f'jog para sozinho sem renovação ({m})')

            # cliente TCP cru: ping, mensagem longa, quadro sem máscara
            s, resp, want = raw_handshake()
            ok(resp.startswith('HTTP/1.1 101') and want in resp, 'handshake com cabeçalho em minúsculas')
            s.sendall(frame(0x9, b'oi'))
            op, pl = read_frame(s)
            while op != 0xA:
                op, pl = read_frame(s)
            ok(pl == b'oi', 'ping recebe pong com o mesmo conteúdo')
            s.sendall(frame(0x1, ('7|c=zero&v=0&lixo=' + 'x' * 400).encode()))
            got = None
            for _ in range(30):
                op, pl = read_frame(s)
                if op == 1 and pl.startswith(b'{"re":7'):
                    got = pl
                    break
            ok(got is not None and b'"ok":true' in got, 'mensagem de 420 bytes (tamanho estendido) processada')
            s.sendall(frame(0x1, b'8|c=stop', mask=False))
            closed = False
            try:
                for _ in range(40):
                    op, pl = read_frame(s)
                    if op == 0x8:
                        closed = True
                        break
            except (ConnectionError, socket.timeout):
                closed = True
            ok(closed, 'quadro sem máscara derruba a conexão (exigência da RFC)')
            s.close()

            # limite de conexões e liberação das vagas
            extra = [raw_handshake()[0] for _ in range(3)]          # Chromium + 3 = 4 (máximo)
            s5 = socket.create_connection(('127.0.0.1', PORT))
            s5.sendall(b'GET / HTTP/1.1\r\nSec-WebSocket-Key: abc\r\n\r\n')
            s5.settimeout(2)
            try:
                rej = s5.recv(100) == b''
            except (ConnectionResetError, socket.timeout):
                rej = True
            ok(rej, '5ª conexão é recusada (máximo 4)')
            for x in extra:
                x.close()
            await pg.wait_for_timeout(300)
            s6, resp6, _ = raw_handshake()
            ok(resp6.startswith('HTTP/1.1 101'), 'vagas voltam quando as conexões fecham')
            s6.close()
            n = await pg.evaluate('window.__out.msgs.length')
            await pg.wait_for_timeout(500)
            ok(await pg.evaluate('window.__out.msgs.length') > n, 'o Chromium continua recebendo depois de tudo')
            await b.close()
    finally:
        proc.kill()
    print('tudo certo' if not fails else f'{fails} falha(s)')
    raise SystemExit(1 if fails else 0)

asyncio.run(main())
