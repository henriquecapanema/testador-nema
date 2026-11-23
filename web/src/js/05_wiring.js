
// ---------- Ligações: diagrama ESP32 + driver ----------
const TB_MS=[['1',1,'ON','ON','OFF',200],['2/A',2,'ON','OFF','ON',400],['2/B',2,'OFF','ON','ON',400],['4',4,'ON','OFF','OFF',800],['8',8,'OFF','ON','OFF',1600],['16',16,'OFF','OFF','ON',3200],['32',32,'OFF','OFF','OFF',6400]];
const TB_CUR=[[0.5,0.7,'ON','ON','ON'],[1.0,1.2,'ON','OFF','ON'],[1.5,1.7,'ON','ON','OFF'],[2.0,2.2,'ON','OFF','OFF'],[2.5,2.7,'OFF','ON','ON'],[2.8,2.9,'OFF','OFF','ON'],[3.0,3.2,'OFF','ON','OFF'],[3.5,4.0,'OFF','OFF','OFF']];
const TMC_MS={8:['GND','GND'],16:['VDD','VDD'],32:['VDD','GND'],64:['GND','VDD']};   // [MS1, MS2]
let WIRE={drv:'tb6600',tbMs:'8',tbCur:1.5,tmcMs:16,irms:1.0};
try{const x=JSON.parse(localStorage.getItem('nemabot.wire')||'null');if(x&&x.drv)WIRE=Object.assign(WIRE,x);}catch(e){}
const saveWire=()=>{try{localStorage.setItem('nemabot.wire',JSON.stringify(WIRE));}catch(e){}};
const C={step:'#93A5FF',dir:'#67E8F9',ena:'#FCD34D',gnd:'#8A8A9C',v24:'#FB7185',v5:'#6EE7B7',uart:'#C4B5FD',mA1:'#F87171',mA2:'#4ADE80',mB1:'#FACC15',mB2:'#60A5FA'};

