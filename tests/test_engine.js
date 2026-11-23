const NE=require('../web/sim/engine.js');
let fails=0;const ok=(c,m)=>{if(!c){fails++;console.log('FALHOU:',m);}else console.log('ok  ',m);};
function goto(e,deg,maxT=60){const tgt=e.degToSteps(deg);e.cmd('goto',deg);let over=0;const t0=e.t;const dir=Math.sign(tgt-e.pos);
  while(e.t-t0<maxT){e.service()||(e.t+=0.001);over=Math.max(over,dir*(e.pos-tgt));if(e.vel===0&&e.pos===e.target&&e.mode==='idle')break;}
  return {over,time:e.t-t0,pos:e.pos,tgt};}
// 1 trapezoidal
{const e=new NE();const r=goto(e,90);ok(r.pos===r.tgt&&r.over<=0,'trapezoidal 90° chega exato sem passar ('+r.time.toFixed(2)+' s)');}
// 2 curva S em várias combinações
for(const [rpm,acc,jerk,deg] of [[5,4000,40000,90],[30,4000,40000,360],[30,20000,40000,45],[2,1000,5000,10],[30,4000,1e6,180],[15,8000,20000,-270]]){
  const e=new NE();e.cmd('rpm',rpm);e.cmd('acc',acc);e.cmd('ramp',1,jerk);
  const t=new NE();t.cmd('rpm',rpm);t.cmd('acc',acc);const rt=goto(t,deg);
  const r=goto(e,deg);ok(r.pos===r.tgt&&r.over<=2&&r.time<rt.time*2.2,`curva S rpm${rpm} acc${acc} jerk${jerk} ${deg}°: passou ${r.over} passos, ${r.time.toFixed(2)} s (trapézio ${rt.time.toFixed(2)} s)`);}
// 3 curva S mudando o alvo no meio
{const e=new NE();e.cmd('rpm',20);e.cmd('ramp',1,40000);e.cmd('goto',300);for(let i=0;i<20000;i++)e.service();e.cmd('goto',-100);
 let n=0;while(!(e.vel===0&&e.pos===e.target)&&n<5e7){e.service()||(e.t+=0.001);n++;}ok(e.pos===e.degToSteps(-100),'curva S com troca de alvo no meio termina no alvo novo');}
function runToIdle(e,maxT){const t0=e.t;while(e.t-t0<maxT){e.service()||(e.t+=0.001);if(e.mode==='idle'&&e.vel===0&&e.pos===e.target)return e.t-t0;}return -1;}
// 4 rotinas
{const e=new NE();e.cmd('loop',90,3,100);const t=runToIdle(e,120);ok(t>0&&e.cycles===3&&e.pos===0,'loop 3 ciclos termina em 0° ('+t.toFixed(1)+' s)');}
{const e=new NE();e.cmd('sweep',90,10,30,5);const t=runToIdle(e,300);ok(t>0&&e.pos===0&&Math.abs(e.sweepRpm-35)<0.01,'varredura rpm 10→30 em 5 degraus termina ('+t.toFixed(1)+' s, último '+e.sweepRpm.toFixed(1)+')');}
{const e=new NE();e.cmd('accsweep',45,1000,9000,5);const t=runToIdle(e,300);ok(t>0&&e.pos===0,'varredura de aceleração termina em 0° ('+t.toFixed(1)+' s)');}
{const e=new NE();e.cmd('backlash',10,500);const seen=new Set();const t0=e.t;while(e.t-t0<60){e.service()||(e.t+=0.001);seen.add(e.subMode());if(e.mode==='idle')break;}ok(e.mode==='idle'&&e.pos===0&&seen.has('hold+')&&seen.has('hold-'),'folga passa por hold+ e hold- e termina em 0°');}
{const e=new NE();e.cmd('drift',90,5,50);const t=runToIdle(e,120);ok(t>0&&e.pos===0&&e.cycles===5,'repetibilidade 5 ciclos termina em 0°');}
{const e=new NE();e.cmd('goto',30);runToIdle(e,30);const base=e.pos;e.cmd('stiction',0.5,4,200);let maxk=0;const t0=e.t;while(e.t-t0<60){e.service()||(e.t+=0.001);maxk=Math.max(maxk,e.pos-base);if(e.mode==='idle'&&e.vel===0&&e.pos===e.target)break;}
 ok(e.pos===base&&maxk===e.degToSteps(2),'atrito estático sobe 4×0,5° e volta à posição inicial');}
{const e=new NE();e.wp=[{deg:45,rpm:10,dwell:100},{deg:-30,rpm:3,dwell:100},{deg:0,rpm:20,dwell:0}];e.wpLoop=false;e.cmd('waypoints');const t=runToIdle(e,120);ok(t>0&&e.pos===0,'trajetória de 3 pontos termina no último ponto');}
{const e=new NE();e.cmd('resonance',2,10,3,1);const t=runToIdle(e,30);ok(t>3&&t<6&&e.mode==='idle','ressonância roda ~3 s e para ('+t.toFixed(2)+' s)');}
// 5 jog com watchdog
{const e=new NE();e.cmd('rpm',10);let t0=e.t;let lastRenew=-1;while(e.t-t0<1){if(e.t-lastRenew>=0.15){e.cmd('jog',1,1);lastRenew=e.t;}e.service()||(e.t+=0.001);}
 const relT=e.t;while(e.mode==='jog'){e.service()||(e.t+=0.001);}const stopAt=e.t;runToIdle(e,10);ok(stopAt-relT>0.3&&stopAt-relT<0.47,'jog para sozinho ~450 ms depois da última renovação ('+((stopAt-relT)*1000).toFixed(0)+' ms)');}
// 6 batentes
{const e=new NE();e.cmd('limits',1,-30,60,0);const r=goto(e,200);ok(e.pos===e.degToSteps(60),'goto 200° com batente em 60° para em 60°');
 e.cmd('rpm',20);let t0=e.t,lr=-1;while(e.t-t0<15){if(e.t-lr>=0.15){e.cmd('jog',-1,1);lr=e.t;}e.service()||(e.t+=0.001);}ok(e.pos===e.degToSteps(-30),'jog com batente para em −30°');
 e.cmd('resonance',2,10,3,1);ok(e.mode!=='resonance','ressonância recusada com batentes ligados');}
// 7 ENA
{const e=new NE();e.cmd('limits',0,-120,120,1|2);ok(e.enaUse&&e.enaOn,'ENA ligado e com torque');e.cmd('goto',90);for(let i=0;i<3000;i++)e.service();e.cmd('ena',0);ok(e.enaOn,'ao pedir soltar em movimento, só solta depois de parar');
 runToIdle(e,30);e.service();ok(!e.enaOn,'soltou o torque depois de parar');e.cmd('goto',0);ok(e.enaOn,'comando de movimento religa o torque');}
console.log(fails?fails+' falhas':'tudo certo');
