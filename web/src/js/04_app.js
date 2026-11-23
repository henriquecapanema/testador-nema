
// ---------- Jog (segurar para girar) ----------
let jogTimer=null,jogBtn=null,jogScale=1;
function jogStart(dir,btn,e){
  if(jogTimer)return;
  try{btn.setPointerCapture(e.pointerId);}catch(x){}
  jogBtn=btn;btn.classList.add('on-pri');resumeLive();
  const go=()=>api('/cmd?c=jog&v='+dir+'&p2='+jogScale).then(j=>{if(!j.ok&&jogTimer){jogEnd();toast(j.error||'jog falhou','err');}});
  go();jogTimer=setInterval(go,150);        // o ESP32 para sozinho se ficar 450 ms sem renovação
}
function jogEnd(){
  if(!jogTimer)return;
  clearInterval(jogTimer);jogTimer=null;
  if(jogBtn)jogBtn.classList.remove('on-pri');jogBtn=null;
  api('/cmd?c=stop');
}
[['jogL',-1],['jogR',1]].forEach(([id,dir])=>{
  const b=$(id);
  b.addEventListener('pointerdown',e=>{e.preventDefault();jogStart(dir,b,e);});
  ['pointerup','pointercancel','lostpointercapture','pointerleave'].forEach(ev=>b.addEventListener(ev,jogEnd));
  b.addEventListener('contextmenu',e=>e.preventDefault());
});
window.addEventListener('blur',jogEnd);
document.addEventListener('visibilitychange',()=>{if(document.hidden)jogEnd();});
document.querySelectorAll('#jseg button').forEach(b=>b.onclick=()=>{
  jogScale=+b.dataset.s;document.querySelectorAll('#jseg button').forEach(x=>x.classList.toggle('active',x===b));
});

// ---------- Rampa: Trapezoidal | Curva S ----------
let rampUser=null;                          // evita que o status sobrescreva logo depois do clique
function setRampUI(r){document.querySelectorAll('#rseg button').forEach(x=>x.classList.toggle('active',+x.dataset.r===r));$('jerkRow').hidden=r!==1;}
document.querySelectorAll('#rseg button').forEach(b=>b.onclick=async()=>{
  const r=+b.dataset.r;setRampUI(r);rampUser=Date.now();
  const j=await send('ramp',r,null,$('jerk').value);
  if(j.ok)toast(r?'Rampa em curva S':'Rampa trapezoidal','ok');
});
$('jerk').addEventListener('change',async()=>{const j=await send('ramp',1,null,$('jerk').value);if(j.ok)pulse($('f-jerk'),'ackf',700);});
$('jerk').addEventListener('keydown',e=>{if(e.key==='Enter')$('jerk').blur();});

