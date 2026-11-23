#!/usr/bin/env python3
"""Monta a interface a partir de web/src e gera:

  firmware/teste_nema17_wifi/web_page.h   a página embutida no ESP32 (gzip)
  dist/index.html                         a página pura (pode abrir apontando pro ESP32)
  dist/previa.html                        a página com o motor simulado, abre direto no navegador

Uso:  python3 tools/build_web.py
"""
import gzip, pathlib, re

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / 'web' / 'src'
SIM = ROOT / 'web' / 'sim'
DIST = ROOT / 'dist'
HEADER = ROOT / 'firmware' / 'teste_nema17_wifi' / 'web_page.h'


def build_page() -> str:
    css = (SRC / 'style.css').read_text()
    js = '\n'.join(p.read_text() for p in sorted((SRC / 'js').glob('*.js')))
    page = (SRC / 'index.html').read_text()
    assert page.count('/*@CSS*/') == 1 and page.count('/*@JS*/') == 1
    page = page.replace('/*@CSS*/', css.rstrip('\n')).replace('/*@JS*/', js.rstrip('\n'))
    return page, js


def check(page: str, js: str) -> None:
    ids = set(re.findall(r'id="([^"]+)"', page))
    used = set(re.findall(r"\$\('([^']+)'\)", js))
    missing = sorted(used - ids - {'msApply'})
    urls = set(re.findall(r'https?://[^"\' )<]+', page))
    if missing:
        raise SystemExit(f'ids usados no JS que não existem no HTML: {missing}')
    if urls:
        raise SystemExit(f'a página não pode depender de nada externo: {urls}')


def write_header(page: str) -> int:
    gz = gzip.compress(page.encode(), compresslevel=9, mtime=0)   # mtime=0: mesmo conteúdo, mesmo arquivo
    rows = [', '.join(f'0x{x:02x}' for x in gz[i:i + 16]) for i in range(0, len(gz), 16)]
    HEADER.write_text(
        '// web_page.h — a página inteira (HTML + CSS + JS) comprimida em gzip, servida direto da flash.\n'
        '// Sem nada externo: funciona também na rede própria "nemabot", sem internet.\n'
        '// GERADO por tools/build_web.py a partir de web/src. Não edite à mão.\n'
        '#pragma once\n#include <Arduino.h>\n\n'
        f'// {len(page.encode()):,} bytes de página → {len(gz):,} bytes em gzip\n'
        'const uint8_t PAGE_GZ[] PROGMEM = {\n  ' + ',\n  '.join(rows) + '\n};\n'
        'const size_t PAGE_GZ_LEN = sizeof(PAGE_GZ);\n')
    return len(gz)


def write_dist(page: str) -> None:
    DIST.mkdir(exist_ok=True)
    (DIST / 'index.html').write_text(page)
    sim = (SIM / 'engine.js').read_text() + '\n' + (SIM / 'mock.js').read_text()
    banner = ('<div style="position:fixed;left:0;right:0;bottom:0;z-index:40;background:#2a2108;'
              'color:#FCD34D;font:12px -apple-system,sans-serif;padding:6px 12px;text-align:center;'
              'border-top:1px solid #4a3a10">Prévia com motor simulado: os botões funcionam, '
              'mas nada vai para o ESP32.</div>')
    prev = page.replace('<title>', '<script>' + sim + '</script><title>', 1).replace('</body>', banner + '</body>', 1)
    (DIST / 'previa.html').write_text(prev)


if __name__ == '__main__':
    page, js = build_page()
    check(page, js)
    gz = write_header(page)
    write_dist(page)
    print(f'página: {len(page.encode()):,} bytes, {gz:,} em gzip → {HEADER.relative_to(ROOT)}, dist/index.html, dist/previa.html')
