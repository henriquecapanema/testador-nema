// ---------- Log (recolhível) ----------
let lseq=0,stepPeak=0,peakBefore=0,accSm=0;
const prevPeak=()=>peakBefore;
function setLogOpen(o){$('logcard').classList.toggle('lopen',o);try{localStorage.setItem('nemabot.log',o?'1':'0');}catch(e){}if(o){const b=$('log');b.scrollTop=b.scrollHeight;}}
$('lhead').onclick=e=>{if(e.target.closest('button'))return;setLogOpen(!$('logcard').classList.contains('lopen'));};
function addLog(lines,newSeq){
  if(newSeq===undefined)return;
  const box=$('log'),atBottom=box.scrollTop+box.clientHeight>=box.scrollHeight-6;
  const line=(cls,html)=>{const d=document.createElement('div');if(cls)d.className=cls;d.innerHTML=html;box.appendChild(d);};
  if(newSeq<lseq)line('sep','— ESP32 reiniciou —');
  const esc=t=>t.replace(/&/g,'&amp;').replace(/</g,'&lt;');
  for(const [ms,t] of lines){
    const sec=ms/1000,tt=sec<3600?(Math.floor(sec/60)+':'+(sec%60).toFixed(1).padStart(4,'0')):(sec/3600).toFixed(2)+' h';
    line(/erro|falh|travad|guru|panic|não conectou/i.test(t)?'err':'','<span class="t">'+tt+'</span>'+esc(t));
    $('logPrev').textContent=t.trim();
  }
  while(box.childNodes.length>800)box.removeChild(box.firstChild);
  lseq=newSeq;
  if(atBottom)box.scrollTop=box.scrollHeight;
}
$('lclear').onclick=()=>{$('log').innerHTML='';$('logPrev').textContent='';toast('Log limpo','info');};
$('lcopy').onclick=()=>copyText($('log').innerText).then(()=>toast('Log copiado','ok'),()=>toast('Não consegui copiar','err'));
try{setLogOpen(localStorage.getItem('nemabot.log')==='1');}catch(e){}
buildFields();renderWp();renderRes();renderRuns();
try{const t=localStorage.getItem('nemabot.tab');if(t==='sys'||t==='tst'||t==='wire')showTab(t);}catch(e){}

