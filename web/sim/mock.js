// Simulador do ESP32 para a prévia: usa o mesmo motor de movimento (engine.js) dos testes.
(function(){
const E=new NemaEngine();
const ring=[];let seqN=0;const logL=[];let logN=0;
const lg=t=>{logL.push([Math.round(E.t*1000),t]);logN++;if(logL.length>64)logL.shift();};
['Conectando em "WIFI NEMA 2.4GHz"','Conectado. Abra http://nemabot.local ou http://192.168.0.42','OTA ligado: no Arduino IDE escolha a porta de rede "nemabot".','Comandos: r <rpm> | g <graus> | + <graus> | - <graus> | l <graus> | s | x | z | ?'].forEach(lg);
const tgtShown=()=>(E.mode==='jog'||E.mode==='resonance')?E.pos:E.target;   // giro contínuo: alvo interno é 'infinito'
const sample=ts=>{ring.push([Math.round(ts*1000),+E.stepsToDeg(E.pos).toFixed(2),+E.stepsToDeg(tgtShown()).toFixed(2),+(E.vel*60/E.STEPS_OUT_REV).toFixed(2)]);seqN++;if(ring.length>1024)ring.shift();};
const t0=performance.now();
setInterval(()=>{E.runUntil((performance.now()-t0)/1000,sample);while(E.logs.length)lg(E.logs.shift());},20);
const json=(o,code)=>Promise.resolve(new Response(JSON.stringify(o),{status:code||200,headers:{'Content-Type':'application/json'}}));
const ok=o=>json(Object.assign({ok:true},o||{}));
const err=(m,code)=>json({ok:false,error:m},code||400);
let scanReady=0;
window.__E=E;
window.fetch=function(url,opt){
  const u=new URL(url,'http://nemabot.local'),q=k=>u.searchParams.get(k);
  const body=opt&&opt.body?new URLSearchParams(opt.body):new URLSearchParams();
  switch(u.pathname){
    case '/cmd':{const c=q('c');
      if(c==='resonance'&&E.limEn)return err('ressonância bloqueada com batentes ligados');
      const known=['goto','move','rpm','acc','ramp','loop','sweep','accsweep','resonance','backlash','drift','stiction','waypoints','jog','stop','zero','ena'];
      if(!known.includes(c))return err('comando desconhecido');
      if(c!=='jog')lg('[web] '+c+' '+(+q('v')||0).toFixed(2));
      E.cmd(c,q('v'),q('p2'),q('p3'),q('p4'));return ok({c,v:+q('v')||0});}
    case '/status':{
      let since=+q('since')||0;if(since>seqN||seqN-since>1000)since=Math.max(0,seqN-1000);
      let ls=+q('lsince')||0;if(ls>logN||logN-ls>64)ls=Math.max(0,logN-64);
      return json({pos:E.stepsToDeg(E.pos),target:E.stepsToDeg(tgtShown()),rpm:E.vel*60/E.STEPS_OUT_REV,moving:E.vel!==0,mode:E.mode,subMode:E.subMode(),cycles:E.cycles,
        sweep:E.mode==='resonance'?E.resRpmNow:E.sweepRpm,stepAcc:E.sweepAcc,rpmSet:E.rpm,acc:E.acc,ramp:E.rampMode,jerk:E.jerk,
        limEn:E.limEn,limMin:E.limMin,limMax:E.limMax,enaUse:E.enaUse,enaOn:E.enaOn,enaInv:E.enaInv,
        net:'sta',ssid:'WIFI NEMA 2.4GHz',ip:'192.168.0.42',rssi:-58,saved:'WIFI NEMA 2.4GHz',ota:true,micro:E.MICRO,ratio:E.RATIO,rpmMax:E.RPM_MAX,
        up:Math.round(E.t*1000),beat:0,logSeq:logN,log:logL.slice(logL.length-(logN-ls)),seq:seqN,samples:ring.slice(ring.length-(seqN-since))});}
    case '/scan':{
      if(q('fresh')||!scanReady){scanReady=performance.now();return json({done:false});}
      if(performance.now()-scanReady<1600)return json({done:false});scanReady=0;
      const nets=[['WIFI NEMA 2.4GHz',-58,1,6],['Vizinho_2G',-71,1,1],['NET_VIRTUA_88',-79,1,11],['Creality-K1C',-63,1,6],['CLARO_5G2A1B',-82,1,3],['Visitantes',-74,0,6],['VIVOFIBRA-7731',-88,1,9],['iPhone de Ana',-67,1,1]]
        .map(([ssid,rssi,enc,ch])=>({ssid,rssi,enc:!!enc,ch})).sort((a,b)=>b.rssi-a.rssi);
      return json({done:true,nets});}
    case '/motor':{const ms=+body.get('ms'),r=+body.get('ratio');lg('Motor: micropasso 1/'+ms+', redução '+r.toFixed(2)+':1 salvos.');E.cmd('cfg',ms,r);return ok();}
    case '/safety':{const lo=+body.get('limMin'),hi=+body.get('limMax'),en=body.get('limEn')==='1';
      if(en&&!(lo<hi))return err('mínimo precisa ser menor que o máximo');
      E.cmd('limits',en?1:0,lo,hi,(body.get('enaUse')==='1'?1:0)|(body.get('enaInv')==='1'?2:0));lg('Segurança salva.');return ok();}
    case '/waypoints':{
      const pts=(body.get('pts')||'').split(';').filter(Boolean).map(s=>{const [d,r,w]=s.split(',').map(Number);return {deg:d,rpm:r,dwell:w};});
      if(pts.length<1||pts.length>8)return err('de 1 a 8 pontos');
      if(E.limEn&&pts.some(p=>p.deg<E.limMin||p.deg>E.limMax))return err('ponto fora dos batentes');
      E.wp=pts;E.wpLoop=body.get('loop')==='1';E.cmd('waypoints');return ok({n:pts.length});}
    case '/wifi':case '/wifi/forget':case '/ota':return ok();
  }
  return Promise.reject(new Error('404'));
};
// WebSocket simulado: mesmo protocolo do firmware (ws_server + web.cpp), em cima das rotas acima.
let wsN=0;
window.WebSocket=function(url){
  const ws=this;ws.url=url;ws.readyState=0;wsN++;
  let since=0,lsince=0,ready=false,mute=false;
  const emit=d=>{if(ws.readyState===1&&!mute&&ws.onmessage)ws.onmessage({data:d});};
  setTimeout(()=>{if(ws.readyState!==0)return;ws.readyState=1;if(ws.onopen)ws.onopen();},50);
  ws.send=msg=>{
    if(ws.readyState!==1)throw new Error('fechado');
    const i=msg.indexOf('|'),head=msg.slice(0,i),q=msg.slice(i+1);
    if(head==='s'){const p=new URLSearchParams(q);since=+p.get('since')||0;lsince=+p.get('lsince')||0;ready=true;return;}
    window.fetch('/cmd?'+q).then(r=>r.json()).then(j=>emit(JSON.stringify(Object.assign({re:+head},j))));
  };
  const timer=setInterval(()=>{
    if(!ready||ws.readyState!==1||mute)return;
    window.fetch('/status?since='+since+'&lsince='+lsince).then(r=>r.json()).then(j=>{since=j.seq;lsince=j.logSeq;j.ws=1;emit(JSON.stringify(j));});
  },100);
  ws.close=()=>{ws.readyState=3;clearInterval(timer);};
  // para os testes: derrubar a conexão ou deixá-la muda (aberta, sem dados)
  ws._kill=()=>{ws.close();if(ws.onclose)ws.onclose({});};
  ws._mute=()=>{mute=true;};
  window.__ws=ws;window.__wsN=wsN;
};
window.WebSocket.OPEN=1;
setTimeout(()=>E.cmd('loop',90,0,300),400);
})();
