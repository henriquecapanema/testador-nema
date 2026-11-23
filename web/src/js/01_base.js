const $=id=>document.getElementById(id);

// ---------- Utilidades: toasts, carregamento, pulso, API ----------
function toast(msg,type){
  const c={info:'var(--accent-primary)',ok:'var(--accent-success)',warn:'var(--accent-warning)',err:'var(--accent-danger)'}[type||'info'];
  const t=document.createElement('div');t.className='toast';t.style.setProperty('--c',c);t.textContent=msg;$('toasts').appendChild(t);
  setTimeout(()=>{t.classList.add('out');setTimeout(()=>t.remove(),200);},2600);
}
const SPIN='<svg class="spin" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M8 2a6 6 0 1 0 6 6"/></svg>';
function setBtnLoading(b,on,label){
  if(on){if(!b.dataset.html){b.dataset.html=b.innerHTML;b.style.minWidth=b.offsetWidth+'px';}b.innerHTML=SPIN+(label?'<span>'+label+'</span>':'');b.disabled=true;}
  else{if(b.dataset.html){b.innerHTML=b.dataset.html;delete b.dataset.html;}b.disabled=false;b.style.minWidth='';}
}
function pulse(el,cls,ms){cls=cls||'ack';el.classList.remove(cls);void el.offsetWidth;el.classList.add(cls);setTimeout(()=>el.classList.remove(cls),ms||300);}
async function api(url,opt){
  const ac=new AbortController(),to=setTimeout(()=>ac.abort(),4000);
  try{
    const r=await fetch(url,Object.assign({cache:'no-store'},opt||{},{signal:ac.signal}));
    const txt=await r.text();let j;
    try{j=JSON.parse(txt);}catch(e){j={ok:r.ok,error:r.ok?undefined:txt};}
    if(j.ok===undefined)j.ok=r.ok;
    return j;
  }catch(e){return {ok:false,error:'sem resposta do ESP32'};}
  finally{clearTimeout(to);}
}
function copyText(t){                       // funciona também em http (sem navigator.clipboard)
  try{if(navigator.clipboard&&window.isSecureContext)return navigator.clipboard.writeText(t);}catch(e){}
  return new Promise((res,rej)=>{const a=document.createElement('textarea');a.value=t;a.style.position='fixed';a.style.opacity='0';document.body.appendChild(a);a.select();
    try{document.execCommand('copy')?res():rej();}catch(e){rej(e);}a.remove();});
}
// ---------- Comandos: pelo WebSocket quando aberto, senão por HTTP ----------
const WS={sock:null,open:false,id:0,wait:{},last:0};
function wsCmd(q){
  return new Promise(res=>{
    const id=++WS.id,fail=()=>{delete WS.wait[id];res({ok:false,error:'sem resposta do ESP32'});};
    const to=setTimeout(fail,4000);
    WS.wait[id]=j=>{clearTimeout(to);delete WS.wait[id];res(j);};
    try{WS.sock.send(id+'|'+q);}catch(e){clearTimeout(to);fail();}
  });
}
function cmdApi(q){return WS.open?wsCmd(q):api('/cmd?'+q);}
const MOVES=['goto','move','loop','sweep','accsweep','resonance','backlash','drift','stiction','waypoints','jog'];
async function send(c,v,btn,p2,p3,p4){
  if(MOVES.includes(c))resumeLive();       // movimento novo: gráfico volta ao tempo real
  let q='c='+c+'&v='+(v===undefined||v===''?0:v);
  [p2,p3,p4].forEach((p,i)=>{if(p!==undefined&&p!=='')q+='&p'+(i+2)+'='+p;});
  const j=await cmdApi(q);
  if(j.ok){if(btn)pulse(btn);}else toast(j.error||'falha no comando','err');
  return j;
}

// ---------- Abas ----------
function showTab(t){
  document.querySelectorAll('.tab').forEach(b=>b.classList.toggle('active',b.dataset.tab===t));
  $('tab-main').hidden=!(t==='ctl'||t==='tst');$('tab-sys').hidden=t!=='sys';$('tab-wire').hidden=t!=='wire';
  if(t==='wire')renderWiring();
  $('deck-ctl').hidden=t!=='ctl';$('deck-tst').hidden=t!=='tst';   // só a coluna esquerda troca; o gráfico continua
  try{localStorage.setItem('nemabot.tab',t);}catch(e){}
  if(t==='ctl'||t==='tst')requestAnimationFrame(()=>draw());
}
document.querySelectorAll('.tab').forEach(b=>b.onclick=()=>showTab(b.dataset.tab));