function wiringSVG(){
  const tb=WIRE.drv==='tb6600',ena=!!(S&&S.enaUse);
  let o='',wires=[];
  const T=(x,y,t,a,cls,extra)=>'<text x="'+x+'" y="'+y+'"'+(a?' text-anchor="'+a+'"':'')+' class="'+(cls||'wl')+'"'+(extra||'')+'>'+t+'</text>';
  const W=(id,label,color,pts,opt)=>wires.push({id,label,color,pts,opt:opt||{}});
  const curve=(a,b,bend)=>{const dx=(bend===undefined?Math.max(60,Math.abs(b[0]-a[0])*0.45):bend);const s=b[0]>a[0]?1:-1;
    return 'M'+a[0]+','+a[1]+' C'+(a[0]+s*dx)+','+a[1]+' '+(b[0]-s*dx)+','+b[1]+' '+b[0]+','+b[1];};

  // ---- ESP32 (direita) ----
  const ex=800,ey=56,ew=232,eh=500;
  const L=['EN','VP','VN','D34','D35','D32','D33','D25','D26','D27','D14','D12','D13','GND','VIN'];
  const R=['D23','D22','TX0','RX0','D21','D19','D18','D5','TX2','RX2','D4','D2','D15','GND','3V3'];
  const py=i=>ey+86+i*27;
  const used={D25:C.step,D26:C.dir,D27:ena?C.ena:null,GND:C.gnd,VIN:C.v5};
  const usedR={'3V3':tb?null:C.v5,GND:tb?null:C.gnd,RX2:tb?null:C.uart};
  o+='<g class="board"><rect x="'+ex+'" y="'+ey+'" width="'+ew+'" height="'+eh+'" rx="16" fill="url(#pcb)" stroke="#2C3140"/>';
  o+='<path d="M'+(ex+70)+','+(ey+14)+' h20 v14 h12 v-14 h12 v14 h12 v-14 h12 v14 h12 v-14 h20" fill="none" stroke="#C9A227" stroke-width="2.4" stroke-linejoin="round" opacity=".85"/>';
  o+='<rect x="'+(ex+62)+'" y="'+(ey+42)+'" width="108" height="118" rx="6" fill="url(#chip)" stroke="#3A3F4D"/>'+T(ex+116,ey+96,'ESP32','middle','chipt')+T(ex+116,ey+114,'WROOM-32','middle','chips');
  o+='<rect x="'+(ex+90)+'" y="'+(ey+eh-26)+'" width="52" height="30" rx="5" fill="#C7C9D1" stroke="#7B7F8C"/>';
  o+=T(ex+116,ey+eh-40,'ESP32 DevKit','middle','bname');
  L.forEach((n,i)=>{const y=py(i),c=used[n];o+='<circle cx="'+(ex+18)+'" cy="'+y+'" r="6.5" fill="#0B0C10" stroke="'+(c||'#4A4F5E')+'" stroke-width="'+(c?2.5:1.4)+'"/>'+(c?'<circle cx="'+(ex+18)+'" cy="'+y+'" r="2.6" fill="'+c+'"/>':'')+T(ex+32,y+4,n,'',c?'pin on':'pin');});
  R.forEach((n,i)=>{const y=py(i),c=usedR[n];o+='<circle cx="'+(ex+ew-18)+'" cy="'+y+'" r="6.5" fill="#0B0C10" stroke="'+(c||'#4A4F5E')+'" stroke-width="'+(c?2.5:1.4)+'"/>'+(c?'<circle cx="'+(ex+ew-18)+'" cy="'+y+'" r="2.6" fill="'+c+'"/>':'')+T(ex+ew-32,y+4,n,'end',c?'pin on':'pin');});
  o+='</g>';
  const P=n=>[ex+18,py(L.indexOf(n))],PR=n=>[ex+ew-18,py(R.indexOf(n))];

  // ---- motor (esquerda) ----
  const mx=40,my=150;
  o+='<g class="board"><rect x="'+mx+'" y="'+my+'" width="150" height="150" rx="14" fill="url(#metal)" stroke="#4A4F5E"/>';
  [[14,14],[136,14],[14,136],[136,136]].forEach(([a,b])=>o+='<circle cx="'+(mx+a)+'" cy="'+(my+b)+'" r="5" fill="#0B0C10" stroke="#5B6070"/>');
  o+='<circle cx="'+(mx+75)+'" cy="'+(my+75)+'" r="34" fill="#16181F" stroke="#5B6070"/><circle cx="'+(mx+75)+'" cy="'+(my+75)+'" r="9" fill="#C7C9D1"/>';
  o+=T(mx+75,my+176,'NEMA 17','middle','bname')+T(mx+75,my+193,'17HS4401','middle','chips')+'</g>';
  const MW=[['A+',C.mA1,'vermelho'],['A−',C.mA2,'verde'],['B+',C.mB1,'amarelo'],['B−',C.mB2,'azul']];
  const mp=i=>[mx+150,my+30+i*30];

  // ---- fonte 24 V e conversor 5 V (embaixo) ----
  const fx=250,fy=520;
  o+='<g class="board"><rect x="'+fx+'" y="'+fy+'" width="230" height="74" rx="10" fill="#15161C" stroke="#3A3F4D"/>'+T(fx+20,fy+30,'Fonte 24 V','','bname',' text-anchor="start"')+T(fx+20,fy+50,'5 A · 120 W','','chips',' text-anchor="start"');
  o+='<rect x="'+(fx+150)+'" y="'+(fy+16)+'" width="64" height="42" rx="5" fill="#0B0C10" stroke="#3A3F4D"/>'+T(fx+168,fy+42,'+','middle','pin on')+T(fx+196,fy+42,'−','middle','pin on')+'</g>';
  const f24=[fx+168,fy+16],fG=[fx+196,fy+16];
  const bx=560,by=520;
  o+='<g class="board"><rect x="'+bx+'" y="'+by+'" width="170" height="74" rx="10" fill="#101A16" stroke="#2C4A3C"/>'+T(bx+18,by+30,'Step-down','','bname',' text-anchor="start"')+T(bx+18,by+50,'24 V → 5 V','','chips',' text-anchor="start"')+'</g>';
  const bIn=[bx,by+37],b5=[bx+170,by+26],bG=[bx+170,by+50];

  if(tb){
    // ---- TB6600 (como o seu: 12 bornes numa fileira, chaves com ON para baixo) ----
    const tx=270,ty=70,tw=350,th=380;
    o+='<g class="board"><rect x="'+tx+'" y="'+ty+'" width="'+tw+'" height="'+th+'" rx="12" fill="#14161B" stroke="#30343F"/>';
    for(let i=0;i<11;i++)o+='<rect x="'+(tx+22+i*29)+'" y="'+(ty+12)+'" width="16" height="52" rx="4" fill="#1B1E25" stroke="#272B34"/>';
    o+=T(tx+tw/2,ty+98,'TB6600','middle','bname big')+T(tx+tw/2,ty+117,'Microstep Driver · DC 9–42 V','middle','chips');
    o+='<circle cx="'+(tx+tw-26)+'" cy="'+(ty+96)+'" r="5" fill="#3B82F6"/>'+T(tx+tw-36,ty+100,'PWR','end','chips');
    // chaves DIP (SW1–SW6, ON para baixo)
    const ms=TB_MS.find(m=>m[0]===WIRE.tbMs)||TB_MS[4],cu=TB_CUR.find(c=>c[0]===+WIRE.tbCur)||TB_CUR[2];
    const sw=[ms[2],ms[3],ms[4],cu[2],cu[3],cu[4]];
    const dx0=tx+tw/2-96,dy0=ty+142;
    o+='<rect x="'+(dx0-12)+'" y="'+(dy0-10)+'" width="216" height="104" rx="8" fill="#C81E1E" opacity=".95"/>';
    sw.forEach((v,i)=>{const x=dx0+i*32,on=v==='ON';
      o+=T(x+14,dy0+8,String(i+1),'middle','dipt')+'<rect x="'+(x+5)+'" y="'+(dy0+14)+'" width="18" height="46" rx="3" fill="#7F1414"/>'+
        '<rect class="dip" x="'+(x+7)+'" y="'+(on?dy0+36:dy0+16)+'" width="14" height="22" rx="2" fill="#F4F4F6"/>'+T(x+14,dy0+76,v,'middle','dipt sm');});
    o+=T(dx0-2,dy0+92,'ON ↓','','dipt')+T(dx0+190,dy0+92,'DIP','end','dipt');
    o+=T(dx0+46,dy0+118,'SW1–3 micropasso','middle','chips')+T(dx0+148,dy0+118,'SW4–6 corrente','middle','chips');
    // bornes: uma fileira na borda de baixo
    const terms=['VCC','GND','A+','A−','B+','B−','PUL+','PUL−','DIR+','DIR−','ENA+','ENA−'];
    const kx=i=>tx+30+i*26.5+(i>=6?10:0),ky=ty+th;
    o+=T(kx(2.5),ky-74,'High Voltage','middle','chips')+T(kx(8.5),ky-74,'Signal','middle','chips');
    o+='<path d="M'+(kx(0)-8)+','+(ky-66)+' h'+(kx(5)-kx(0)+16)+'M'+(kx(6)-8)+','+(ky-66)+' h'+(kx(11)-kx(6)+16)+'" stroke="#3A3F4D" stroke-width="1.2"/>';
    terms.forEach((n,i)=>{const x=kx(i);
      o+='<text transform="translate('+(x+4)+','+(ky-10)+') rotate(-90)" class="pin on">'+n+'</text>'+
        '<rect x="'+(x-11)+'" y="'+(ky-2)+'" width="22" height="26" rx="3" fill="#2E8B57" stroke="#1F6B42"/><circle cx="'+x+'" cy="'+(ky+11)+'" r="5.5" fill="#C9CCD6" stroke="#7B7F8C"/><path d="M'+(x-3.5)+','+(ky+11)+'h7" stroke="#55596A" stroke-width="1.6"/>';});
    o+='</g>';
    const K=n=>[kx(terms.indexOf(n)),ky+24];
    W('step','STEP · D25 → PUL+',C.step,[P('D25'),K('PUL+')],{up:1});
    W('dir','DIR · D26 → DIR+',C.dir,[P('D26'),K('DIR+')],{up:1});
    W('ena','ENA · D27 → ENA+'+(ena?'':' (opcional, desligado em Sistema)'),C.ena,[P('D27'),K('ENA+')],{up:1,off:!ena});
    W('gnd1','GND do ESP32 → PUL−',C.gnd,[P('GND'),K('PUL−')],{up:1});
    W('gnd2','GND do ESP32 → DIR−',C.gnd,[P('GND'),K('DIR−')],{up:1});
    W('gnd3','GND do ESP32 → ENA−',C.gnd,[P('GND'),K('ENA−')],{up:1,off:!ena});
    W('v24','24 V → VCC',C.v24,[f24,K('VCC')],{vert:1});
    W('g24','GND da fonte → GND',C.gnd,[fG,K('GND')],{vert:1});
    MW.forEach(([n,col,nome],i)=>W('m'+i,'Bobina '+n+' ('+nome+') → '+n,col,[mp(i),K(n)],{down:1,i,lx:tx-16-(3-i)*9}));
  }else{
    // ---- TMC2209 ----
    const tx=330,ty=120,tw=200,th=330;
    o+='<g class="board"><rect x="'+tx+'" y="'+ty+'" width="'+tw+'" height="'+th+'" rx="12" fill="url(#tmc)" stroke="#5B3FA0"/>';
    o+='<rect x="'+(tx+50)+'" y="'+(ty+70)+'" width="100" height="100" rx="8" fill="#1A1B22" stroke="#3A3F4D"/>';
    for(let i=0;i<7;i++)o+='<rect x="'+(tx+56+i*13)+'" y="'+(ty+78)+'" width="8" height="84" rx="2" fill="#2A2D36"/>';
    o+=T(tx+tw/2,ty+40,'TMC2209','middle','bname big')+T(tx+tw/2,ty+58,'BTT v1.3 · UART','middle','chips');
    const lg=['VM','GND','2B','2A','1A','1B','VDD','GND'],rg=['EN','MS1','MS2','PDN','CLK','STEP','DIR'];
    const ly=i=>ty+186+i*17,ry=i=>ty+186+i*19;
    lg.forEach((n,i)=>{o+='<circle cx="'+tx+'" cy="'+ly(i)+'" r="4" fill="#C9A227"/>'+T(tx+12,ly(i)+4,n,'','pin on');});
    rg.forEach((n,i)=>{o+='<circle cx="'+(tx+tw)+'" cy="'+ry(i)+'" r="4" fill="#C9A227"/>'+T(tx+tw-12,ry(i)+4,n,'end','pin on');});
    // MS1/MS2 conforme o micropasso
    const msv=TMC_MS[WIRE.tmcMs]||TMC_MS[16];
    [['MS1',msv[0]],['MS2',msv[1]]].forEach(([n,v])=>{const y=ry(rg.indexOf(n));
      o+='<rect x="'+(tx+tw+14)+'" y="'+(y-8)+'" width="40" height="16" rx="8" fill="'+(v==='VDD'?'rgba(110,231,183,.16)':'rgba(138,138,156,.16)')+'" stroke="'+(v==='VDD'?C.v5:C.gnd)+'"/>'+T(tx+tw+34,y+4,v,'middle','pin on small');});
    // capacitor 100 µF
    o+='<g transform="translate('+(tx+tw/2-11)+','+(ty+th+14)+')"><rect x="0" y="0" width="22" height="30" rx="4" fill="#1D3B6B" stroke="#3B6BB5"/><rect x="0" y="0" width="22" height="6" rx="3" fill="#9CB8E8"/></g>'+T(tx+tw/2+20,ty+th+34,'100 µF no VM','','chips');
    o+='</g>';
    const LG=n=>[tx,ly(lg.indexOf(n))],LG2=i=>[tx,ly(i)],RG=n=>[tx+tw,ry(rg.indexOf(n))];
    W('step','STEP · D25 → STEP',C.step,[P('D25'),RG('STEP')],{sig:1});
    W('dir','DIR · D26 → DIR',C.dir,[P('D26'),RG('DIR')],{sig:1});
    W('ena','EN · D27 → EN (LOW = torque)'+(ena?'':' (opcional)'),C.ena,[P('D27'),RG('EN')],{sig:1,off:!ena});
    W('uart','UART · RX2 (GPIO16) → PDN, com resistor de 1 kΩ (futuro)',C.uart,[PR('RX2'),RG('PDN')],{off:1,around:1});
    W('vdd','3V3 do ESP32 → VDD (lógica)',C.v5,[PR('3V3'),LG2(6)],{around:1});
    W('gl','GND do ESP32 → GND (lógica)',C.gnd,[P('GND'),LG2(7)],{under:1});
    W('v24','24 V → VM',C.v24,[f24,LG('VM')],{power:22});
    W('g24','GND da fonte → GND (motor)',C.gnd,[fG,LG2(1)],{power:34});
    const tm=[['A+','1A'],['A−','1B'],['B+','2A'],['B−','2B']];
    MW.forEach(([n,col,nome],i)=>W('m'+i,'Bobina '+n+' ('+nome+') → '+tm[i][1],col,[mp(i),LG(tm[i][1])],{motor:1}));
  }
  // alimentação do ESP32 (comum aos dois)
  W('b24','24 V → entrada do step-down',C.v24,[[fx+230,fy+37],bIn],{flat:1});
  W('b5','5 V do step-down → VIN',C.v5,[b5,P('VIN')]);
  W('bg','GND do step-down → GND do ESP32 (terra comum)',C.gnd,[bG,P('GND')]);

  // desenha os fios
  let wl='';
  wires.forEach(w=>{
    const [a,b]=w.pts;let d;
    if(w.opt.around){const yb=ey+eh+50,side=w.id==='uart'?1:-1,ex2=b[0]+side*60;
      d='M'+a[0]+','+a[1]+' C'+(a[0]+50)+','+a[1]+' '+(a[0]+50)+','+yb+' '+(a[0]-20)+','+yb+' L'+(ex2-side*10)+','+yb+' C'+(ex2+side*30)+','+yb+' '+(b[0]+side*50)+','+b[1]+' '+b[0]+','+b[1];}
    else if(w.opt.under){const yb=b[1]+90;d='M'+a[0]+','+a[1]+' C'+(a[0]-80)+','+a[1]+' '+(a[0]-60)+','+yb+' '+(a[0]-140)+','+yb+' L'+(b[0]-40)+','+yb+' C'+(b[0]-70)+','+yb+' '+(b[0]-60)+','+b[1]+' '+b[0]+','+b[1];}
    else if(w.opt.up){d='M'+a[0]+','+a[1]+' C'+(a[0]-140)+','+a[1]+' '+b[0]+','+(b[1]+70)+' '+b[0]+','+b[1];}       // ESP32 → borne (entra por baixo)
    else if(w.opt.down){const lx=w.opt.lx,yb=b[1]+14+w.opt.i*6;   // motor → desce pela esquerda da caixa → entra no borne por baixo
      d='M'+a[0]+','+a[1]+' C'+(a[0]+24)+','+a[1]+' '+lx+','+a[1]+' '+lx+','+(a[1]+24)+' L'+lx+','+(yb-10)+' Q'+lx+','+yb+' '+(lx+10)+','+yb+' L'+(b[0]-8)+','+yb+' Q'+b[0]+','+yb+' '+b[0]+','+(yb-8)+' L'+b[0]+','+b[1];}
    else if(w.opt.vert){d='M'+a[0]+','+a[1]+' C'+a[0]+','+(a[1]-20)+' '+b[0]+','+(b[1]+30)+' '+b[0]+','+b[1];}       // fonte → borne
    else if(w.opt.power){const yy=a[1]-w.opt.power,xx=b[0]-26-w.opt.power;   // sobe, contorna pela esquerda e entra no borne
      d='M'+a[0]+','+a[1]+' L'+a[0]+','+(yy+8)+' Q'+a[0]+','+yy+' '+(a[0]-8)+','+yy+' L'+(xx+8)+','+yy+' Q'+xx+','+yy+' '+xx+','+(yy-8)+' L'+xx+','+(b[1]+8)+' Q'+xx+','+b[1]+' '+(xx+8)+','+b[1]+' L'+b[0]+','+b[1];}
    else if(w.opt.flat){d='M'+(a[0])+','+(a[1])+' L'+b[0]+','+b[1];}
    else if(w.opt.bend){d='M'+a[0]+','+a[1]+' C'+a[0]+','+(a[1]-w.opt.bend)+' '+(b[0]-w.opt.bend)+','+b[1]+' '+b[0]+','+b[1];}
    else d=curve(a,b);
    wl+='<g class="wire'+(w.opt.off?' off':'')+'" data-id="'+w.id+'" data-label="'+w.label+'">'+
      '<path d="'+d+'" class="halo"/><path d="'+d+'" stroke="'+w.color+'" class="core"/>'+(w.opt.sig&&!w.opt.off?'<path d="'+d+'" class="flow"/>':'')+
      '<circle cx="'+a[0]+'" cy="'+a[1]+'" r="3.2" fill="'+w.color+'"/><circle cx="'+b[0]+'" cy="'+b[1]+'" r="3.2" fill="'+w.color+'"/></g>';
  });
  const defs='<defs><linearGradient id="pcb" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#16203A"/><stop offset="1" stop-color="#0F1628"/></linearGradient>'+
    '<linearGradient id="chip" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#3A3E4A"/><stop offset="1" stop-color="#22252E"/></linearGradient>'+
    '<linearGradient id="metal" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#2B2E37"/><stop offset="1" stop-color="#1A1C22"/></linearGradient>'+
    '<linearGradient id="tmc" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#3B2A6B"/><stop offset="1" stop-color="#271B4A"/></linearGradient></defs>';
  return '<svg viewBox="0 0 1072 620" class="wsvg">'+defs+wl+o+'</svg>';
}