// ---------- Polling do status ----------
function setPill(cls,txt,title){const d=$('dot');d.className='pdot '+cls;$('conn').textContent=txt;$('pill').title=title||txt;}
// O mesmo JSON chega por dois caminhos: empurrado pelo WebSocket (padrão) ou pelo polling de /status (reserva).
function onStatus(s){
    seq=s.seq;addSamples(s.samples);draw();
    addLog(s.log||[],s.logSeq);

    // telemetria
    $('pos').textContent=s.pos.toFixed(1);
    $('tgt').textContent=s.target.toFixed(1)+'°';
    $('spd').textContent=s.rpm.toFixed(1)+' rpm';
    S=s;curMode=s.mode;
    {const run=TEST_MODES.includes(s.mode);                // grava cada execução de teste
      if(run&&!wasRunning)startRun(s);
      if(run)recordRun(s);
      if(!run&&wasRunning)endRun();
      wasRunning=run;}
    const mb=$('mode'),lab=MODE_LABEL[s.mode];
    if(lab){mb.className='badge '+(/sweep|resonance/.test(s.mode)?'warn':'pri');mb.textContent=lab+(s.subMode?' · '+s.subMode:'');}
    else if(s.moving){mb.className='badge pri';mb.textContent='movendo';}
    else{mb.className='badge';mb.textContent='parado';}
    // deck de testes
    testRunning=TEST_MODES.includes(s.mode);
    const tg=$('tgo');
    if(!tg.disabled&&tg.dataset.run!==String(testRunning)){         // só redesenha quando muda (não atrapalha o clique)
      tg.dataset.run=String(testRunning);
      tg.className='btn block big '+(testRunning?'running':'pri');
      tg.innerHTML=testRunning?'<span class="rdot"></span>Em teste · clique para parar<kbd class="k dark">I</kbd>':'Iniciar teste<kbd class="k">I</kbd>';
    }
    $('tprog').textContent=testRunning?lab:'';
    if(testRunning){const P=testProgress(s);$('tnow').hidden=false;$('tstep').textContent=P.step;$('tval').textContent=P.val;
      $('tbar').style.width=(P.frac===null?0:Math.max(0,Math.min(1,P.frac))*100)+'%';$('tbarw').hidden=P.frac===null;}
    else $('tnow').hidden=true;
    // marca no gráfico a cada troca de degrau (varreduras)
    // velocidade agora e pico do degrau (pelas amostras de 100 Hz, para não perder o pico)
    for(const x of s.samples)stepPeak=Math.max(stepPeak,Math.abs(x[3]));
    stepPeak=Math.max(stepPeak,Math.abs(s.rpm));
    const isSweep=s.mode==='sweep'||s.mode==='accsweep';
    if(testRunning&&(s.subMode!==lastSub||s.mode!==lastMode)&&(isSweep||s.mode!==lastMode)){
      const prev=G.steps[G.steps.length-1];peakBefore=stepPeak;
      if(prev&&prev.open){prev.label+=' · pico '+fmtN(prevPeak(),1)+' rpm';prev.open=false;}
      if(isSweep&&s.subMode){G.steps.push({t:lastT(),label:s.subMode,open:true});if(curRun)curRun.steps.push({t:lastT()-curRun.t0,label:s.subMode});}
      stepPeak=Math.abs(s.rpm);draw();
    }
    if(!testRunning&&lastMode&&TEST_MODES.includes(lastMode)){const prev=G.steps[G.steps.length-1];if(prev&&prev.open){prev.label+=' · pico '+fmtN(stepPeak,1)+' rpm';prev.open=false;draw();}}
    $('tvNow').textContent=fmtN(Math.abs(s.rpm),1);$('tvPk').textContent=fmtN(stepPeak,1);
    // aceleração real: derivada da velocidade nas amostras (janela de ~60 ms), em passos/s² do motor
    {const n=G.t.length;let a=0;
      if(n>2&&s.micro){let k=n-1;while(k>0&&G.t[n-1]-G.t[k]<0.06)k--;const dt=G.t[n-1]-G.t[k];
        if(dt>0.02&&dt<0.2)a=Math.abs((G.v[n-1]-G.v[k])/dt)*200*s.micro*s.ratio/60;}
      accSm=accSm*0.6+a*0.4;const txt=fmtN(accSm<50?0:accSm);
      $('taNow').textContent=txt;$('accNow').textContent=txt+' p/s²';}
    lastSub=s.subMode||'';lastMode=s.mode;
    $('tmark').disabled=!testRunning;
    $('tstdot').style.display=testRunning?'block':'none';
    // rampa, ENA, batentes
    if(s.ramp!==undefined){
      if(!rampUser||Date.now()-rampUser>1500)setRampUI(+s.ramp);
      if(document.activeElement!==$('jerk'))$('jerk').value=s.jerk;
    }
    if(s.enaUse!==undefined){
      const eb=$('enaBtn');eb.hidden=!s.enaUse;
      if(s.enaUse){eb.className='btn sm ena '+(s.enaOn?'locked':'free');eb.innerHTML=(s.enaOn?LOCK_I+'Travado':FREE_I+'Solto');
        eb.title=s.enaOn?'Motor com torque · clique para soltar':'Motor sem torque · clique para travar';}
      applyLimitsUI(s);
      wiringTick(s);
      if(!safetyShown){safetyShown=true;$('limEn').checked=!!s.limEn;$('limMin').value=s.limMin;$('limMax').value=s.limMax;$('enaUse').checked=!!s.enaUse;$('enaInv').checked=!!s.enaInv;}
    }
    dial(s.pos,s.target);
    if(!dragging){const t=Math.round(Math.max(-360,Math.min(360,s.target)));sl.value=t;$('slv').textContent=t;}
    if(document.activeElement!==$('rpm'))$('rpm').value=s.rpmSet;
    if(document.activeElement!==$('acc'))$('acc').value=s.acc;

    // motor
    if(s.ratio!==undefined){
      const one=Math.abs(s.ratio-1)<1e-6;
      $('rbadge').textContent=Math.abs(s.ratio-20)<1e-6?'J2':(+s.ratio).toFixed(s.ratio%1?2:0)+':1';
      $('rpmMax').textContent='máx. '+(+s.rpmMax).toFixed(s.rpmMax<10?1:0);
      $('rpmUnit').textContent=one?'rpm motor':'rpm';
      $('rpm').max=s.rpmMax;
      updateAutoHints();
      $('ptitle').textContent='Posição (graus '+(one?'no eixo do motor':'na saída')+')';
      $('vtitle').textContent='Velocidade (rpm '+(one?'no motor':'na saída')+')';
      $('mcur').textContent=fmtCfg(s.micro,s.ratio);
      if(!motorShown){motorShown=true;$('mms').value=String(s.micro);$('mratio').value=s.ratio;}
      if(mPend){
        if(s.micro===mPend.ms&&Math.abs(s.ratio-mPend.ratio)<0.006&&!s.moving){
          mPend=null;setBtnLoading($('msave'),false);
          toast('Motor: '+fmtCfg(s.micro,s.ratio)+' aplicado · posição em 0°','ok');
          const card=$('msave').closest('.card');card.classList.add('flash');setTimeout(()=>card.classList.remove('flash'),1400);
        }else if(Date.now()-mPend.t>8000){mPend=null;setBtnLoading($('msave'),false);toast('O ESP32 não confirmou a configuração nova','warn');}
      }
    }

    // wi-fi e OTA
    const sta=s.net==='sta';
    curSsid=sta?s.ssid:'';
    $('espBadge').className='badge ok';
    const nb=$('netBadge');nb.className='badge '+(sta?'ok':'warn');nb.textContent=sta?'Conectado':'Modo AP';
    const esc=t=>String(t).replace(/&/g,'&amp;').replace(/</g,'&lt;');
    $('netinfo').innerHTML=sta
      ?'<b>'+esc(s.ssid)+'</b><span>'+s.ip+'</span>'+bars(s.rssi)+'<span>'+s.rssi+' dBm</span>'
      :'<b>Rede própria “nemabot”</b><span>'+s.ip+'</span><span>'+(s.saved?'não conectou em “'+esc(s.saved)+'”':'nenhuma rede salva')+'</span>';
    if(!netShown){netShown=true;if(s.saved&&!$('ssid').value)$('ssid').value=s.saved;}
    const ob=$('otaBadge');ob.className='badge'+(s.ota?' ok':'');ob.textContent=s.ota?'Ativo':'Inativo';
    const addr=sta?'nemabot.local · '+s.ip:s.ip;
    if($('otaAddr').textContent!==addr){$('otaAddr').textContent=addr;$('otaAddr').dataset.v=sta?'nemabot.local':s.ip;}
    $('sysdot').style.display=(!sta||!s.ota)?'block':'none';
    $('sysdot').parentElement.title=!sta?'Em modo AP: configure o Wi-Fi':!s.ota?'OTA desligado':'';

    // conexão
    const up='ligado há '+(s.up<120000?Math.round(s.up/1000)+' s':Math.round(s.up/60000)+' min');
    if(s.beat>1000)setPill('err','Motor travado','ESP32 responde, mas o laço do motor está travado há '+(s.beat/1000).toFixed(0)+' s');
    else if(s.seq===0&&s.up>5000)setPill('warn','Sem amostras','Conectado, mas o timer de amostras está parado · '+up);
    else setPill('ok','Conectado','Conectado a '+ESP+' por '+(WS.open?'WebSocket':'HTTP (reserva)')+' · '+up);
}