// ---------- Testes ----------
const TEST_MODES=['loop','sweep','accsweep','resonance','backlash','drift','stiction','waypoints'];
const MODE_LABEL={loop:'endurance',sweep:'limite de rpm',accsweep:'limite de acel.',resonance:'ressonância',backlash:'folga',drift:'repetibilidade',stiction:'atrito estático',waypoints:'trajetória',jog:'jog'};
// [id, rótulo, unidade, padrão, tipo]   (padrão '' = automático pelo ESP32)
const TESTS={
  loop:{d:'Vai e volta a amplitude sem parar. Para amaciar o cicloidal e testar desgaste.',
    f:[['amp','Amplitude','°',90],['cycles','Ciclos','0 = ∞',0],['dwell','Pausa','ms',300]],a:v=>[v.amp,v.cycles,v.dwell]},
  sweep:{d:'Sobe a amplitude e volta, aumentando o rpm a cada degrau. Aperte M quando o motor perder passo.',
    f:[['amp','Amplitude','°',90],['steps','Degraus','',15],['rpmMin','rpm inicial','rpm',''],['rpmMax','rpm final','rpm','']],a:v=>[v.amp,v.rpmMin,v.rpmMax,v.steps]},
  accsweep:{d:'Vai e volta com rpm fixo, aumentando a aceleração a cada degrau. Use rpm alto: em rpm baixo a rampa é curta e os degraus ficam parecidos. Aperte M quando perder passo.',
    f:[['amp','Amplitude','°',90],['steps','Degraus','',10],['accMin','Acel. inicial','p/s²',1000],['accMax','Acel. final','p/s²',20000],['rpm','Velocidade do teste','rpm',20]],a:v=>[v.amp,v.accMin,v.accMax,v.steps]},
  resonance:{d:'Giro contínuo num sentido, com o rpm subindo devagar. Serve para achar as faixas de vibração. Bloqueado com batentes ligados.',
    f:[['rpmMin','rpm inicial','rpm',''],['rpmMax','rpm final','rpm',''],['dur','Duração','s',30],['dir','Sentido','',1,'dir']],a:v=>[v.rpmMin,v.rpmMax,v.dur,v.dir]},
  backlash:{d:'Vai a +amp, volta a 0° e segura; vai a −amp, volta a 0° e segura. Meça a folga com um relógio comparador nas duas pausas.',
    f:[['amp','Amplitude','°',10],['hold','Segurar','ms',3000]],a:v=>[v.amp,v.hold]},
  drift:{d:'Muitas idas e voltas rápidas, terminando parado em 0°. Compare com uma marca física para ver se perdeu passo.',
    f:[['amp','Amplitude','°',90],['cycles','Ciclos','',20],['dwell','Pausa','ms',100]],a:v=>[v.amp,v.cycles,v.dwell]},
  stiction:{d:'Avança em degraus pequenos com pausa e depois volta ao início. Veja se o eixo acompanha cada degrau.',
    f:[['step','Degrau','°',0.25],['count','Quantidade','',10],['dwell','Pausa','ms',1000]],a:v=>[v.step,v.count,v.dwell]},
  waypoints:{d:'Percorre a lista de pontos em ordem, cada um com o seu rpm e a sua pausa.',f:[],wp:true},
};
let TST={sel:'sweep',vals:{}};
try{const x=JSON.parse(localStorage.getItem('nemabot.tst')||'null');if(x&&TESTS[x.sel])TST=x;}catch(e){}
const saveTst=()=>{try{localStorage.setItem('nemabot.tst',JSON.stringify(TST));}catch(e){}};
function tstVal(k,id,def){const v=(TST.vals[k]||{})[id];return v===undefined?def:v;}

