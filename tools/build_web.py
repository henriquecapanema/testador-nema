#!/usr/bin/env python3
"""Monta a interface a partir de web/src e gera:

  web/interface.html                      a interface completa, versionada: o nemabot.local baixa ela do GitHub
  dist/index.html                         a mesma interface, para abrir no computador apontando pro ESP32
  dist/previa.html                        a mesma interface com o motor simulado, abre direto no navegador
  firmware/teste_nema17_wifi/web_page.h   o que vai na flash do ESP32 (gzip): por padrão o carregador
                                          com a reserva (web/mini/index.html, ~2 KB); com --embutir, a completa

Uso:  python3 tools/build_web.py [--embutir]
No dia a dia nem precisa rodar: tools/serve.py monta a interface a cada recarga do navegador.
"""
import gzip, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / 'web' / 'src'
SIM = ROOT / 'web' / 'sim'
DIST = ROOT / 'dist'
HEADER = ROOT / 'firmware' / 'teste_nema17_wifi' / 'web_page.h'
MINI = ROOT / 'web' / 'mini' / 'index.html'
IFACE = ROOT / 'web' / 'interface.html'   # versionada: o nemabot.local baixa esta do GitHub


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


def write_header(page: str, what: str) -> int:
    gz = gzip.compress(page.encode(), compresslevel=9, mtime=0)   # mtime=0: mesmo conteúdo, mesmo arquivo
    rows = [', '.join(f'0x{x:02x}' for x in gz[i:i + 16]) for i in range(0, len(gz), 16)]
    HEADER.write_text(
        f'// web_page.h — {what}, comprimida em gzip e servida direto da flash.\n'
        '// Sem nada externo: funciona também na rede própria "nemabot", sem internet.\n'
        '// GERADO por tools/build_web.py a partir de web/src. Não edite à mão.\n'
        '#pragma once\n#include <Arduino.h>\n\n'
        f'// {len(page.encode()):,} bytes de página → {len(gz):,} bytes em gzip\n'
        'const uint8_t PAGE_GZ[] PROGMEM = {\n  ' + ',\n  '.join(rows) + '\n};\n'
        'const size_t PAGE_GZ_LEN = sizeof(PAGE_GZ);\n')
    return len(gz)


def write_dist(page: str) -> None:
    IFACE.write_text(page)
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
    write_dist(page)
    print(f'interface: {len(page.encode()):,} bytes → web/interface.html (commite junto), dist/index.html, dist/previa.html')
    if '--embutir' in sys.argv[1:]:
        flash, what = page, 'a interface completa (HTML + CSS + JS)'
    else:
        flash, what = MINI.read_text(), 'o carregador: baixa web/interface.html do GitHub (ou usa a última cópia) e, sem nada, mostra a reserva'
    gz = write_header(flash, what)
    print(f'flash do ESP32: {len(flash.encode()):,} bytes, {gz:,} em gzip → {HEADER.relative_to(ROOT)}'
          + (' (interface completa)' if flash is page else ' (carregador + reserva)'))
