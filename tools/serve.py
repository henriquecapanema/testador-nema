#!/usr/bin/env python3
"""Serve a interface do nemabot no computador. O ESP32 fica só como "cérebro" (API + WebSocket).

  python3 tools/serve.py                      abre http://localhost:8000 falando com nemabot.local
  python3 tools/serve.py --esp 192.168.4.1    outro endereço (ex.: rede própria "nemabot")
  python3 tools/serve.py --porta 8080 --sem-navegador

A página é montada de web/src a cada recarga: edite, salve e aperte Cmd+R, sem build e sem gravar
o ESP32. Em /previa fica a mesma interface com o motor simulado.
Só escuta em 127.0.0.1 (o próprio computador). Precisa só da biblioteca padrão do Python.
"""
import argparse, http.server, pathlib, sys, webbrowser

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import build_web  # noqa: E402


def previa(page: str) -> str:
    sim = (build_web.SIM / 'engine.js').read_text() + '\n' + (build_web.SIM / 'mock.js').read_text()
    return page.replace('<title>', '<script>' + sim + '</script><title>', 1)


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path.split('?', 1)[0]
        if path not in ('/', '/index.html', '/previa'):
            self.send_error(404)
            return
        try:
            page, js = build_web.build_page()
            build_web.check(page, js)
        except (SystemExit, Exception) as e:          # erro de montagem aparece na própria página
            body = f'<pre style="color:#b91c1c">erro ao montar a interface:\n{e}</pre>'.encode()
            self.send_response(500)
        else:
            body = (previa(page) if path == '/previa' else page).encode()
            self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_header('Cache-Control', 'no-store')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, fmt, *args):
        pass


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--esp', default='', help='endereço do ESP32 (padrão: o último usado, ou nemabot.local)')
    ap.add_argument('--porta', type=int, default=8000)
    ap.add_argument('--sem-navegador', action='store_true')
    a = ap.parse_args()
    srv = http.server.ThreadingHTTPServer(('127.0.0.1', a.porta), Handler)
    url = f'http://localhost:{a.porta}/' + (f'?esp={a.esp}' if a.esp else '')
    print(f'interface em {url}   (prévia simulada em http://localhost:{a.porta}/previa)   Ctrl+C para sair')
    if not a.sem_navegador:
        webbrowser.open(url)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
