// Motor de movimento do nemabot: espelho em JS do motionService() do firmware.
// Usado no simulador da prévia e nos testes. Mesmos nomes e regras do C++.
function NemaEngine(){
  const E=this;
  // ---- configuração (NVS no firmware) ----
  E.MICRO=8;E.RATIO=20;const MOTOR_RPM_MAX=600,V_START=200,BIG=20000000;
  E.applyCfg=()=>{E.STEPS_OUT_REV=200*E.MICRO*E.RATIO;E.RPM_MAX=MOTOR_RPM_MAX/E.RATIO;if(E.rpm>E.RPM_MAX)E.rpm=E.RPM_MAX;};
  // ---- estado ----
  E.pos=0;E.target=0;E.vel=0;E.rpm=5;E.acc=4000;E.rampMode=0;E.jerk=40000;E.curAcc=0;
  E.mode='idle';E.cycles=0;
  let dwelling=false,dwellUntil=0,seqPhase=0,seqBase=0,seqAmp=0,seqCycles=0,seqDwell=300;
  E.sweepRpm=0;let sweepStep=0,sweepMax=0;E.sweepAcc=0;let accStep=0,accMax=0;
  let resStart=0,resDur=30,resMin=0,resMax=0,resDir=1;E.resRpmNow=0;
  let holdMs=3000,stStep=0.25,stCount=10;E.stK=0;
  E.wp=[];E.wpLoop=false;E.wpIdx=0;let wpRpmNow=0;
  let jogDir=1,jogScale=1,jogLast=0;
  E.limEn=false;E.limMin=-120;E.limMax=120;
  E.enaUse=false;E.enaInv=true;E.enaOn=true;let pendEnaOff=false;
  let pendCfg=null;
  E.t=0;const millis=()=>Math.floor(E.t*1000);
  E.logs=[];const log=m=>E.logs.push(m);
  E.applyCfg();

  const sign=x=>(x>0)-(x<0);
  E.degToSteps=d=>Math.round(d/360*E.STEPS_OUT_REV);
  E.stepsToDeg=s=>s*360/E.STEPS_OUT_REV;
  function clampSteps(s){if(!E.limEn)return s;const lo=E.degToSteps(E.limMin),hi=E.degToSteps(E.limMax);return Math.max(lo,Math.min(hi,s));}
  function setT(s){E.target=clampSteps(s);}
  function currentVmax(){
    let r=E.rpm;
    if(E.mode==='sweep')r=E.sweepRpm;
    else if(E.mode==='waypoints'&&wpRpmNow>0)r=wpRpmNow;
    else if(E.mode==='resonance')r=E.resRpmNow;
    else if(E.mode==='jog')r=E.rpm*jogScale;
    r=Math.min(r,E.RPM_MAX);
    return r*E.STEPS_OUT_REV/60;
  }
  const accNow=()=>E.mode==='accsweep'?E.sweepAcc:E.acc;
  function stopDistance(){const sv=Math.abs(E.vel),v0=Math.min(V_START,currentVmax()),s=(sv*sv-v0*v0)/(2*accNow());return s>0?Math.ceil(s):0;}
  function stopRamp(){E.mode='idle';dwelling=false;const d=stopDistance();E.target=E.pos+(E.vel>0?d:E.vel<0?-d:0);}
  function setEna(on){if(!E.enaUse)return;E.enaOn=on;}
  function ensureEna(){pendEnaOff=false;if(E.enaUse&&!E.enaOn)setEna(true);}
  function startSequence(m,ampDeg){E.mode=m;seqBase=E.target;seqAmp=E.degToSteps(ampDeg);seqPhase=0;dwelling=false;E.cycles=0;setT(seqBase+seqAmp);}
  function finish(msg){E.mode='idle';dwelling=false;if(msg)log(msg);}

  E.cmd=function(c,p1,p2,p3,p4){
    p1=+p1||0;p2=+p2||0;p3=+p3||0;p4=+p4||0;
    switch(c){
      case 'goto':ensureEna();E.mode='idle';dwelling=false;setT(E.degToSteps(Math.max(-3600,Math.min(3600,p1))));break;
      case 'move':ensureEna();E.mode='idle';dwelling=false;setT(E.target+E.degToSteps(p1));break;
      case 'rpm':if(p1>0)E.rpm=Math.min(p1,E.RPM_MAX);break;
      case 'acc':if(p1>=100)E.acc=Math.min(p1,50000);break;
      case 'ramp':E.rampMode=p1?1:0;if(p2>0)E.jerk=Math.max(1000,Math.min(1e7,p2));break;
      case 'loop':ensureEna();seqCycles=p2>0?Math.round(p2):0;seqDwell=p3>0?p3:300;startSequence('loop',p1||90);
        log('Loop de '+(p1||90)+'° · '+(seqCycles?seqCycles+' ciclos':'sem fim'));break;
      case 'sweep':{ensureEna();let lo=p2>0?p2:E.RPM_MAX/15,hi=p3>0?Math.min(p3,E.RPM_MAX):E.RPM_MAX;if(lo>hi)[lo,hi]=[hi,lo];
        const n=p4>=2?Math.round(p4):15;E.sweepRpm=lo;sweepMax=hi;sweepStep=(hi-lo)/(n-1);startSequence('sweep',p1||90);
        log('Varredura de rpm: '+lo.toFixed(1)+' → '+hi.toFixed(1)+' em '+n+' degraus');break;}
      case 'accsweep':{ensureEna();let lo=p2>0?p2:1000,hi=p3>0?Math.min(p3,50000):20000;if(lo>hi)[lo,hi]=[hi,lo];
        const n=p4>=2?Math.round(p4):10;E.sweepAcc=lo;accMax=hi;accStep=(hi-lo)/(n-1);startSequence('accsweep',p1||90);
        log('Varredura de aceleração: '+lo+' → '+hi+' p/s²');break;}
      case 'resonance':if(E.limEn){log('Ressonância recusada: batentes ligados');break;}
        ensureEna();resMin=p1>0?Math.min(p1,E.RPM_MAX):E.RPM_MAX/15;resMax=p2>0?Math.min(p2,E.RPM_MAX):E.RPM_MAX;resDur=p3>0?p3:30;resDir=p4<0?-1:1;
        resStart=millis();E.resRpmNow=resMin;E.mode='resonance';dwelling=false;E.target=E.pos+resDir*BIG;
        log('Ressonância: '+resMin.toFixed(1)+' → '+resMax.toFixed(1)+' rpm em '+resDur+' s');break;
      case 'backlash':ensureEna();holdMs=p2>0?p2:3000;startSequence('backlash',p1||10);log('Folga: ±'+(p1||10)+'°, segura '+holdMs+' ms');break;
      case 'drift':ensureEna();seqCycles=p2>0?Math.round(p2):20;seqDwell=p3>0?p3:100;startSequence('drift',p1||90);log('Repetibilidade: '+seqCycles+' ciclos de '+(p1||90)+'°');break;
      case 'stiction':ensureEna();stStep=p1||0.25;stCount=p2>0?Math.round(p2):10;seqDwell=p3>0?p3:1000;
        E.mode='stiction';seqBase=E.target;seqPhase=0;E.stK=1;dwelling=false;E.cycles=0;setT(seqBase+E.degToSteps(stStep));
        log('Atrito estático: '+stCount+' degraus de '+stStep+'°');break;
      case 'waypoints':if(!E.wp.length)break;ensureEna();E.mode='waypoints';E.wpIdx=0;E.cycles=0;dwelling=false;wpRpmNow=E.wp[0].rpm;setT(E.degToSteps(E.wp[0].deg));
        log('Trajetória: '+E.wp.length+' pontos'+(E.wpLoop?' em loop':''));break;
      case 'jog':ensureEna();jogDir=p1<0?-1:1;jogScale=p2>0?Math.max(0.05,Math.min(1,p2)):1;jogLast=millis();E.mode='jog';dwelling=false;setT(E.pos+jogDir*BIG);break;
      case 'stop':stopRamp();break;
      case 'zero':{const p=E.pos;E.pos-=p;E.target-=p;seqBase-=p;log('Posição zerada.');break;}
      case 'ena':if(!E.enaUse)break;if(p1){setEna(true);pendEnaOff=false;}else{stopRamp();pendEnaOff=true;}break;
      case 'limits':E.limEn=!!p1;E.limMin=p2;E.limMax=p3;{const use=!!(p4&1);E.enaInv=!!(p4&2);if(use&&!E.enaUse){E.enaUse=true;setEna(true);}else if(!use&&E.enaUse){E.enaOn=true;E.enaUse=false;}}
        if(E.limEn&&E.mode==='resonance')stopRamp();break;
      case 'cfg':pendCfg={ms:p1,ratio:p2};E.mode='idle';dwelling=false;{const d=stopDistance();E.target=E.pos+(E.vel>0?d:E.vel<0?-d:0);}break;
    }
  };

  function curDwell(){
    switch(E.mode){
      case 'loop':case 'drift':return seqDwell;
      case 'sweep':case 'accsweep':return seqPhase===0?300:700;
      case 'backlash':return (seqPhase===1||seqPhase===3)?holdMs:300;
      case 'stiction':return seqPhase===0?seqDwell:0;
      case 'waypoints':return E.wp[E.wpIdx]?E.wp[E.wpIdx].dwell:0;
    }
    return 0;
  }
  function seqStep(){
    switch(E.mode){
      case 'loop':
        if(seqPhase===0){seqPhase=1;setT(seqBase);return;}
        E.cycles++;if(seqCycles>0&&E.cycles>=seqCycles){finish('Loop: '+E.cycles+' ciclos concluídos.');return;}
        seqPhase=0;setT(seqBase+seqAmp);return;
      case 'sweep':
        if(seqPhase===0){seqPhase=1;setT(seqBase);return;}
        E.cycles++;E.sweepRpm+=sweepStep;
        if(sweepStep<=0||E.sweepRpm>sweepMax*1.0001){finish('Varredura de rpm encerrada.');return;}
        log('  '+E.sweepRpm.toFixed(1)+' rpm');seqPhase=0;setT(seqBase+seqAmp);return;
      case 'accsweep':
        if(seqPhase===0){seqPhase=1;setT(seqBase);return;}
        E.cycles++;E.sweepAcc+=accStep;
        if(accStep<=0||E.sweepAcc>accMax*1.0001){finish('Varredura de aceleração encerrada.');return;}
        log('  '+E.sweepAcc.toFixed(0)+' p/s²');seqPhase=0;setT(seqBase+seqAmp);return;
      case 'backlash':
        if(seqPhase===0){seqPhase=1;setT(seqBase);return;}
        if(seqPhase===1){seqPhase=2;setT(seqBase-seqAmp);return;}
        if(seqPhase===2){seqPhase=3;setT(seqBase);return;}
        finish('Folga: medição concluída (segurou em 0° vindo de + e de −).');return;
      case 'drift':
        if(seqPhase===0){seqPhase=1;setT(seqBase);return;}
        E.cycles++;if(E.cycles>=seqCycles){finish('Repetibilidade: '+E.cycles+' ciclos, parado em 0°. Confira a marca física.');return;}
        seqPhase=0;setT(seqBase+seqAmp);return;
      case 'stiction':
        if(seqPhase===1){finish('Atrito estático: voltou à posição inicial.');return;}
        if(E.stK<stCount){E.stK++;setT(seqBase+E.degToSteps(stStep*E.stK));return;}
        seqPhase=1;setT(seqBase);return;
      case 'waypoints':
        E.wpIdx++;
        if(E.wpIdx>=E.wp.length){if(!E.wpLoop){finish('Trajetória concluída.');return;}E.wpIdx=0;E.cycles++;}
        wpRpmNow=E.wp[E.wpIdx].rpm;setT(E.degToSteps(E.wp[E.wpIdx].deg));return;
    }
  }
  function onArrived(){
    if(E.mode==='idle'||E.mode==='jog'||E.mode==='resonance')return;
    if(!dwelling){dwelling=true;dwellUntil=millis()+curDwell();return;}
    if(millis()<dwellUntil)return;
    dwelling=false;seqStep();
  }

  function updateVelocity(){
    const d=E.target-E.pos,sd=sign(d),ss=sign(E.vel),sv=Math.abs(E.vel),vmax=currentVmax(),v0=Math.min(V_START,vmax),a=accNow();
    if(ss===0){E.vel=sd*v0;E.curAcc=a*0.1;return;}
    if(E.rampMode===0){                                   // trapezoidal (igual ao de antes)
      const stopSteps=(sv*sv-v0*v0)/(2*a);
      const brake=(sd!==ss)||(Math.abs(d)<=stopSteps)||(sv>vmax*1.001);
      if(brake){const s2=sv*sv-2*a;if(s2<=v0*v0)E.vel=(sd===ss)?ss*v0:sd*v0;else E.vel=ss*Math.sqrt(s2);}
      else if(sv<vmax)E.vel=ss*Math.min(Math.sqrt(sv*sv+2*a),vmax);
      return;
    }
    // curva S: a aceleração muda no máximo 'jerk' por segundo (Δa = jerk·dt, dt = 1/v)
    const jstep=E.jerk/Math.max(sv,v0),aMin=a*0.1;
    const aReq=(sd===ss&&d!==0)?(sv*sv-v0*v0)/(2*Math.abs(d)):Infinity;   // desaceleração que para exatamente no alvo
    const over=sv>vmax*1.001;
    const brake=(sd!==ss)||over||aReq>=a*0.5;
    if(brake){
      let dec;
      if(sd!==ss||over){E.curAcc=Math.min(a,E.curAcc+jstep);dec=E.curAcc;}
      else{const want=Math.min(a,aReq);E.curAcc=E.curAcc<want?Math.min(want,E.curAcc+jstep):Math.max(want,E.curAcc-jstep);
        dec=aReq>a*0.9?Math.min(aReq,a*1.5):E.curAcc;}
      const s2=sv*sv-2*dec;
      if(s2<=v0*v0)E.vel=(sd===ss)?ss*v0:sd*v0;else E.vel=ss*Math.sqrt(s2);
    }else if(sv<vmax){
      const taper=(vmax-sv)<=(E.curAcc+aMin)/2*(E.curAcc-aMin)/E.jerk;  // perto da velocidade de cruzeiro: suaviza
      const want=taper?aMin:a;
      E.curAcc=E.curAcc<want?Math.min(want,E.curAcc+jstep):Math.max(want,E.curAcc-jstep);
      E.vel=ss*Math.min(Math.sqrt(sv*sv+2*E.curAcc),vmax);
    }else E.curAcc=Math.max(aMin,E.curAcc-jstep);
  }

  // Uma volta do loop(). Retorna true se deu um passo (e avançou o tempo pelo período).
  E.service=function(){
    if(E.mode==='jog'&&millis()-jogLast>450){log('Jog: sem renovação há 450 ms, parando.');stopRamp();}
    if(E.mode==='resonance'){
      const el=(millis()-resStart)/1000;
      if(el>=resDur){log('Ressonância concluída.');stopRamp();}
      else{E.resRpmNow=resMin+(resMax-resMin)*el/resDur;E.target=E.pos+resDir*BIG;}
    }
    if(E.vel===0&&E.pos===E.target){
      if(pendCfg){E.MICRO=pendCfg.ms;E.RATIO=pendCfg.ratio;E.applyCfg();E.pos=0;E.target=0;seqBase=0;pendCfg=null;log('Motor: config aplicada. Posição zerada.');}
      if(pendEnaOff){pendEnaOff=false;setEna(false);log('Torque solto.');}
      onArrived();return false;
    }
    updateVelocity();
    if(E.vel===0)return false;
    E.pos+=E.vel>0?1:-1;E.t+=1/Math.abs(E.vel);return true;
  };
  // Avança a simulação até o instante 'until' (s), chamando 'sample' a cada 10 ms.
  E.runUntil=function(until,sample){
    while(E.t<until){
      const before=E.t;
      if(!E.service())E.t=Math.min(until,(Math.round(E.t*1000)+1)/1000);   // ocioso: avança 1 ms
      if(sample){const a=Math.floor(before*100),b=Math.floor(E.t*100);for(let k=a+1;k<=b;k++)sample(k/100);}
    }
  };
  E.subMode=function(){
    switch(E.mode){
      case 'loop':return 'ciclo '+(E.cycles+1)+(seqCycles?'/'+seqCycles:'');
      case 'sweep':return E.sweepRpm.toFixed(1)+' rpm';
      case 'accsweep':return E.sweepAcc.toFixed(0)+' p/s²';
      case 'resonance':return E.resRpmNow.toFixed(1)+' rpm';
      case 'backlash':return dwelling&&seqPhase===1?'hold+':dwelling&&seqPhase===3?'hold-':['→ +','→ 0','→ −','→ 0'][seqPhase];
      case 'drift':return (E.cycles+1)+'/'+seqCycles;
      case 'stiction':return seqPhase?'voltando':E.stK+'/'+stCount;
      case 'waypoints':return 'wp '+(E.wpIdx+1)+'/'+E.wp.length;
      case 'jog':return 'jog '+(jogDir>0?'+':'−');
    }
    return '';
  };
}
if(typeof module!=='undefined')module.exports=NemaEngine;