function wireRows(){
  const tb=WIRE.drv==='tb6600',ena=!!(S&&S.enaUse);
  const r=tb?[
    [C.step,'STEP','D25 (GPIO 25)','PUL+'],[C.dir,'DIR','D26 (GPIO 26)','DIR+'],[C.ena,'ENA'+(ena?'':' · opcional'),'D27 (GPIO 27)','ENA+'],
    [C.gnd,'GND sinal','GND','PUL−, DIR−, ENA−'],[C.v24,'Potência','Fonte 24 V +','VCC'],[C.gnd,'GND potência','Fonte 24 V −','GND'],
    [C.mA1,'Bobina A','vermelho / verde','A+ / A−'],[C.mB1,'Bobina B','amarelo / azul','B+ / B−'],[C.v5,'ESP32','Step-down 5 V','VIN (+ GND comum)']]
   :[[C.step,'STEP','D25 (GPIO 25)','STEP'],[C.dir,'DIR','D26 (GPIO 26)','DIR'],[C.ena,'EN'+(ena?'':' · opcional'),'D27 (GPIO 27)','EN (LOW = torque)'],
    [C.v5,'Lógica','3V3','VDD'],[C.gnd,'GND lógica','GND','GND (lado lógico)'],[C.v24,'Potência','Fonte 24 V + (100 µF)','VM'],[C.gnd,'GND potência','Fonte 24 V −','GND (lado motor)'],
    [C.mA1,'Bobina A','vermelho / verde','1A / 1B'],[C.mB1,'Bobina B','amarelo / azul','2A / 2B'],[C.uart,'UART · futuro','RX2 (GPIO 16) + 1 kΩ','PDN']];
  return r.map(([c,a,b,d])=>'<div class="wrow"><i style="background:'+c+'"></i><b>'+a+'</b><span>'+b+'</span><span class="arr">→</span><span>'+d+'</span></div>').join('');
}

