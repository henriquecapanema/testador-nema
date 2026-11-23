"""Testa o caminho WebSocket da página na prévia (WebSocket simulado em web/sim/mock.js):
conecta, para o polling, manda comandos pelo WebSocket, reconecta quando cai e quando fica mudo."""
import asyncio, pathlib
from playwright.async_api import async_playwright

U = (pathlib.Path(__file__).resolve().parent.parent / 'dist' / 'previa.html').as_uri()
fails = 0


def ok(c, m):
    global fails
    print(('ok   ' if c else 'FALHOU ') + m)
    fails += not c


async def main():
    async with async_playwright() as p:
        b = await p.chromium.launch()
        pg = await b.new_page(viewport={'width': 1440, 'height': 900})
        errs = []
        pg.on('pageerror', lambda e: errs.append(str(e)))
        # conta as chamadas de fetch por rota (o mock troca window.fetch; embrulhamos depois dele)
        await pg.goto(U)
        await pg.evaluate('''()=>{window.__calls={};const f=window.fetch;window.fetch=(u,o)=>{const k=new URL(u,'http://x').pathname+(o?':http':':ws');
            window.__calls[k]=(window.__calls[k]||0)+1;return f(u,o);};}''')
        await pg.wait_for_timeout(1500)
        title = await pg.get_attribute('#pill', 'title')
        ok('WebSocket' in (title or ''), f'conecta por WebSocket ({title})')
        ok(await pg.evaluate('WS.open'), 'WS.open verdadeiro')

        # com o WebSocket aberto, a página não faz polling; o mock só busca /status para alimentar o WS
        await pg.evaluate('window.__calls={}')
        await pg.wait_for_timeout(1000)
        calls = await pg.evaluate('window.__calls')
        ok(calls.get('/status:http', 0) == 0 and 5 <= calls.get('/status:ws', 0) <= 12, f'sem polling HTTP, só o push de ~10/s ({calls})')

        n0 = await pg.evaluate('G.t.length')
        await pg.wait_for_timeout(1000)
        n1 = await pg.evaluate('G.t.length')
        ok(80 <= n1 - n0 <= 120, f'amostras chegam a ~100/s pelo WebSocket (+{n1 - n0})')
        dup = await pg.evaluate('G.t.some((t,i)=>i&&t<=G.t[i-1])')
        ok(not dup, 'sem amostras repetidas ou fora de ordem')

        # comando pelo WebSocket
        await pg.evaluate('window.__calls={}')
        r = await pg.evaluate("send('goto',45)")
        ok(r.get('ok') and r.get('c') == 'goto', f'goto responde {r}')
        calls = await pg.evaluate('window.__calls')
        ok(calls.get('/cmd:ws', 0) == 1 and calls.get('/cmd:http', 0) == 0, f'o comando foi pelo WebSocket ({calls})')
        r = await pg.evaluate("send('xyz',1)")
        ok(not r.get('ok') and 'desconhecido' in r.get('error', ''), f'erro volta pelo WebSocket ({r.get("error")})')
        await pg.wait_for_timeout(2500)
        tgt = await pg.evaluate('S.target')
        ok(abs(tgt - 45) < 0.01, f'motor recebeu o alvo ({tgt:.2f}°)')

        # cai a conexão: volta ao polling e reconecta
        await pg.evaluate('window.__ws._kill()')
        await pg.wait_for_timeout(200)
        ok(not await pg.evaluate('WS.open') and await pg.evaluate('pollOn'), 'conexão caiu: volta ao polling HTTP')
        await pg.wait_for_timeout(1200)
        ok(await pg.evaluate('WS.open') and await pg.evaluate('window.__wsN') >= 2, 'reconecta sozinho')
        n0 = await pg.evaluate('G.t.length')
        await pg.wait_for_timeout(600)
        dup = await pg.evaluate('G.t.some((t,i)=>i&&t<=G.t[i-1])')
        ok(await pg.evaluate('G.t.length') > n0 and not dup, 'após reconectar, amostras seguem sem repetição')

        # conexão aberta mas muda: detecta em ~1,5 s
        await pg.evaluate('window.__ws._mute()')
        await pg.wait_for_timeout(3000)
        n = await pg.evaluate('window.__wsN')
        ok(n >= 3, f'conexão muda é derrubada e refeita (conexões: {n})')
        await pg.wait_for_timeout(1500)
        ok('WebSocket' in (await pg.get_attribute('#pill', 'title') or ''), 'volta a "Conectado por WebSocket"')

        # jog pelo WebSocket
        await pg.evaluate('window.__calls={}')
        await pg.evaluate("showTab('ctl')")
        await pg.click('#mseg button[data-m="jog"]')
        jl = await pg.query_selector('#jogR')
        box = await jl.bounding_box()
        await pg.mouse.move(box['x'] + box['width'] / 2, box['y'] + box['height'] / 2)
        await pg.mouse.down(); await pg.wait_for_timeout(700)
        mode = await pg.evaluate('S.mode')
        await pg.mouse.up()
        calls = await pg.evaluate('window.__calls')
        ok(mode == 'jog', f'jog roda segurando o botão (modo {mode})')
        ok(calls.get('/cmd:ws', 0) >= 4 and calls.get('/cmd:http', 0) == 0, f'renovações do jog vão pelo WebSocket ({calls})')

        ok(not errs, f'sem erros de JavaScript {errs}')
        await b.close()
    print('tudo certo' if not fails else f'{fails} falha(s)')
    raise SystemExit(1 if fails else 0)

asyncio.run(main())
