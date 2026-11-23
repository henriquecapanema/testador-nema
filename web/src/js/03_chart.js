// ---------- Gráfico ----------
const C_POS='#93A5FF',C_TGT='#FCD34D',C_VEL='#67E8F9',C_MARK='#FB7185',INK='#9898A8',MUT='#7A7A8C',GRID='#1C1C26',ZERO='#323244',SURF='#111115';
const G={t:[],p:[],g:[],v:[]};
Object.defineProperty(G,'marks',{value:[],enumerable:false});   // falhas marcadas: {t, label}
Object.defineProperty(G,'steps',{value:[],enumerable:false});   // trocas de degrau das varreduras: {t, label}
let seq=0,win=10,paused=false,tFrozen=0,tPause=0,hoverT=null;
const PAD={l:44,r:62,t:8,b:20};
const GAP=0.15;                  // intervalo entre amostras acima disso = buraco (s)
function addSamples(a){
  for(const [t,p,g,v] of a){
    const n=G.t.length;
    if(n&&t/1000<=G.t[n-1]){
      if(t/1000<G.t[n-1]-1){for(const k in G)G[k].length=0;G.marks.length=0;G.steps.length=0;}   // ESP32 reiniciou
      else continue;                                                                             // repetida (troca de conexão)
    }
    G.t.push(t/1000);G.p.push(p);G.g.push(g);G.v.push(v);
  }
  const n=G.t.length;
  const keep=(paused||(typeof testRunning!=='undefined'&&testRunning))?300:60;   // pausado ou em teste: até 5 min
  if(n&&G.t[0]<G.t[n-1]-keep-2){let i=0;while(G.t[i]<G.t[n-1]-keep)i++;for(const k in G)G[k].splice(0,i);}
}
function lb(a,x){let l=0,h=a.length;while(l<h){const m=(l+h)>>1;if(a[m]<x)l=m+1;else h=m;}return l;}
function ticks(lo,hi,n){
  const s0=(hi-lo)/n,m=10**Math.floor(Math.log10(s0)),f=s0/m,s=(f<1.5?1:f<3?2:f<7?5:10)*m,o=[];
  for(let v=Math.ceil(lo/s)*s;v<=hi+s*1e-6;v+=s)o.push(Math.abs(v)<s*1e-6?0:v);return o;
}
function fmt(v,step){return step>=1?v.toFixed(0):v.toFixed(step>=0.1?1:2);}
function view(){
  const tEnd=paused?tFrozen:(G.t.length?G.t[G.t.length-1]:0);
  return {t0:tEnd-win,tEnd,i0:Math.max(0,lb(G.t,tEnd-win)-1),i1:Math.min(G.t.length,lb(G.t,tEnd+1e-9)+1)};
}
const yz={cp:null,cv:null};       // zoom vertical de cada gráfico (null = automático)
let sel=null;                     // retângulo sendo arrastado
const lastT=()=>G.t.length?G.t[G.t.length-1]:0;
const refT=()=>paused?tPause:lastT();   // "agora" do eixo de tempo (fixo enquanto pausado)
function plot(cv,series,incl0,labels){
  const r=cv.getBoundingClientRect(),d=window.devicePixelRatio||1,W=r.width,H=r.height;
  if(cv.width!==Math.round(W*d)||cv.height!==Math.round(H*d)){cv.width=Math.round(W*d);cv.height=Math.round(H*d);}
  const c=cv.getContext('2d');c.setTransform(d,0,0,d,0,0);c.clearRect(0,0,W,H);
  const V=view();let lo=Infinity,hi=-Infinity;
  const GA=ghostArr(cv.id);let gi0=0,gi1=0;          // execução anterior (cinza)
  if(GA){gi0=Math.max(0,lb(GA.t,V.t0-GA.base)-1);gi1=Math.min(GA.t.length,lb(GA.t,V.tEnd-GA.base)+1);}
  if(yz[cv.id]){lo=yz[cv.id].lo;hi=yz[cv.id].hi;}
  else{
    for(const s of series)for(let i=V.i0;i<V.i1;i++){const y=s.a[i];if(y<lo)lo=y;if(y>hi)hi=y;}
    if(GA)for(let i=gi0;i<gi1;i++){const y=GA.a[i];if(y<lo)lo=y;if(y>hi)hi=y;}
    if(!isFinite(lo)){lo=-1;hi=1;}
    if(incl0){lo=Math.min(lo,0);hi=Math.max(hi,0);}
    if(hi-lo<2){const m=(hi+lo)/2;lo=m-1;hi=m+1;}
    const pd=(hi-lo)*0.08;lo-=pd;hi+=pd;
  }
  const pw=W-PAD.l-PAD.r,ph=H-PAD.t-PAD.b;
  const X=t=>PAD.l+(t-V.t0)/win*pw,Y=v=>PAD.t+(hi-v)/(hi-lo)*ph;
  c.font='11px system-ui,sans-serif';c.lineWidth=1;
  // grade horizontal + números do eixo Y (mais linhas quanto mais alto o gráfico)
  const yt=ticks(lo,hi,Math.max(3,Math.round(ph/45))),ys=yt.length>1?yt[1]-yt[0]:1;
  c.textAlign='right';c.textBaseline='middle';
  for(const v of yt){const y=Math.round(Y(v))+.5;if(y<PAD.t-1||y>PAD.t+ph+1)continue;c.strokeStyle=v===0?ZERO:GRID;c.beginPath();c.moveTo(PAD.l,y);c.lineTo(W-PAD.r,y);c.stroke();c.fillStyle=MUT;c.fillText(fmt(v,ys),PAD.l-6,y);}
  // grade vertical + tempo (relativo à amostra mais recente)
  const tL=refT(),xt=ticks(V.t0-tL,V.tEnd-tL,Math.max(3,Math.round(pw/110))),xs=xt.length>1?xt[1]-xt[0]:1;
  c.textAlign='center';c.textBaseline='top';
  for(const s of xt){const x=X(tL+s);if(x<PAD.l-1||x>W-PAD.r+1)continue;
    c.strokeStyle=GRID;c.beginPath();c.moveTo(Math.round(x)+.5,PAD.t);c.lineTo(Math.round(x)+.5,PAD.t+ph);c.stroke();
    c.fillStyle=MUT;c.fillText(Math.abs(s)<xs*1e-3?'agora':fmt(s,xs)+' s',x,H-PAD.b+5);}
  c.save();c.beginPath();c.rect(PAD.l,PAD.t-2,pw,ph+4);c.clip();
  for(let i=Math.max(V.i0,1);i<V.i1;i++){       // faixas sem dados
    if(G.t[i]-G.t[i-1]>GAP){const xa=X(G.t[i-1]),xb=X(G.t[i]);
      c.fillStyle='rgba(152,152,168,.07)';c.fillRect(xa,PAD.t,xb-xa,ph);
      if(labels&&xb-xa>50){c.fillStyle=MUT;c.textAlign='center';c.textBaseline='middle';c.fillText('sem dados',(xa+xb)/2,PAD.t+ph/2);}}
  }
  if(GA&&gi1>gi0){
    c.strokeStyle='rgba(152,152,168,.38)';c.lineWidth=2.5;c.lineJoin='round';c.setLineDash([]);c.beginPath();let gp=null;
    const gst=Math.max(1,Math.floor((gi1-gi0)/(pw*2)));
    for(let i=gi0;i<gi1;i+=gst){const x=X(GA.base+GA.t[i]),y=Y(GA.a[i]);(gp===null||GA.t[i]-gp>0.5)?c.moveTo(x,y):c.lineTo(x,y);gp=GA.t[i];}
    c.stroke();
  }
  if(GA)for(const m of GA.marks){             // falhas da execução anterior
    const x=X(GA.base+m.t);if(x<PAD.l-1||x>W-PAD.r+1)continue;
    c.strokeStyle='rgba(251,113,133,.45)';c.lineWidth=1.5;c.setLineDash([2,4]);c.beginPath();c.moveTo(Math.round(x)+.5,PAD.t);c.lineTo(Math.round(x)+.5,PAD.t+ph);c.stroke();c.setLineDash([]);
    if(labels){c.font='600 10.5px system-ui,sans-serif';c.fillStyle='rgba(251,113,133,.7)';c.textAlign='center';c.textBaseline='top';c.fillText(m.label+' ant.',x,PAD.t+20);c.font='11px system-ui,sans-serif';}
  }
  const step=Math.max(1,Math.floor((V.i1-V.i0)/(pw*2)));
  for(const s of series){
    c.strokeStyle=s.color;c.lineWidth=2;c.lineJoin='round';c.setLineDash(s.dash||[]);c.beginPath();
    let pt=null;
    const pnt=i=>{const x=X(G.t[i]),y=Y(s.a[i]);(pt===null||G.t[i]-pt>GAP)?c.moveTo(x,y):c.lineTo(x,y);pt=G.t[i];};
    for(let i=V.i0;i<V.i1;i+=step)pnt(i);
    if(V.i1>V.i0&&(V.i1-1-V.i0)%step)pnt(V.i1-1);
    c.stroke();
  }
  {const rowEnd=[-1e9,-1e9];                   // trocas de degrau: linha pontilhada; rótulo em 2 linhas, some se não couber
  c.font='600 10.5px system-ui,sans-serif';
  for(const m of G.steps){
    const x=X(m.t);if(x<PAD.l-1||x>W-PAD.r+1)continue;
    let row=-1;const tw=labels?c.measureText(m.label).width:0;
    if(labels){if(x+2>rowEnd[0])row=0;else if(x+2>rowEnd[1])row=1;}
    c.strokeStyle='rgba(152,152,168,.45)';c.lineWidth=1;c.setLineDash([1.5,3]);c.beginPath();c.moveTo(Math.round(x)+.5,PAD.t+(row>=0?16+row*14:0));c.lineTo(Math.round(x)+.5,PAD.t+ph);c.stroke();c.setLineDash([]);
    if(row>=0){c.fillStyle='#9898A8';c.textAlign='left';c.textBaseline='top';c.fillText(m.label,x+4,PAD.t+3+row*14);rowEnd[row]=x+4+tw+8;}
  }
  c.font='11px system-ui,sans-serif';}
  for(const m of G.marks){                   // falhas marcadas: linha tracejada coral
    const x=X(m.t);if(x<PAD.l-1||x>W-PAD.r+1)continue;
    c.strokeStyle=C_MARK;c.lineWidth=1.5;c.setLineDash([4,3]);c.beginPath();c.moveTo(Math.round(x)+.5,PAD.t);c.lineTo(Math.round(x)+.5,PAD.t+ph);c.stroke();c.setLineDash([]);
    if(labels){c.font='700 11px system-ui,sans-serif';const w=c.measureText(m.label).width+10;c.fillStyle=C_MARK;c.beginPath();c.roundRect(x-w/2,PAD.t,w,16,4);c.fill();
      c.fillStyle='#18080B';c.textAlign='center';c.textBaseline='middle';c.fillText(m.label,x,PAD.t+8.5);c.font='11px system-ui,sans-serif';}
  }
  c.setLineDash([]);
  let hi_=null;
  if(hoverT!==null&&G.t.length){
    let k=lb(G.t,hoverT);if(k>=G.t.length)k=G.t.length-1;if(k>0&&hoverT-G.t[k-1]<G.t[k]-hoverT)k--;
    const x=X(G.t[k]);c.strokeStyle=MUT;c.lineWidth=1;c.beginPath();c.moveTo(Math.round(x)+.5,PAD.t);c.lineTo(Math.round(x)+.5,PAD.t+ph);c.stroke();
    for(const s of series){c.beginPath();c.arc(x,Y(s.a[k]),5,0,7);c.fillStyle=s.color;c.fill();c.lineWidth=2;c.strokeStyle=SURF;c.stroke();}
    hi_=k;
  }
  if(sel&&sel.cv===cv){                     // retângulo de zoom
    const dx=Math.abs(sel.x1-sel.x0)>8,dy=Math.abs(sel.y1-sel.y0)>8;
    const xa=dx?Math.min(sel.x0,sel.x1):PAD.l,xb=dx?Math.max(sel.x0,sel.x1):W-PAD.r;
    const ya=dy?Math.min(sel.y0,sel.y1):PAD.t,yb=dy?Math.max(sel.y0,sel.y1):PAD.t+ph;
    if(dx||dy){c.fillStyle='rgba(147,165,255,.12)';c.fillRect(xa,ya,xb-xa,yb-ya);c.strokeStyle='rgba(147,165,255,.7)';c.lineWidth=1;c.strokeRect(xa+.5,ya+.5,xb-xa-1,yb-ya-1);}
  }
  c.restore();
  if(paused&&labels){                       // aviso de pausado
    const t='❚❚ PAUSADO · Continuar volta ao tempo real';c.font='600 12px system-ui,sans-serif';
    const w=c.measureText(t).width+16;c.fillStyle='rgba(252,211,77,.10)';c.strokeStyle='rgba(252,211,77,.45)';c.lineWidth=1;
    c.beginPath();c.roundRect(PAD.l+8,PAD.t+ph-30,w,22,6);c.fill();c.stroke();
    c.fillStyle='#FCD34D';c.textAlign='left';c.textBaseline='middle';c.fillText(t,PAD.l+16,PAD.t+ph-19);c.font='11px system-ui,sans-serif';
  }
  if(labels&&V.i1>V.i0){                     // rótulos diretos na ponta direita
    const k=V.i1-1;let ls=series.map(s=>({s,y:Y(s.a[k])}));
    if(ls.length===2&&Math.abs(ls[0].y-ls[1].y)<14){const m=(ls[0].y+ls[1].y)/2,up=ls[0].y<=ls[1].y?0:1;ls[up].y=m-7;ls[1-up].y=m+7;}
    c.textAlign='left';c.textBaseline='middle';
    for(const l of ls){const y=Math.max(PAD.t+5,Math.min(PAD.t+ph-5,l.y));c.strokeStyle=l.s.color;c.lineWidth=2;c.setLineDash(l.s.dash||[]);c.beginPath();c.moveTo(W-PAD.r+4,y);c.lineTo(W-PAD.r+12,y);c.stroke();c.setLineDash([]);c.fillStyle=INK;c.fillText(l.s.name,W-PAD.r+15,y);}
  }
  cv._x2t=x=>V.t0+(x-PAD.l)/pw*win;cv._ok=x=>x>=PAD.l&&x<=W-PAD.r;
  cv._y2v=y=>hi-(y-PAD.t)/ph*(hi-lo);cv._lo=lo;cv._hi=hi;cv._pw=pw;cv._W=W;
  return hi_;
}
function draw(){
  if($('tab-main').hidden)return;
  const k=plot($('cp'),[{a:G.p,color:C_POS,name:'posição'},{a:G.g,color:C_TGT,dash:[6,4],name:'alvo'}],false,true);
  plot($('cv'),[{a:G.v,color:C_VEL,name:'rpm'}],true,false);
  const tip=$('tip');
  if(k===null||k===undefined){tip.style.display='none';return;}
  tip.innerHTML=(G.t[k]-refT()).toFixed(2)+' s<br><i style="border-top:2px solid '+C_POS+'"></i>posição '+G.p[k].toFixed(2)+'°<br><i style="border-top:2px dashed '+C_TGT+'"></i>alvo '+G.g[k].toFixed(2)+'°<br><i style="border-top:2px solid '+C_VEL+'"></i>velocidade '+G.v[k].toFixed(2)+' rpm';
  tip.style.display='block';
}
function hover(e){
  const cv=e.currentTarget,r=cv.getBoundingClientRect(),x=e.clientX-r.left;
  if(!cv._ok||!cv._ok(x)){hoverT=null;draw();return;}
  hoverT=cv._x2t(x);draw();
  const tip=$('tip'),cr=$('chart').getBoundingClientRect();
  let left=e.clientX-cr.left+14;if(left+tip.offsetWidth>cr.width-8)left=e.clientX-cr.left-tip.offsetWidth-14;
  tip.style.left=left+'px';tip.style.top=(r.top-cr.top+8)+'px';
}
// ---------- Zoom ----------
function setWin(w){
  win=Math.max(0.2,Math.min(300,w));
  const o=$('win');let cu=o.querySelector('option[value="c"]');
  const m=[...o.options].find(x=>x.value!=='c'&&Math.abs(+x.value-win)<1e-6);
  if(m){o.value=m.value;if(cu)cu.remove();}
  else{if(!cu){cu=document.createElement('option');cu.value='c';o.appendChild(cu);}cu.textContent=(win<10?win.toFixed(win<1?2:1):win.toFixed(0))+' s';o.value='c';}
}
function freeze(){if(!paused){paused=true;tFrozen=tPause=lastT();$('pause').textContent='Continuar';}}
function clampPan(){const t0=G.t.length?G.t[0]:0;tFrozen=Math.min(lastT(),Math.max(t0+Math.min(win,lastT()-t0),tFrozen));}
function zoomUI(){$('reset').style.display='';}
function resumeLive(){if(paused){paused=false;$('pause').textContent='Pausar';zoomUI();}}
function resetZoom(){fitView();}
// Ajustar: enquadra o último teste inteiro (ou todo o histórico), sem sair da pausa
function fitView(r){
  if(!G.t.length){draw();return;}
  const t0buf=G.t[0],t1buf=G.t[G.t.length-1];
  let a=t0buf,b=t1buf;
  r=r||(typeof curRun!=='undefined'&&curRun)||(typeof RUNS!=='undefined'&&RUNS[RUNS.length-1])||null;
  if(r&&r.t.length){const ra=r.t0,rb=r.t0+r.t[r.t.length-1];if(rb>t0buf&&ra<t1buf){a=Math.max(t0buf,ra-0.6);b=Math.min(t1buf,rb+0.6);}}
  yz.cp=yz.cv=null;paused=true;tPause=t1buf;tFrozen=b;setWin(Math.max(0.5,b-a));
  $('pause').textContent='Continuar';zoomUI();draw();
}
function wheel(e){
  e.preventDefault();
  const cv=e.currentTarget,r=cv.getBoundingClientRect(),x=e.clientX-r.left,y=e.clientY-r.top;
  if(e.shiftKey||x<PAD.l){                   // altura (no Mac, shift+rodinha vira deltaX)
    const dd=e.deltaY||e.deltaX,f=Math.exp(dd*0.002),v=cv._y2v(y);
    yz[cv.id]={lo:v-(v-cv._lo)*f,hi:v+(cv._hi-v)*f};
  }else if(!e.ctrlKey&&Math.abs(e.deltaX)>Math.abs(e.deltaY)){   // dois dedos pro lado: anda no tempo
    freeze();tFrozen+=e.deltaX/cv._pw*win;clampPan();
  }else{                                     // rodinha ou pinça: tempo
    const f=Math.exp(e.deltaY*(e.ctrlKey?0.01:0.002)),old=win;
    if(paused){const tc=cv._x2t(Math.max(PAD.l,Math.min(cv._W-PAD.r,x)));setWin(win*f);tFrozen=tc+(tFrozen-tc)*win/old;clampPan();}
    else setWin(win*f);
  }
  zoomUI();draw();
}
function down(e){
  if(e.button!==0)return;
  const cv=e.currentTarget,r=cv.getBoundingClientRect(),x=e.clientX-r.left,y=e.clientY-r.top;
  if(!cv._ok(x))return;
  sel={cv,x0:x,y0:y,x1:x,y1:y};cv.setPointerCapture(e.pointerId);
}
function move(e){
  const cv=e.currentTarget;
  if(sel&&sel.cv===cv){const r=cv.getBoundingClientRect();sel.x1=Math.max(PAD.l,Math.min(r.width-PAD.r,e.clientX-r.left));sel.y1=Math.max(PAD.t,Math.min(r.height-PAD.b,e.clientY-r.top));hoverT=null;$('tip').style.display='none';draw();return;}
  hover(e);
}
function up(e){
  if(!sel||sel.cv!==e.currentTarget)return;
  const s=sel,cv=s.cv;sel=null;
  if(Math.abs(s.x1-s.x0)>8){const a=cv._x2t(Math.min(s.x0,s.x1)),b=cv._x2t(Math.max(s.x0,s.x1));freeze();setWin(b-a);tFrozen=b;}
  if(Math.abs(s.y1-s.y0)>8)yz[cv.id]={lo:cv._y2v(Math.max(s.y0,s.y1)),hi:cv._y2v(Math.min(s.y0,s.y1))};
  zoomUI();draw();
}
for(const id of ['cp','cv']){
  const el=$(id);
  el.addEventListener('pointerdown',down);el.addEventListener('pointermove',move);el.addEventListener('pointerup',up);
  el.addEventListener('pointercancel',()=>{sel=null;draw();});
  el.addEventListener('pointerleave',()=>{if(!sel){hoverT=null;draw();}});
  el.addEventListener('wheel',wheel,{passive:false});
  el.addEventListener('dblclick',resetZoom);
}
$('win').onchange=()=>{if($('win').value!=='c'){setWin(+$('win').value);if(paused){clampPan();}zoomUI();draw();}};
$('pause').onclick=()=>{paused=!paused;tFrozen=tPause=lastT();$('pause').textContent=paused?'Continuar':'Pausar';zoomUI();draw();};
$('reset').onclick=resetZoom;
$('clear').onclick=()=>{for(const k in G)G[k].length=0;G.marks.length=0;G.steps.length=0;draw();};
$('csv').onclick=()=>{
  let s='tempo_s,posicao_graus,alvo_graus,velocidade_rpm\n';
  for(let i=0;i<G.t.length;i++)s+=G.t[i].toFixed(3)+','+G.p[i]+','+G.g[i]+','+G.v[i]+'\n';
  const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([s],{type:'text/csv'}));a.download='nemabot_movimento.csv';a.click();
};
window.addEventListener('resize',draw);