// ---------- Barra de posição ----------
const sl=$('sl');
let dragging=false,lastSend=0,pend=null;
function slSend(){
  clearTimeout(pend);
  const now=Date.now();
  if(now-lastSend>80){lastSend=now;send('goto',sl.value);}
  else pend=setTimeout(()=>{lastSend=Date.now();send('goto',sl.value);},80);
}
sl.addEventListener('input',()=>{dragging=true;$('slv').textContent=sl.value;if($('live').checked)slSend();});
sl.addEventListener('change',()=>{clearTimeout(pend);send('goto',sl.value);dragging=false;});
sl.addEventListener('pointerup',()=>{dragging=false;});
window.addEventListener('pointerup',()=>{if(dragging){dragging=false;clearTimeout(pend);send('goto',sl.value);}});
document.querySelectorAll('[data-g]').forEach(b=>b.onclick=()=>{sl.value=b.dataset.g;$('slv').textContent=b.dataset.g;send('goto',b.dataset.g,b);});

// ---------- Movimento (Absoluto | Relativo) ----------
document.querySelectorAll('#mseg button').forEach(b=>b.onclick=()=>{
  const m=b.dataset.m;
  document.querySelectorAll('#mseg button').forEach(x=>x.classList.toggle('active',x===b));
  $('m-abs').hidden=m!=='abs';$('m-rel').hidden=m!=='rel';$('m-jog').hidden=m!=='jog';
  if(m!=='jog')(m==='abs'?$('ga'):$('ra')).focus();
});
$('goto').onclick=()=>send('goto',$('ga').value,$('goto'));
$('rp').onclick=()=>send('move',$('ra').value,$('rp'));
$('rm').onclick=()=>send('move',-$('ra').value,$('rm'));
$('ga').addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();$('goto').click();}});
$('ra').addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();$('rp').click();}});

// ---------- Parâmetros ----------
[['rpm','rpm'],['acc','acc']].forEach(([id,c])=>{
  const el=$(id);
  el.addEventListener('change',async()=>{const j=await send(c,el.value);if(j.ok)pulse($('f-'+id),'ackf',700);});
  el.addEventListener('keydown',e=>{if(e.key==='Enter')el.blur();});
});

// ---------- Testes, parar, zerar ----------
let curMode='idle';
$('stop').onclick=async()=>{const j=await send('stop');if(j.ok)toast('Parado','warn');};
$('zero').onclick=async()=>{const j=await send('zero',0,$('zero'));if(j.ok)toast('Posição zerada: agora é 0°','ok');};
document.addEventListener('keydown',e=>{
  const typing=/INPUT|SELECT|TEXTAREA/.test(e.target.tagName)&&e.target.type!=='range'&&e.target.type!=='checkbox';
  if(e.key==='Escape'||(e.key===' '&&!typing&&e.target.tagName!=='BUTTON')){e.preventDefault();$('stop').click();}
  else if((e.key==='m'||e.key==='M')&&!typing&&!e.metaKey&&!e.ctrlKey){e.preventDefault();markFail();}
  else if((e.key==='i'||e.key==='I')&&!typing&&!e.metaKey&&!e.ctrlKey&&!$('deck-tst').hidden&&!$('tab-main').hidden){e.preventDefault();if(!$('tgo').disabled)$('tgo').click();}
});

// ---------- Ajuda do gráfico ----------
$('help').onclick=e=>{e.stopPropagation();$('pop').hidden=!$('pop').hidden;};
document.addEventListener('click',e=>{if(!$('pop').hidden&&!$('pop').contains(e.target))$('pop').hidden=true;});

// ---------- Campos de senha com olho ----------
const EYE='<svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M1 8s2.6-5 7-5 7 5 7 5-2.6 5-7 5-7-5-7-5z"/><circle cx="8" cy="8" r="2"/></svg>';
const EYEOFF='<svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M1 8s2.6-5 7-5 7 5 7 5-2.6 5-7 5-7-5-7-5z"/><circle cx="8" cy="8" r="2"/><path d="M2 2l12 12"/></svg>';
document.querySelectorAll('.eye').forEach(b=>{b.innerHTML=EYE;b.onclick=()=>{const i=$(b.dataset.for),show=i.type==='password';i.type=show?'text':'password';b.innerHTML=show?EYEOFF:EYE;b.title=show?'Esconder senha':'Mostrar senha';};});