// O que cada teste faz, mede e como ler (aparece embaixo do seletor de rotina)
const TINFO={
  loop:['Vai e volta a amplitude sem parar (ou pelo número de ciclos), com pausa em cada ponta.',
        'Se o conjunto aguenta uso contínuo: aquecimento do motor e do driver, desgaste e folga do cicloidal aumentando com o tempo.',
        'O movimento deve se repetir sempre igual. Se depois de muitos ciclos o motor engasgar ou a saída começar a errar o ponto, aperte M. Toque o motor e o driver para ver se esquentaram.'],
  sweep:['Sobe a amplitude e volta, aumentando o rpm a cada degrau (a aceleração fica fixa).',
        'A velocidade máxima em que o motor ainda acompanha, com a carga que estiver montada.',
        'O gráfico mostra o que o ESP32 mandou, não o eixo real. Quando o motor perder passo, ele chia, tranca ou não completa o curso: aperte M. O rpm daquele degrau é o seu limite; no braço, use uns 70% dele.'],
  accsweep:['Vai e volta com o rpm fixo, aumentando a aceleração a cada degrau.',
        'A aceleração máxima que o motor consegue dar à carga sem perder passo, ou seja, quanto torque sobra para arrancar e frear.',
        'Nos degraus baixos o movimento é suave. Passando do limite, o motor tranca na arrancada ou na freada: aperte M (o gráfico não mostra a perda de passo). Ficam anotados a aceleração do degrau e o pico de velocidade. Use rpm alto, senão a rampa é curta e os degraus ficam parecidos.'],
  resonance:['Gira sem parar num sentido, subindo o rpm devagar do valor inicial ao final.',
        'As faixas de rpm em que o motor vibra ou faz barulho (ressonância), onde é mais fácil perder passo.',
        'Escute e encoste a mão: quando vibrar forte, aperte M e o rpm daquele instante fica anotado. Evite essas faixas no uso. Só funciona com os batentes desligados, porque gira sem limite.'],
  backlash:['Vai a +amplitude, volta a 0° e segura; depois vai a −amplitude, volta a 0° e segura.',
        'A folga do cicloidal: a diferença de posição da saída ao chegar em 0° por um lado e pelo outro.',
        'Com um relógio comparador (ou um ponteiro comprido) na saída, anote a leitura nas duas pausas em 0°. A diferença entre elas é a folga. O gráfico não mostra a folga, porque o motor vai ao mesmo lugar nos dois casos.'],
  drift:['Faz muitas idas e voltas rápidas e termina parado exatamente em 0°.',
        'Se o motor perde passos que vão se acumulando ao longo de muitos movimentos.',
        'Antes de começar, faça uma marca no eixo (uma fita). No fim, se a marca não voltou ao mesmo lugar, houve perda de passo. A diferença dividida pelo número de ciclos dá a perda por ciclo.'],
  stiction:['Avança em degraus bem pequenos, com pausa em cada um, e depois volta ao início.',
        'O menor movimento que a saída consegue fazer de verdade: atrito, emperramento e micropasso efetivo.',
        'Observe a saída com um relógio comparador: ela deveria andar um pouco a cada degrau. Se ficar parada por vários degraus e depois pular de uma vez, o atrito está segurando. O menor degrau que ela acompanha é a sua resolução real.'],
  waypoints:['Percorre a lista de pontos em ordem, cada um com o seu rpm e a sua pausa (pode repetir).',
        'Um movimento parecido com o uso real do braço, com mudanças de sentido e de velocidade.',
        'Confira se ele chega em cada ponto sem engasgar e se, no fim, a saída volta exatamente ao mesmo lugar (use uma marca no eixo).'],
};
function renderInfo(k){
  const I=TINFO[k];if(!I){$('tdesc').textContent='';return;}
  $('tdesc').innerHTML='';
  [['Faz',I[0]],['Mede',I[1]],['Como ler',I[2]]].forEach(([l,t])=>{const d=document.createElement('div');d.innerHTML='<b>'+l+'</b><span></span>';d.querySelector('span').textContent=t;$('tdesc').appendChild(d);});
}
function buildFields(){
  const k=$('tsel').value,T=TESTS[k],box=$('tfields');
  renderInfo(k);box.innerHTML='';
  T.f.forEach(([id,label,unit,def,kind])=>{
    const w=document.createElement('div');
    if(kind==='dir'){
      w.innerHTML='<span class="lbl">'+label+'</span><label class="field sel"><select data-id="'+id+'"><option value="1">Positivo (+)</option><option value="-1">Negativo (−)</option></select></label>';
      w.querySelector('select').value=String(tstVal(k,id,def));
    }else{
      w.innerHTML='<span class="lbl">'+label+'</span><label class="field"><input type="number" step="any" data-id="'+id+'"'+(def===''?' placeholder="auto"':'')+'><span class="unit">'+unit+'</span></label>';
      const v=tstVal(k,id,def);w.querySelector('input').value=v;
    }
    box.appendChild(w);
  });
  box.querySelectorAll('[data-id]').forEach(el=>el.addEventListener('change',()=>{(TST.vals[k]=TST.vals[k]||{})[el.dataset.id]=el.value;saveTst();}));
  box.querySelectorAll('input').forEach(el=>el.addEventListener('keydown',e=>{if(e.key==='Enter'){el.blur();$('tgo').click();}}));
  $('wpEd').hidden=!T.wp;
  TST.sel=k;saveTst();updateAutoHints();
}
function updateAutoHints(){                 // mostra no placeholder o valor que o ESP32 vai usar
  if(!S)return;
  $('tfields').querySelectorAll('input[placeholder]').forEach(el=>{
    const id=el.dataset.id;el.placeholder='auto · '+(id==='rpmMin'?(S.rpmMax/15).toFixed(1):(+S.rpmMax).toFixed(0));
  });
}
$('tsel').value=TST.sel;$('tsel').onchange=buildFields;