// Reserva: polling HTTP enquanto o WebSocket não está aberto.
let pollOn=false;
async function poll(){
  if(WS.open){pollOn=false;return;}
  try{
    const ac=new AbortController(),to=setTimeout(()=>ac.abort(),1500);
    const s=await (await fetch(espUrl('/status?since='+seq+'&lsince='+lseq),{signal:ac.signal,cache:'no-store'})).json();
    clearTimeout(to);
    if(!WS.open)onStatus(s);
  }catch(e){if(!WS.open){$('espBadge').className='badge warn';}if(!WS.open)setPill('err','Offline','Sem resposta do ESP32 em '+ESP+' (troque o endereço em Sistema → Robô)');}
  if(WS.open){pollOn=false;return;}
  setTimeout(poll,100);
}
function startPoll(){if(!pollOn){pollOn=true;poll();}}

// WebSocket na porta 81: o ESP32 empurra o status a cada 100 ms.
function wsDrop(){                          // fecha e volta ao polling até reconectar
  const sock=WS.sock,was=WS.open&&WS.got;
  WS.fails=was?0:(WS.fails||0)+1;           // falhou sem receber nada: tenta com menos frequência
  WS.sock=null;WS.open=false;
  if(sock){sock.onopen=sock.onmessage=sock.onclose=sock.onerror=null;try{sock.close();}catch(e){}}
  for(const k in WS.wait)WS.wait[k]({ok:false,error:'conexão caiu'});
  startPoll();
  setTimeout(wsConnect,was?500:WS.fails<3?3000:30000);
}
function wsConnect(){
  if(WS.sock)return;
  let sock;
  try{sock=new WebSocket('ws://'+ESP.replace(/:\d+$/,'')+':81/');}catch(e){startPoll();return;}
  WS.sock=sock;
  sock.onopen=()=>{WS.open=true;WS.got=false;WS.last=Date.now();sock.send('s|since='+seq+'&lsince='+lseq);};
  sock.onmessage=e=>{
    WS.last=Date.now();WS.got=true;WS.fails=0;
    let j;try{j=JSON.parse(e.data);}catch(x){return;}
    if(j.re!==undefined){const f=WS.wait[j.re];if(f)f(j);return;}
    try{onStatus(j);}catch(x){console.error(x);}
  };
  sock.onclose=sock.onerror=wsDrop;
}
setInterval(()=>{                           // conexão aberta mas muda: considera caída
  if(WS.open&&Date.now()-WS.last>1500){setPill('err','Offline','Sem dados do ESP32 há '+((Date.now()-WS.last)/1000).toFixed(0)+' s');wsDrop();}
},500);
startPoll();
wsConnect();