function renderWiring(){
  const box=$('wdiag');box.classList.add('fade');
  setTimeout(()=>{
    box.innerHTML=wiringSVG();box.classList.remove('fade');
    $('wtable').innerHTML=wireRows();
    box.querySelectorAll('.wire').forEach(g=>{
      g.addEventListener('mouseenter',e=>{box.classList.add('dim');g.classList.add('hl');const t=$('wtip');t.textContent=g.dataset.label;t.hidden=false;});
      g.addEventListener('mousemove',e=>{const r=box.getBoundingClientRect(),t=$('wtip');t.style.left=(e.clientX-r.left+14)+'px';t.style.top=(e.clientY-r.top+14)+'px';});
      g.addEventListener('mouseleave',()=>{box.classList.remove('dim');g.classList.remove('hl');$('wtip').hidden=true;});
    });
  },120);
  // painel de configuração
  const tb=WIRE.drv==='tb6600';
  document.querySelectorAll('#dseg button').forEach(b=>b.classList.toggle('active',b.dataset.d===WIRE.drv));
  $('cfg-tb').hidden=!tb;$('cfg-tmc').hidden=tb;
  $('wname').textContent=tb?'TB6600':'TMC2209';
  if(tb){
    const ms=TB_MS.find(m=>m[0]===WIRE.tbMs)||TB_MS[4],cu=TB_CUR.find(c=>c[0]===+WIRE.tbCur)||TB_CUR[2];
    $('tbMs').value=ms[0];$('tbCur').value=String(cu[0]);
    $('dipRow').innerHTML=[ms[2],ms[3],ms[4],cu[2],cu[3],cu[4]].map((v,i)=>'<div class="dsw'+(v==='ON'?' on':'')+'"><span></span><b>S'+(i+1)+'</b><em>'+v+'</em></div>').join('');
    $('tbInfo').innerHTML=ms[5]+' pulsos/volta · corrente '+cu[0].toFixed(1)+' A (pico '+cu[1].toFixed(1)+' A)'+(cu[1]>1.75?' · <span style="color:var(--accent-warning)">acima do 17HS4401 (1,7 A)</span>':'');
  }else{
    $('tmcMs').value=String(WIRE.tmcMs);$('irms').value=WIRE.irms;
    const msv=TMC_MS[WIRE.tmcMs];
    $('tmcInfo').innerHTML='MS1 → <b>'+msv[0]+'</b> · MS2 → <b>'+msv[1]+'</b><br>Vref ≈ <b>'+(WIRE.irms*1.41).toFixed(2)+' V</b> para '+(+WIRE.irms).toFixed(2)+' A RMS'+(WIRE.irms>1.2?' · <span style="color:var(--accent-warning)">use dissipador e ventilação</span>':'');
  }
  syncMsHint();
}
function drvMicro(){return WIRE.drv==='tb6600'?(TB_MS.find(m=>m[0]===WIRE.tbMs)||TB_MS[4])[1]:+WIRE.tmcMs;}
function syncMsHint(){
  if(!S)return;const want=drvMicro(),same=S.micro===want;
  const html=same?'<span class="badge ok">ESP32 também em '+(want==1?'passo cheio':'1/'+want)+'</span>'
    :'<span class="badge warn">ESP32 está em '+(S.micro==1?'passo cheio':'1/'+S.micro)+'</span><button class="btn sm" id="msApply">Usar '+(want==1?'passo cheio':'1/'+want)+' no ESP32</button>';
  if(html===msHtml)return;msHtml=html;$('msSync').innerHTML=html;
  const b=$('msApply');if(b)b.onclick=async()=>{setBtnLoading(b,true,'Aplicando…');const j=await api('/motor',{method:'POST',body:new URLSearchParams({ms:want,ratio:S.ratio})});
    if(!j.ok){setBtnLoading(b,false);toast(j.error||'Não consegui aplicar','err');return;}mPend={ms:want,ratio:+S.ratio,t:Date.now()};};
}
document.querySelectorAll('#dseg button').forEach(b=>b.onclick=()=>{WIRE.drv=b.dataset.d;saveWire();renderWiring();});
$('tbMs').onchange=()=>{WIRE.tbMs=$('tbMs').value;saveWire();renderWiring();};
$('tbCur').onchange=()=>{WIRE.tbCur=+$('tbCur').value;saveWire();renderWiring();};
$('tmcMs').onchange=()=>{WIRE.tmcMs=+$('tmcMs').value;saveWire();renderWiring();};
$('irms').onchange=()=>{const v=Math.max(0.1,Math.min(2,+$('irms').value||1));WIRE.irms=v;saveWire();renderWiring();};
let wireEna=null,msHtml='';
function wiringTick(s){
  if(wireEna!==!!s.enaUse){wireEna=!!s.enaUse;if(!$('tab-wire').hidden)renderWiring();}
  syncMsHint();
}
TB_MS.forEach(m=>{const o=document.createElement('option');o.value=m[0];o.textContent=(m[1]==1?'Passo cheio':'1/'+m[0])+' · '+m[5]+' pulsos/volta';$('tbMs').appendChild(o);});
TB_CUR.forEach(c=>{const o=document.createElement('option');o.value=String(c[0]);o.textContent=c[0].toFixed(1)+' A (pico '+c[1].toFixed(1)+' A)';$('tbCur').appendChild(o);});