// editor de waypoints (salvo no navegador)
let WP={pts:[{deg:0,rpm:10,dwell:500},{deg:90,rpm:10,dwell:500},{deg:-45,rpm:5,dwell:500}],loop:false};
try{const x=JSON.parse(localStorage.getItem('nemabot.wp')||'null');if(x&&Array.isArray(x.pts)&&x.pts.length>=2)WP=x;}catch(e){}
const saveWp=()=>{try{localStorage.setItem('nemabot.wp',JSON.stringify(WP));}catch(e){}};
function renderWp(){
  const box=$('wpRows');box.innerHTML='';
  WP.pts.forEach((p,i)=>{
    const r=document.createElement('div');r.className='wprow';
    r.innerHTML='<label class="field"><input type="number" step="any" data-k="deg"><span class="unit">°</span></label>'+
      '<label class="field"><input type="number" step="any" data-k="rpm"><span class="unit">rpm</span></label>'+
      '<label class="field"><input type="number" step="any" data-k="dwell"><span class="unit">ms</span></label>'+
      '<button class="btn sm icon" title="Remover ponto"'+(WP.pts.length<=2?' disabled':'')+'>×</button>';
    r.querySelectorAll('input').forEach(el=>{el.value=p[el.dataset.k];el.addEventListener('change',()=>{p[el.dataset.k]=+el.value;saveWp();});});
    r.querySelector('button').onclick=()=>{WP.pts.splice(i,1);saveWp();renderWp();};
    box.appendChild(r);
  });
  $('wpAdd').disabled=WP.pts.length>=8;$('wpLoop').checked=!!WP.loop;
}
$('wpAdd').onclick=()=>{if(WP.pts.length>=8)return;const l=WP.pts[WP.pts.length-1];WP.pts.push({deg:l.deg,rpm:l.rpm,dwell:l.dwell});saveWp();renderWp();};
$('wpLoop').onchange=()=>{WP.loop=$('wpLoop').checked;saveWp();};

// iniciar / parar
let S=null,testRunning=false,TSTART=null,lastSub='',lastMode='';
$('tgo').onclick=async()=>{
  if(testRunning){const j=await send('stop',0,$('tgo'));if(j.ok)toast('Teste parado','warn');delete $('tgo').dataset.run;return;}
  const k=$('tsel').value,T=TESTS[k];
  if(k==='resonance'&&S&&S.limEn){toast('Ressonância bloqueada: desligue os batentes em Sistema','warn');return;}
  setBtnLoading($('tgo'),true,'Iniciando…');
  let j;
  if(T.wp){
    if(S&&S.limEn){const bad=WP.pts.find(p=>p.deg<S.limMin||p.deg>S.limMax);if(bad){setBtnLoading($('tgo'),false);toast('O ponto '+bad.deg+'° está fora dos batentes','warn');return;}}
    resumeLive();
    j=await api('/waypoints',{method:'POST',body:new URLSearchParams({pts:WP.pts.map(p=>[p.deg,p.rpm,p.dwell].join(',')).join(';'),loop:WP.loop?1:0})});
    if(!j.ok)toast(j.error||'Não consegui enviar a trajetória','err');
  }else{
    const v={};$('tfields').querySelectorAll('[data-id]').forEach(el=>v[el.dataset.id]=el.value);
    if(k==='accsweep'&&+v.rpm>0){const r=await send('rpm',v.rpm);if(!r.ok){setBtnLoading($('tgo'),false);return;}}
    const a=T.a(v);j=await send(k,a[0],null,a[1],a[2],a[3]);
    if(j.ok)TSTART={k,v};
  }
  setBtnLoading($('tgo'),false);delete $('tgo').dataset.run;
  if(j.ok)toast('Teste iniciado: '+MODE_LABEL[k],'info');
};

