import asyncio
from playwright.async_api import async_playwright
import pathlib
import tempfile
SHOTS=tempfile.gettempdir()
U=(pathlib.Path(__file__).resolve().parent.parent/'dist'/'previa.html').as_uri()
async def main():
    async with async_playwright() as p:
        b=await p.chromium.launch(executable_path='/usr/bin/google-chrome')
        pg=await b.new_page(viewport={'width':1440,'height':960})
        errs=[]; pg.on('pageerror',lambda e:errs.append(str(e))); pg.on('console',lambda m:errs.append(m.text) if m.type=='error' else None)
        ev=lambda js:pg.evaluate(js)
        R=[]
        await pg.goto(U); await pg.wait_for_timeout(1500)
        await pg.click('#stop'); await pg.wait_for_timeout(1500)
        # jog
        await pg.click('#mseg button[data-m="jog"]')
        bb=await pg.locator('#jogL').bounding_box(); p0=await ev('__E.pos')
        await pg.mouse.move(bb['x']+40,bb['y']+20); await pg.mouse.down(); await pg.wait_for_timeout(1200)
        mid=await ev('__E.mode'); await pg.mouse.up(); await pg.wait_for_timeout(1500)
        R.append(('jog: girou para trás e parou ao soltar', mid=='jog' and await ev('__E.pos')<p0 and await ev('__E.vel')==0))
        # curva S
        await pg.click('#rseg button[data-r="1"]'); await pg.wait_for_timeout(300)
        R.append(('curva S: jerk visível e ESP32 em ramp=1', not await pg.is_hidden('#jerkRow') and await ev('__E.rampMode')==1))
        # testes
        cv_before=await ev("$('cp')")
        await pg.click('.tab[data-tab="tst"]'); await pg.wait_for_timeout(300)
        R.append(('aba Testes troca só a coluna esquerda (mesmo canvas, deck controle oculto)', await ev("!$('tab-main').hidden && $('deck-ctl').hidden && !$('deck-tst').hidden && document.querySelectorAll('canvas').length===2")))
        await pg.select_option('#tsel','sweep'); await pg.wait_for_timeout(100)
        await pg.fill('[data-id="rpmMin"]','5'); await pg.fill('[data-id="rpmMax"]','30'); await pg.fill('[data-id="steps"]','6')
        await pg.click('#tgo'); await pg.wait_for_timeout(5000)
        R.append(('varredura iniciou e o botão virou Parar teste', await ev('__E.mode')=='sweep' and 'Em teste' in await pg.inner_text('#tgo')))
        await pg.click('body',position={'x':700,'y':800}); await pg.keyboard.press('m'); await pg.wait_for_timeout(1500)
        R.append(('tecla M: parou, marcou no gráfico e na lista', await ev('__E.mode')=='idle' and await ev('G.marks.length')==1 and await ev("document.querySelectorAll('#tres .res').length")==1 and await ev('paused')))
        await pg.screenshot(path=SHOTS+'/v3_tests.png')
        # trajetória
        await pg.select_option('#tsel','waypoints'); await pg.wait_for_timeout(100)
        await pg.click('#wpAdd'); n=await ev("document.querySelectorAll('#wpRows .wprow').length")
        await pg.click('#tgo'); await pg.wait_for_timeout(800)
        R.append(('trajetória: editor com '+str(n)+' pontos e ESP32 em waypoints', n==4 and await ev('__E.mode')=='waypoints'))
        await pg.screenshot(path=SHOTS+'/v3_wp.png')
        await pg.click('#tgo'); await pg.wait_for_timeout(1500)
        # segurança
        await pg.click('.tab[data-tab="sys"]',timeout=5000); await pg.wait_for_timeout(300)
        await pg.click('label:has(#limEn) .trk'); await pg.fill('#limMin','-90'); await pg.fill('#limMax','90'); await pg.click('label:has(#enaUse) .trk'); await pg.click('#safeSave'); await pg.wait_for_timeout(600)
        R.append(('segurança salva: batentes e ENA ligados no ESP32', await ev('__E.limEn && __E.limMin===-90 && __E.enaUse')))
        await pg.screenshot(path=SHOTS+'/v3_sys.png',full_page=True)
        await pg.click('.tab[data-tab="ctl"]'); await pg.wait_for_timeout(400)
        R.append(('cabeçalho mostra batentes e botão de torque', not await pg.is_hidden('#limBadge') and not await pg.is_hidden('#enaBtn')))
        R.append(('chips ±180 desabilitados e barra com faixa', await ev("document.querySelector('.chip[data-g=\"180\"]').disabled && document.querySelector('.chip[data-g=\"-180\"]').disabled && !!$('sl').style.getPropertyValue('--trk')")))
        await pg.click('#mseg button[data-m="abs"]'); await pg.fill('#ga','170'); await pg.click('#goto'); await pg.wait_for_timeout(6000)
        R.append(('Ir 170° com batente 90° para em 90°', abs(await ev('__E.stepsToDeg(__E.pos)')-90)<0.01))
        await pg.click('#enaBtn'); await pg.wait_for_timeout(800)
        R.append(('torque solto: botão vira Solto', await ev('!__E.enaOn') and 'Solto' in await pg.inner_text('#enaBtn')))
        await pg.screenshot(path=SHOTS+'/v3_ctl.png')
        await pg.click('#goto'); await pg.wait_for_timeout(300)
        R.append(('movimento religa o torque', await ev('__E.enaOn')))
        await pg.click('.tab[data-tab="tst"]'); await pg.select_option('#tsel','resonance'); await pg.click('#tgo'); await pg.wait_for_timeout(300)
        R.append(('ressonância bloqueada com batentes', await ev('__E.mode')!='resonance'))
        for name,res in R: print('ok  ' if res else 'FALHOU',name)
        print('erros:',errs)
        await b.close()
asyncio.run(main())