// ---------- Wi-Fi ----------
function bars(rssi){
  const n=rssi>-55?4:rssi>-65?3:rssi>-75?2:1,col=rssi>-60?'#6EE7B7':rssi>=-75?'#FCD34D':'#FB7185';
  let s='<svg width="18" height="14" viewBox="0 0 18 14" style="flex:none">';
  for(let i=0;i<4;i++){const h=4+i*3.3;s+='<rect x="'+(i*4.6)+'" y="'+(14-h).toFixed(1)+'" width="3.2" height="'+h.toFixed(1)+'" rx="1" fill="'+(i<n?col:'#2A2A36')+'"/>';}
  return s+'</svg>';
}
const LOCK='<svg width="12" height="14" viewBox="0 0 12 14" fill="none" stroke="#9898A8" stroke-width="1.5" style="flex:none"><rect x="1.5" y="6" width="9" height="7" rx="1.5"/><path d="M3.5 6V4a2.5 2.5 0 0 1 5 0v2"/></svg>';
let curSsid='',scanning=false;
$('scan').onclick=async()=>{
  if(scanning)return;scanning=true;setBtnLoading($('scan'),true,'Buscando…');
  let r=await api('/scan?fresh=1');
  for(let i=0;i<40&&r.ok!==false&&!r.done;i++){await new Promise(x=>setTimeout(x,400));r=await api('/scan');}
  setBtnLoading($('scan'),false);scanning=false;
  if(!r.done){toast(r.error||'A busca não terminou, tente de novo','err');return;}
  renderNets(r.nets||[]);
};
function renderNets(nets){
  const L=$('netlist');L.innerHTML='';L.hidden=false;
  if(!nets.length){L.innerHTML='<div class="sub" style="padding:12px">Nenhuma rede encontrada.</div>';return;}
  nets.forEach(n=>{
    const b=document.createElement('button');b.className='net';b.type='button';
    b.innerHTML='<span class="nm"></span>'+(n.enc?LOCK:'<span style="width:12px"></span>')+bars(n.rssi)+'<span class="db">'+n.rssi+' dBm</span>';
    const nm=b.querySelector('.nm');nm.textContent=n.ssid;
    if(n.ssid===curSsid){const bd=document.createElement('span');bd.className='badge ok';bd.textContent='Atual';nm.appendChild(bd);}
    b.onclick=()=>{L.querySelectorAll('.net').forEach(x=>x.classList.toggle('sel',x===b));$('ssid').value=n.ssid;$('pass').value='';(n.enc?$('pass'):$('save')).focus();};
    L.appendChild(b);
  });
}
$('save').onclick=async()=>{
  const ssid=$('ssid').value.trim();
  if(!ssid){toast('Escolha ou digite o nome da rede','warn');$('ssid').focus();return;}
  setBtnLoading($('save'),true,'Salvando…');
  const j=await api('/wifi',{method:'POST',body:new URLSearchParams({ssid,pass:$('pass').value})});
  setBtnLoading($('save'),false);
  if(j.ok){$('pass').value='';toast('Salvo. Reiniciando em “'+ssid+'”…','ok');}else toast(j.error||'Não consegui salvar','err');
};
$('pass').addEventListener('keydown',e=>{if(e.key==='Enter')$('save').click();});
let fgT=null;
$('forget').onclick=async()=>{
  const b=$('forget');
  if(!b.classList.contains('confirm')){
    b.classList.add('confirm');b.textContent='Confirmar?';
    fgT=setTimeout(()=>{b.classList.remove('confirm');b.textContent='Esquecer rede';},3000);return;
  }
  clearTimeout(fgT);b.classList.remove('confirm');b.textContent='Esquecer rede';
  const j=await api('/wifi/forget',{method:'POST'});
  j.ok?toast('Rede esquecida. Reiniciando na rede “nemabot” (192.168.4.1)','ok'):toast(j.error||'Não consegui','err');
};

// ---------- Motor ----------
let mPend=null,motorShown=false,netShown=false;
const fmtCfg=(m,r)=>(m==1?'passo cheio':'1/'+m)+' · '+(+r).toFixed(r%1?2:0)+':1';
$('msave').onclick=async()=>{
  const ms=+$('mms').value,ratio=+$('mratio').value;
  if(!(ratio>=0.1)){toast('Redução inválida','warn');return;}
  setBtnLoading($('msave'),true,'Aplicando…');
  const j=await api('/motor',{method:'POST',body:new URLSearchParams({ms,ratio})});
  if(!j.ok){setBtnLoading($('msave'),false);toast(j.error||'Não consegui aplicar','err');return;}
  mPend={ms,ratio,t:Date.now()};
};

// ---------- OTA ----------
$('otas').onclick=async()=>{
  const p=$('otap').value;
  if(p.length<6){toast('A senha precisa ter pelo menos 6 caracteres','warn');$('otap').focus();return;}
  setBtnLoading($('otas'),true,'Salvando…');
  const j=await api('/ota',{method:'POST',body:new URLSearchParams({pass:p})});
  setBtnLoading($('otas'),false);
  if(j.ok){$('otap').value='';toast('Senha do OTA salva. Reiniciando…','ok');}else toast(j.error||'Não consegui salvar','err');
};
$('otaAddr').onclick=()=>copyText($('otaAddr').dataset.v||'nemabot.local').then(()=>toast('Endereço copiado','ok'),()=>toast('Não consegui copiar','err'));