// marcar falha
let RES=[];
try{RES=JSON.parse(localStorage.getItem('nemabot.res')||'[]');}catch(e){}
const saveRes=()=>{try{localStorage.setItem('nemabot.res',JSON.stringify(RES));}catch(e){}};
function resText(r){return '#'+r.n+'  '+r.label+'  |  rpm '+r.rpm+'  |  acel '+r.acc+'  |  pos '+r.pos+'°'+(r.sub?'  |  '+r.sub:'');}
function renderRes(){
  const box=$('tres');
  if(!RES.length){box.innerHTML='<div class="sub" style="padding:12px">Nenhuma marcação ainda. Durante um teste, aperte M quando o motor falhar.</div>';return;}
  box.innerHTML='';
  RES.forEach(r=>{const d=document.createElement('div');d.className='res';
    d.innerHTML='<span class="n">#'+r.n+'</span><span><span class="m">'+r.label+'</span> · rpm '+r.rpm+' · acel '+r.acc+' · pos '+r.pos+'°'+(r.sub?' · <span class="m">'+r.sub+'</span>':'')+'</span>';
    box.appendChild(d);});
  box.scrollTop=box.scrollHeight;
}
function markFail(){
  if(!testRunning||!S)return;
  send('stop');freeze();
  const n=RES.length?RES[RES.length-1].n+1:1;
  const rpm=S.mode==='sweep'||S.mode==='resonance'?(+S.sweep).toFixed(1):(+S.rpmSet).toFixed(1);
  const acc=S.mode==='accsweep'?(+S.stepAcc).toFixed(0):(+S.acc).toFixed(0);
  const r={n,label:MODE_LABEL[S.mode]||S.mode,rpm,acc,pos:S.pos.toFixed(1),sub:S.subMode||''};
  RES.push(r);saveRes();renderRes();
  G.marks.push({t:lastT(),label:'✕'+n});if(curRun)curRun.marks.push({t:lastT()-curRun.t0,label:'✕'+n});draw();
  toast('Falha #'+n+' marcada: rpm '+rpm+', acel '+acc,'err');
}
$('tmark').onclick=markFail;
$('rcopy').onclick=()=>copyText(RES.map(resText).join('\n')).then(()=>toast('Marcações copiadas','ok'),()=>toast('Não consegui copiar','err'));
$('rclear').onclick=()=>{RES=[];saveRes();renderRes();G.marks.length=0;draw();toast('Marcações limpas','info');};

// ---------- Segurança e hardware ----------
let safetyShown=false;
$('safeSave').onclick=async()=>{
  const lo=+$('limMin').value,hi=+$('limMax').value;
  if($('limEn').checked&&!(lo<hi)){toast('O mínimo precisa ser menor que o máximo','warn');return;}
  setBtnLoading($('safeSave'),true,'Salvando…');
  const j=await api('/safety',{method:'POST',body:new URLSearchParams({limEn:$('limEn').checked?1:0,limMin:lo,limMax:hi,enaUse:$('enaUse').checked?1:0,enaInv:$('enaInv').checked?1:0})});
  setBtnLoading($('safeSave'),false);
  j.ok?toast('Segurança salva'+($('limEn').checked?' · batentes '+lo+'° a '+hi+'°':''),'ok'):toast(j.error||'Não consegui salvar','err');
};
const LOCK_I='<svg width="13" height="13" viewBox="0 0 12 14" fill="none" stroke="currentColor" stroke-width="1.6"><rect x="1.5" y="6" width="9" height="7" rx="1.5"/><path d="M3.5 6V4a2.5 2.5 0 0 1 5 0v2"/></svg>';
const FREE_I='<svg width="13" height="13" viewBox="0 0 12 14" fill="none" stroke="currentColor" stroke-width="1.6"><rect x="1.5" y="6" width="9" height="7" rx="1.5"/><path d="M3.5 6V4a2.5 2.5 0 0 1 4.9-.7"/></svg>';
let enaWant=null;
$('enaBtn').onclick=async()=>{
  if(!S)return;const on=!S.enaOn;enaWant=on;
  const j=await send('ena',on?1:0);
  if(j.ok)toast(on?'Torque ligado: motor travado':'Soltando o torque (o motor para antes)',on?'ok':'warn');
};
$('limBadge').onclick=()=>{showTab('sys');setTimeout(()=>$('safety').scrollIntoView({behavior:'smooth',block:'start'}),30);};
function applyLimitsUI(s){
  const en=!!s.limEn,lo=+s.limMin,hi=+s.limMax;
  $('limBadge').hidden=!en;
  if(en)$('limTxt').textContent=lo.toFixed(lo%1?1:0).replace('-','−')+'° … '+hi.toFixed(hi%1?1:0).replace('-','−')+'°';
  if(en){
    const a=Math.max(0,Math.min(100,(lo+360)/7.2)),b=Math.max(0,Math.min(100,(hi+360)/7.2));
    sl.style.setProperty('--trk','linear-gradient(90deg,#141419 0 '+a+'%,rgba(147,165,255,.28) '+a+'% '+b+'%,#141419 '+b+'% 100%)');
  }else sl.style.removeProperty('--trk');
  document.querySelectorAll('.chip[data-g]').forEach(c=>{const g=+c.dataset.g;c.disabled=en&&(g<lo||g>hi);});
  // faixa escura no mostrador fora de [mín, máx]
  if(en&&hi-lo<360){
    $('dlim').setAttribute('d',arcPath(82,hi,lo+360));
    const L=a2=>{const s_=Math.sin(a2*Math.PI/180),c_=-Math.cos(a2*Math.PI/180);return 'M'+(70*s_).toFixed(1)+','+(70*c_).toFixed(1)+'L'+(94*s_).toFixed(1)+','+(94*c_).toFixed(1);};
    $('dlimE').setAttribute('d',L(lo)+L(hi));
  }else{$('dlim').setAttribute('d','');$('dlimE').setAttribute('d','');}
}
sl.addEventListener('input',()=>{if(S&&S.limEn){const v=Math.max(+S.limMin,Math.min(+S.limMax,+sl.value));if(v!=+sl.value){sl.value=v;$('slv').textContent=v;}}},true);

// ---------- Progresso do teste em andamento ----------
const fmtN=(x,d)=>(+x).toLocaleString('pt-BR',{minimumFractionDigits:d||0,maximumFractionDigits:d||0});
function testProgress(s){
  const v=(TSTART&&TSTART.k===s.mode)?TSTART.v:{};
  const n=x=>x===''||x===undefined?null:+x;
  let step='',val='',frac=null;
  switch(s.mode){
    case 'sweep':{const lo=n(v.rpmMin)||s.rpmMax/15,hi=n(v.rpmMax)||s.rpmMax,N=n(v.steps)||15;
      step='Degrau '+Math.min(s.cycles+1,N)+' de '+N;val=fmtN(s.sweep,1)+' rpm';frac=(s.cycles+1)/N;break;}
    case 'accsweep':{const N=n(v.steps)||10;step='Degrau '+Math.min(s.cycles+1,N)+' de '+N;val=fmtN(s.stepAcc)+' p/s²';frac=(s.cycles+1)/N;break;}
    case 'resonance':{const lo=n(v.rpmMin)||s.rpmMax/15,hi=n(v.rpmMax)||s.rpmMax;step='Subindo o rpm';val=fmtN(s.sweep,1)+' rpm';frac=hi>lo?(s.sweep-lo)/(hi-lo):null;break;}
    case 'loop':{const N=n(v.cycles)||0;step=N?'Ciclo '+(s.cycles+1)+' de '+N:'Ciclo '+(s.cycles+1)+' (sem fim)';val=fmtN(s.rpmSet,1)+' rpm';frac=N?(s.cycles+1)/N:null;break;}
    case 'drift':{const N=n(v.cycles)||20;step='Ciclo '+(s.cycles+1)+' de '+N;val=fmtN(s.pos,1)+'°';frac=(s.cycles+1)/N;break;}
    case 'backlash':{const p={'→ +':'Indo para +amp','→ 0':'Voltando a 0°','→ −':'Indo para −amp','hold+':'Segurando em 0° (vindo de +)','hold-':'Segurando em 0° (vindo de −)'};
      step=p[s.subMode]||s.subMode;val=fmtN(s.pos,2)+'°';break;}
    case 'stiction':step=s.subMode==='voltando'?'Voltando ao início':'Degrau '+s.subMode;val=fmtN(s.pos,3)+'°';{const mm=/(\d+)\/(\d+)/.exec(s.subMode);if(mm)frac=mm[1]/mm[2];}break;
    case 'waypoints':step='Ponto '+s.subMode.replace('wp ','').replace('/',' de ');val=fmtN(s.target,1)+'° · '+fmtN(s.rpmSet,1)+' rpm';break;
  }
  return {step,val,frac};
}

// ---------- Gravações dos testes e comparação com a anterior ----------
let RUNS=[],curRun=null,GH=null,wasRunning=false;
try{RUNS=JSON.parse(localStorage.getItem('nemabot.runs')||'[]');}catch(e){RUNS=[];}
let runSeq=RUNS.reduce((m,r)=>Math.max(m,r.id),0);
function saveRuns(){
  const keep=RUNS.slice(-6).map(r=>{               // guarda as 6 últimas; reduz para 25 Hz se ficar grande
    if(r.t.length<=3000)return r;
    const o=Object.assign({},r,{t:[],p:[],g:[],v:[]});
    for(let i=0;i<r.t.length;i+=4){o.t.push(r.t[i]);o.p.push(r.p[i]);o.g.push(r.g[i]);o.v.push(r.v[i]);}
    return o;});
  RUNS=keep;
  try{localStorage.setItem('nemabot.runs',JSON.stringify(RUNS));}catch(e){try{RUNS=RUNS.slice(-2);localStorage.setItem('nemabot.runs',JSON.stringify(RUNS));}catch(x){}}
}
function runLabel(mode){
  const v=(TSTART&&TSTART.k===mode)?TSTART.v:{};let d='';
  if(mode==='accsweep')d=(v.accMin||1000)+'→'+(v.accMax||20000)+' p/s²'+(v.rpm?' · '+v.rpm+' rpm':'');
  else if(mode==='sweep')d=(v.rpmMin||'auto')+'→'+(v.rpmMax||'auto')+' rpm';
  else if(mode==='loop'||mode==='drift')d=(v.amp||90)+'°';
  return (MODE_LABEL[mode]||mode)+(d?' · '+d:'');
}
function startRun(s){
  const t0=s.samples.length?s.samples[0][0]/1000:lastT();
  curRun={id:++runSeq,mode:s.mode,label:runLabel(s.mode),p0:null,when:new Date().toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit'}),t0,t:[],p:[],g:[],v:[],marks:[],steps:[]};
  const prev=[...RUNS].reverse().find(r=>r.mode===s.mode);   // compara automaticamente com a anterior da mesma rotina
  GH=prev||null;renderRuns();
}
function recordRun(s){
  if(!curRun)return;
  for(const [t,p,g,v] of s.samples){const tt=t/1000-curRun.t0;if(tt<0)continue;if(curRun.p0===null)curRun.p0=p;curRun.t.push(+tt.toFixed(3));curRun.p.push(p);curRun.g.push(g);curRun.v.push(v);}
}
function endRun(){
  if(!curRun)return;
  const r=curRun;
  if(r.t.length>20){RUNS.push(r);saveRuns();}
  curRun=null;renderRuns();
  if(r.t.length>20){fitView(r);toast('Teste encerrado: o gráfico mostra o teste inteiro. "Continuar" volta ao tempo real.','info');}
}
function baseRun(){                                  // alinhamento: execução atual (ou a última, se parado)
  if(curRun)return curRun;
  return [...RUNS].reverse().find(r=>r!==GH)||null;
}
let ghCache={key:'',a:null};
function ghostArr(id){
  if(!GH)return null;const B=baseRun();if(!B)return null;
  if(id!=='cp')return {t:GH.t,a:GH.v,marks:GH.marks,base:B.t0};
  // posição: desloca para começar no mesmo ângulo da execução atual (os testes são relativos ao início)
  const off=(B.p0!==null&&B.p0!==undefined&&GH.p0!==null&&GH.p0!==undefined)?B.p0-GH.p0:0;
  const key=GH.id+':'+off.toFixed(3)+':'+GH.p.length;
  if(ghCache.key!==key)ghCache={key,a:off?GH.p.map(v=>v+off):GH.p};
  return {t:GH.t,a:ghCache.a,marks:GH.marks,base:B.t0};
}
function renderRuns(){
  const box=$('runs');
  $('ghostLeg').hidden=!GH;if(GH)$('ghostName').textContent='#'+GH.id+' '+GH.label;
  if(!RUNS.length){box.innerHTML='<div class="sub" style="padding:12px">Nenhuma execução gravada. Cada teste que você rodar fica salvo aqui.</div>';draw();return;}
  box.innerHTML='';
  [...RUNS].reverse().forEach(r=>{
    const d=document.createElement('div');d.className='run'+(r===GH?' on':'');
    const dur=r.t.length?r.t[r.t.length-1]:0;
    d.innerHTML='<div class="ri"><b>#'+r.id+'</b> <span></span><div class="sub">'+r.when+' · '+Math.round(dur)+' s'+(r.marks.length?' · <span style="color:var(--accent-danger)">'+r.marks.length+' falha'+(r.marks.length>1?'s':'')+'</span>':'')+'</div></div>'+
      '<button class="btn sm cmp">'+(r===GH?'Comparando':'Comparar')+'</button><button class="btn sm csv">CSV</button><button class="btn sm icon del" title="Apagar">×</button>';
    d.querySelector('.ri span').textContent=r.label;
    d.querySelector('.cmp').onclick=()=>{GH=(GH===r)?null:r;renderRuns();};
    d.querySelector('.csv').onclick=()=>{
      let s='tempo_s,posicao_graus,alvo_graus,velocidade_rpm\n';
      for(let i=0;i<r.t.length;i++)s+=r.t[i]+','+r.p[i]+','+r.g[i]+','+r.v[i]+'\n';
      if(r.marks.length)s+='\nfalhas\n'+r.marks.map(m=>m.t.toFixed(3)+','+m.label).join('\n')+'\n';
      const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([s],{type:'text/csv'}));a.download='nemabot_teste_'+r.id+'_'+r.mode+'.csv';a.click();};
    d.querySelector('.del').onclick=()=>{RUNS=RUNS.filter(x=>x!==r);if(GH===r)GH=null;saveRuns();renderRuns();};
    box.appendChild(d);
  });
  draw();
}
$('ghostX').onclick=()=>{GH=null;renderRuns();};
