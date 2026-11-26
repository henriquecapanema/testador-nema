// ---------- Ligações: braço completo (nemabot) ----------
// Visão geral de todas as ligações do braço: alimentação, ESP32, CAN com as SERVO42D,
// TMC2209 do punho, tela da base e flange. Pinos com * ainda são proposta.
const ARM={j1:'servo',grp:''};
try{const x=JSON.parse(localStorage.getItem('nemabot.arm')||'null');if(x)Object.assign(ARM,x);}catch(e){}
const saveArm=()=>{try{localStorage.setItem('nemabot.arm',JSON.stringify(ARM));}catch(e){}};
const AC={v24:'#FB7185',gnd:'#8A8A9C',v5:'#6EE7B7',v6:'#FDBA74',v33:'#86EFAC',canH:'#FB923C',canL:'#38BDF8',step:'#93A5FF',dir:'#67E8F9',ena:'#FCD34D',
  uart:'#C4B5FD',tft:'#22D3EE',i2c:'#FDE047',pwm:'#F472B6',sens:'#A3E635',mA1:'#F87171',mA2:'#4ADE80',mB1:'#FACC15',mB2:'#60A5FA'};
const ARM_GRPS=[['','Tudo'],['pwr','Potência'],['can','CAN'],['tmc','Punho'],['tela','Tela'],['flange','Flange'],['sens','Sensores']];

function armSVG(){
  const tmcJ1=ARM.j1==='tmc';
  let o='',wires=[];
  const T=(x,y,t,a,cls,ex)=>'<text x="'+x+'" y="'+y+'"'+(a?' text-anchor="'+a+'"':'')+' class="'+(cls||'chips')+'"'+(ex||'')+'>'+t+'</text>';
  const dot=(x,y,c)=>'<circle cx="'+x+'" cy="'+y+'" r="5" fill="#0B0C10" stroke="'+(c||'#4A4F5E')+'" stroke-width="'+(c?2.2:1.2)+'"/>'+(c?'<circle cx="'+x+'" cy="'+y+'" r="2.2" fill="'+c+'"/>':'');
  const pad=(x,y)=>'<circle cx="'+x+'" cy="'+y+'" r="4" fill="#C9A227"/>';
  const W=(id,label,color,grp,pts,opt)=>wires.push({id,label,color,grp,pts,opt:opt||{}});
  const used={};const use=(k,c)=>{used[k]=c;};

  // ===== alimentação (coluna da esquerda) =====
  o+='<g class="board">';
  // tomada, chave e fusível
  o+='<rect x="40" y="70" width="230" height="74" rx="10" fill="#15161C" stroke="#3A3F4D"/>'+T(56,96,'Tomada','','bname',' text-anchor="start"')+T(56,114,'chave liga/desliga + fusível 5 A','','chips',' text-anchor="start"');
  o+='<rect x="210" y="84" width="44" height="22" rx="4" fill="#B91C1C"/>'+T(232,99,'I/O','middle','pin on small');
  // fonte
  o+='<rect x="40" y="190" width="230" height="110" rx="10" fill="#15161C" stroke="#3A3F4D"/>';
  for(let i=0;i<9;i++)o+='<rect x="'+(52+i*13)+'" y="202" width="8" height="40" rx="2" fill="#1B1E25"/>';
  o+=T(56,268,'Fonte 24 V','','bname',' text-anchor="start"')+T(56,286,'5 A · 120 W','','chips',' text-anchor="start"');
  o+=T(258,229,'V+','end','pin on')+T(258,269,'V−','end','pin on')+pad(270,225)+pad(270,265);
  // emergência
  o+='<rect x="40" y="380" width="230" height="100" rx="10" fill="#1F1508" stroke="#B45309"/>';
  o+='<circle cx="90" cy="430" r="30" fill="#FACC15"/><circle cx="90" cy="430" r="22" fill="#DC2626" stroke="#7F1D1D" stroke-width="3"/>';
  o+=T(132,424,'Emergência','','bname',' text-anchor="start"')+T(132,442,'cogumelo · NF','','chips',' text-anchor="start"')+T(132,458,'corta drivers e ferramenta','','chips',' text-anchor="start"');
  o+=pad(120,380)+pad(270,445);
  // step-down 5 V
  o+='<rect x="40" y="560" width="230" height="80" rx="10" fill="#101A16" stroke="#2C4A3C"/>'+T(56,590,'Step-down 24 → 5 V','','bname',' text-anchor="start"')+T(56,608,'3 A · antes da emergência','','chips',' text-anchor="start"');
  o+=pad(100,560)+pad(130,560)+T(100,576,'+','middle','pin on small')+T(130,576,'−','middle','pin on small');
  o+=pad(270,585)+pad(270,615)+T(256,589,'5V','end','pin on small')+T(256,619,'GND','end','pin on small');
  // step-down 6 V
  o+='<rect x="40" y="720" width="230" height="80" rx="10" fill="#1F140A" stroke="#7C4A1E"/>'+T(56,750,'Step-down 24 → 6 V','','bname',' text-anchor="start"')+T(56,768,'5 A · ferramenta (servo)','','chips',' text-anchor="start"');
  o+=pad(100,720)+pad(130,720)+T(100,736,'+','middle','pin on small')+T(130,736,'−','middle','pin on small');
  o+=pad(270,745)+pad(270,775)+T(256,749,'6V','end','pin on small')+T(256,779,'GND','end','pin on small');
  o+='</g>';

  // trilhos: 24 V_DRV (depois da emergência) e GND comum
  const GX=330,GTOP=52,GBOT=1374,VX=345,VTOP=40,VBOT=1214;
  W('gnd_rail','GND comum: tudo tem o terra ligado junto (obrigatório no CAN)',AC.gnd,'pwr',[[270,265],[GX,265],[GX,GTOP],[1790,GTOP]],{});
  W('gnd_rail2','GND comum (trilho de baixo)',AC.gnd,'pwr',[[GX,265],[GX,GBOT],[1790,GBOT]],{});
  W('v24_estop','24 V da fonte → entrada da emergência',AC.v24,'pwr',[[270,225],[300,225],[300,352],[120,352],[120,380]],{});
  W('v24_buck5','24 V da fonte → step-down 5 V (antes da emergência: o ESP32 continua ligado)',AC.v24,'pwr',[[270,229],[314,229],[314,532],[100,532],[100,560]],{});
  W('v24drv_top','24 V_DRV: saída da emergência → V+ das SERVO42D',AC.v24,'pwr',[[270,445],[VX,445],[VX,VTOP],[1790,VTOP]],{});
  W('v24drv_bot','24 V_DRV → VM dos TMC2209 e step-down 6 V',AC.v24,'pwr',[[VX,445],[VX,VBOT],[1790,VBOT]],{});
  W('v24_buck6','24 V_DRV → step-down 6 V (a ferramenta para junto com a emergência)',AC.v24,'pwr',[[VX,702],[100,702],[100,720]],{});
  W('g_buck5','GND do step-down 5 V',AC.gnd,'pwr',[[130,560],[130,546],[GX,546]],{});
  W('g_buck6','GND do step-down 6 V',AC.gnd,'pwr',[[130,720],[130,708],[GX,708]],{});
  o+=T(VX+8,VTOP-6,'24 V_DRV (após a emergência)','','pin on',' fill="'+AC.v24+'" style="fill:'+AC.v24+'"');
  o+=T(GX+8,GBOT-6,'GND comum','','pin on',' style="fill:'+AC.gnd+'"');

  // ===== ESP32 =====
  const ex=560,ey=250,ew=240,eh=560;
  const L=['EN','VP','VN','D34','D35','D32','D33','D25','D26','D27','D14','D12','D13','GND','VIN'];
  const R=['D23','D22','TX0','RX0','D21','D19','D18','D5','TX2','RX2','D4','D2','D15','GND','3V3'];
  const py=i=>ey+90+i*30;
  const P=n=>[ex+18,py(L.indexOf(n))],PR=n=>[ex+ew-18,py(R.indexOf(n))];

  // ===== CJMCU-2551 =====
  const cx=850,cy=86,cw=126,ch=150;
  const CP={VCC:[cx,cy+40],GND:[cx,cy+58],CTX:[cx,cy+76],CRX:[cx,cy+94],CANH:[cx+cw,cy+52],CANL:[cx+cw,cy+76]};

  // ===== SERVO42D =====
  const sv=[{id:'J1',x:1000,on:!tmcJ1,cid:'01',jump:'OFF',sub:'base · 2º kit',motor:'motor de 40 mm'},
            {id:'J3',x:1262,on:true,cid:'03',jump:'OFF',sub:'cotovelo · kit',motor:'motor de 40 mm'},
            {id:'J2',x:1524,on:true,cid:'02',jump:'ON',sub:'ombro · CAN MB',motor:'17HS6001S'}];
  const sy=80,sw=232,sh=210;
  const SL=['EVCC','EGND','IN_1','CAN H','CAN L'],SR=['V+','GND','COM','EN','STP','DIR'];
  const sl=(s,n)=>[s.x+14,sy+96+SL.indexOf(n)*20],sr=(s,n)=>[s.x+sw-14,sy+86+SR.indexOf(n)*20];

  // ===== TMC2209 =====
  const tmc=[];
  if(tmcJ1)tmc.push({id:'J1',x:880,addr:2,step:'D2',dir:'D15',motor:'17HS4401',m17:true});
  tmc.push({id:'J4',x:1110,addr:0,step:'D25',dir:'D26',motor:'NEMA 11'},{id:'J5',x:1340,addr:1,step:'D13',dir:'D14',motor:'NEMA 11'});
  const ty=930,tw=180,th=272;
  const TL=['VM','GND','2B','2A','1A','1B','VDD','GND'],TR=['EN','MS1','MS2','PDN','CLK','STEP','DIR'];
  const tl=(t,i)=>[t.x,ty+118+i*16],tr=(t,n)=>[t.x+tw,ty+118+TR.indexOf(n)*17];

  // ===== tela =====
  const dx=900,dy=330,dw=190,dh=240;
  const DL=['VCC','GND','CS','RES','A0','SDA','SCK','LED'],dp=n=>[dx,dy+40+DL.indexOf(n)*22];

  // ===== flange =====
  const fx=1590,fy=930,fw=196,fh=240;
  const FP=['V+','V+','GND','GND','PWM','ID','SDA','SCL'],fp=i=>[fx+16+i*23.5,fy+fh-18];

  // ---------- fios: CAN ----------
  use('D32',AC.canH);use('D33',AC.canL);use('EN',AC.tft);
  W('can_tx','CAN TX · D32 (GPIO 32) → CTX do CJMCU-2551',AC.step,'can',[P('D32'),[540,py(5)],[540,226],[828,226],[828,CP.CTX[1]],CP.CTX],{sig:1});
  W('can_rx','CAN RX · CRX do CJMCU-2551 → divisor 1 kΩ / 2 kΩ → D33 (a saída do MCP2551 vai a 5 V)',AC.dir,'can',[P('D33'),[532,py(6)],[532,218],[820,218],[820,CP.CRX[1]],CP.CRX],{sig:1});
  W('can_vcc','5 V do step-down → VCC do CJMCU-2551 (ele é de 5 V)',AC.v5,'can',[[460,585],[460,206],[812,206],[812,CP.VCC[1]],CP.VCC],{});
  W('can_gnd','GND → GND do CJMCU-2551',AC.gnd,'can',[[GX,GTOP],[GX,GTOP],[804,GTOP],[804,CP.GND[1]],CP.GND],{});
  const chain=[{p:CP,isT:true}].concat(sv.filter(s=>s.on));
  for(let k=0;k<chain.length-1;k++){
    const a=chain[k],b=chain[k+1];
    const aH=a.isT?a.p.CANH:sl(a,'CAN H'),aL=a.isT?a.p.CANL:sl(a,'CAN L'),bH=sl(b,'CAN H'),bL=sl(b,'CAN L');
    if(a.isT&&b.id==='J1'){
      W('canh_'+b.id,'CAN H: transceptor → CAN H da SERVO42D '+b.id,AC.canH,'can',[aH,[986,aH[1]],[986,bH[1]],bH],{sig:1});
      W('canl_'+b.id,'CAN L: transceptor → CAN L da SERVO42D '+b.id,AC.canL,'can',[aL,[992,aL[1]],[992,bL[1]],bL],{sig:1});
    }else{
      const sx=a.isT?aH[0]+12:a.x+4,yH=64,yL=70;
      W('canh_'+b.id,'CAN H: '+(a.isT?'transceptor':'SERVO42D '+a.id)+' → SERVO42D '+b.id+' (par trançado)',AC.canH,'can',[aH,[sx,aH[1]],[sx,yH],[b.x-6,yH],[b.x-6,bH[1]],bH],{sig:1});
      W('canl_'+b.id,'CAN L: '+(a.isT?'transceptor':'SERVO42D '+a.id)+' → SERVO42D '+b.id+' (par trançado)',AC.canL,'can',[aL,[sx+6,aL[1]],[sx+6,yL],[b.x-12,yL],[b.x-12,bL[1]],bL],{sig:1});
    }
  }
  // V+ e GND das SERVO42D
  sv.forEach(s=>{if(!s.on)return;const a=sr(s,'V+'),g=sr(s,'GND');
    W('sv_v_'+s.id,'24 V_DRV → V+ da SERVO42D '+s.id,AC.v24,'pwr',[[s.x+sw+8,VTOP],[s.x+sw+8,a[1]],a],{});
    W('sv_g_'+s.id,'GND → GND da SERVO42D '+s.id,AC.gnd,'pwr',[[s.x+sw+18,GTOP],[s.x+sw+18,g[1]],g],{});
  });

  // ---------- fios: ESP32 ----------
  use('VIN',AC.v5);use('GND',AC.gnd);
  W('esp_vin','5 V do step-down → VIN do ESP32',AC.v5,'pwr',[[270,585],[460,585],[460,py(14)],P('VIN')],{});
  W('esp_gnd','GND do ESP32 → GND comum',AC.gnd,'pwr',[P('GND'),[GX,py(13)]],{});

  // ---------- fios: TMC2209 ----------
  // sinais: saem pela esquerda do ESP32, passam por baixo dele e entram pela direita de cada TMC
  const leftDown=(pin,lx,ly,t,tn,k)=>{const b=tr(t,tn),vx=t.x+tw+10+k*6;return [P(pin),[lx,P(pin)[1]],[lx,ly],[vx,ly],[vx,b[1]],b];};
  const rightDown=(pin,rx,ly,t,tn,k)=>{const b=tr(t,tn),vx=t.x+tw+10+k*6;return [PR(pin),[rx,PR(pin)[1]],[rx,ly],[vx,ly],[vx,b[1]],b];};
  const laneL={D25:524,D26:516,D27:508,D13:500,D14:492},laneY={D25:840,D26:848,D27:856,D13:864,D14:872,TX2:880,RX2:888,D2:896,D15:904};
  tmc.forEach(t=>{
    const isL=t.step in laneL;
    if(isL){use(t.step,AC.step);use(t.dir,AC.dir);
      W('st_'+t.id,'STEP do '+t.id+' · '+t.step+' → STEP',AC.step,'tmc',leftDown(t.step,laneL[t.step],laneY[t.step],t,'STEP',0),{sig:1});
      W('di_'+t.id,'DIR do '+t.id+' · '+t.dir+' → DIR',AC.dir,'tmc',leftDown(t.dir,laneL[t.dir],laneY[t.dir],t,'DIR',1),{sig:1});
    }else{use(t.step,AC.step);use(t.dir,AC.dir);
      W('st_'+t.id,'STEP do J1 · D2 (GPIO 2, pino de boot) → STEP',AC.step,'tmc',rightDown(t.step,816,laneY.D2,t,'STEP',0),{sig:1});
      W('di_'+t.id,'DIR do J1 · D15 (GPIO 15, pino de boot) → DIR',AC.dir,'tmc',rightDown(t.dir,822,laneY.D15,t,'DIR',1),{sig:1});
    }
    W('en_'+t.id,'EN dos TMC2209 · D27 (comum, LOW = torque) → EN do '+t.id,AC.ena,'tmc',leftDown('D27',laneL.D27,laneY.D27,t,'EN',2),{});
    W('tx_'+t.id,'UART · TX2 (GPIO 17) → 1 kΩ → PDN do '+t.id+' (endereço '+t.addr+')',AC.uart,'tmc',rightDown('TX2',804,laneY.TX2,t,'PDN',3),{});
    W('rx_'+t.id,'UART · PDN do '+t.id+' → RX2 (GPIO 16)',AC.uart,'tmc',rightDown('RX2',810,laneY.RX2,t,'PDN',4),{});
    // potência e lógica (lado esquerdo)
    const vm=tl(t,0),g=tl(t,1),vdd=tl(t,6),gl=tl(t,7);
    W('vm_'+t.id,'24 V_DRV → VM do '+t.id+' (com 100 µF perto do driver)',AC.v24,'tmc',[[t.x-34,VBOT],[t.x-34,vm[1]],vm],{});
    W('gm_'+t.id,'GND → GND (motor) do '+t.id,AC.gnd,'tmc',[[t.x-40,GBOT],[t.x-40,g[1]],g],{});
    W('vdd_'+t.id,'3V3 do ESP32 → VDD (lógica) do '+t.id,AC.v33,'tmc',[PR('3V3'),[844,PR('3V3')[1]],[844,912],[t.x-14,912],[t.x-14,vdd[1]],vdd],{});
    W('gl_'+t.id,'GND → GND (lógica) do '+t.id,AC.gnd,'tmc',[[t.x-46,GBOT],[t.x-46,gl[1]],gl],{});
    // bobinas
    const mx=t.x+30,my=1236,cols=[[AC.mA1,'2B'],[AC.mA2,'2A'],[AC.mB1,'1A'],[AC.mB2,'1B']];
    cols.forEach(([c,n],i)=>{const p=tl(t,2+i);W('m'+i+'_'+t.id,'Bobina '+n+' do '+t.id+' → motor',c,'tmc',[p,[t.x-20+i*0,p[1]],[t.x-20,p[1]],[t.x-20,1226],[mx+12+i*22,1226],[mx+12+i*22,my]],{lanex:i});});
  });
  use('D27',AC.ena);use('TX2',AC.uart);use('RX2',AC.uart);use('3V3',AC.v33);

  // ---------- fios: tela ----------
  use('D23',AC.tft);use('D18',AC.tft);use('D5',AC.tft);use('D4',AC.tft);
  const toTela=(pin,lx,tn,lab,c)=>W('tft_'+tn,lab,c||AC.tft,'tela',[PR(pin),[lx,PR(pin)[1]],[lx,dp(tn)[1]],dp(tn)],{sig:1});
  toTela('D23',852,'SDA','Tela · D23 (MOSI, SPI nativo) → SDA');
  toTela('D18',858,'SCK','Tela · D18 (SCK, SPI nativo) → SCK');
  toTela('D5',864,'CS','Tela · D5* → CS');
  toTela('D4',870,'A0','Tela · D4* → A0 (DC)');
  W('tft_vcc','3V3 do ESP32 → VCC da tela',AC.v33,'tela',[PR('3V3'),[876,PR('3V3')[1]],[876,dp('VCC')[1]],dp('VCC')],{});
  W('tft_led','3V3 → LED da tela (luz de fundo sempre acesa)',AC.v33,'tela',[[876,dp('LED')[1]],dp('LED')],{});
  W('tft_gnd','GND do ESP32 → GND da tela',AC.gnd,'tela',[PR('GND'),[882,PR('GND')[1]],[882,dp('GND')[1]],dp('GND')],{});
  W('tft_res','EN do ESP32 → RES da tela (reinicia junto, sem gastar pino)',AC.tft,'tela',[P('EN'),[548,py(0)],[548,240],[888,240],[888,dp('RES')[1]],dp('RES')],{});

  // ---------- fios: flange ----------
  use('D22',AC.i2c);use('D21',AC.i2c);use('D19',AC.pwm);use('D34',AC.pwm);
  const toF=(pin,rx,ly,i,lab,c)=>{const b=fp(i);W('fl_'+i,lab,c,'flange',[PR(pin),[rx,PR(pin)[1]],[rx,ly],[b[0],ly],b],{});};
  toF('D21',826,1352,6,'Flange · D21 → SDA (via 7)',AC.i2c);
  toF('D22',832,1344,7,'Flange · D22 → SCL (via 8)',AC.i2c);
  toF('D19',838,1336,4,'Flange · D19* → PWM da ferramenta (via 5)',AC.pwm);
  W('fl_5','Flange · ID da ferramenta (via 6) → D34* (ADC; 10 kΩ ao 3V3 no robô, resistor na ferramenta)',AC.pwm,'flange',[P('D34'),[484,py(3)],[484,1328],[fp(5)[0],1328],fp(5)],{});
  W('fl_v6a','6 V do step-down → V+ da flange (vias 1 e 2)',AC.v6,'flange',[[270,745],[290,745],[290,1318],[fp(0)[0],1318],fp(0)],{});
  W('fl_v6b','6 V → V+ (via 2)',AC.v6,'flange',[[fp(0)[0],1318],[fp(1)[0],1318],fp(1)],{});
  W('fl_g','GND do step-down 6 V → GND da flange (vias 3 e 4, encostam primeiro)',AC.gnd,'flange',[[270,775],[282,775],[282,1360],[fp(2)[0],1360],fp(2)],{});
  W('fl_g2','GND → via 4',AC.gnd,'flange',[[fp(2)[0],1360],[fp(3)[0],1360],fp(3)],{});

  // ---------- fios: sensores (hall de homing) ----------
  use('VP',AC.sens);use('VN',AC.sens);
  W('hall_j4','Hall de homing do J4 → VP (GPIO 36*), pull-up 10 kΩ ao 3V3',AC.sens,'sens',[[452,py(1)],P('VP')],{});
  W('hall_j5','Hall de homing do J5 → VN (GPIO 39*), pull-up 10 kΩ ao 3V3',AC.sens,'sens',[[452,py(2)],P('VN')],{});
  if(tmcJ1){use('D35',AC.sens);W('hall_j1','Hall de homing do J1 → D35 (GPIO 35*)',AC.sens,'sens',[[452,py(4)],P('D35')],{});}

  // ===== desenho das placas =====
  // ESP32
  o+='<g class="board"><rect x="'+ex+'" y="'+ey+'" width="'+ew+'" height="'+eh+'" rx="16" fill="url(#apcb)" stroke="#2C3140"/>';
  o+='<path d="M'+(ex+70)+','+(ey+14)+' h20 v14 h12 v-14 h12 v14 h12 v-14 h12 v14 h12 v-14 h20" fill="none" stroke="#C9A227" stroke-width="2.4" stroke-linejoin="round" opacity=".85"/>';
  o+='<rect x="'+(ex+66)+'" y="'+(ey+40)+'" width="108" height="34" rx="5" fill="url(#achip)" stroke="#3A3F4D"/>'+T(ex+120,ey+62,'ESP32 · WROOM-32','middle','chipt');
  o+='<rect x="'+(ex+94)+'" y="'+(ey+eh-24)+'" width="52" height="28" rx="5" fill="#C7C9D1" stroke="#7B7F8C"/>'+T(ex+120,ey+eh-34,'ESP32 DevKit','middle','bname');
  L.forEach((n,i)=>{const c=used[n]||null;o+=dot(ex+18,py(i),c)+T(ex+30,py(i)+4,n,'',c?'pin on':'pin');});
  R.forEach((n,i)=>{const c=used[n]||null;o+=dot(ex+ew-18,py(i),c)+T(ex+ew-30,py(i)+4,n,'end',c?'pin on':'pin');});
  o+='</g>';
  // hall
  const hall=(y,t)=>'<g class="board"><rect x="384" y="'+(y-11)+'" width="68" height="22" rx="5" fill="#1A2410" stroke="#4D7C0F"/>'+T(418,y+4,t,'middle','pin on small')+'</g>';
  o+=hall(py(1),'hall J4')+hall(py(2),'hall J5')+(tmcJ1?hall(py(4),'hall J1'):'');
  // CJMCU-2551
  o+='<g class="board"><rect x="'+cx+'" y="'+cy+'" width="'+cw+'" height="'+ch+'" rx="8" fill="#5B1F6E" stroke="#8E3FA8"/>';
  o+='<rect x="'+(cx+38)+'" y="'+(cy+16)+'" width="50" height="22" rx="3" fill="#1A1B22"/>'+T(cx+63,cy+31,'MCP2551','middle','chips');
  ['VCC','GND','CTX','CRX'].forEach(n=>o+=pad(CP[n][0],CP[n][1])+T(CP[n][0]+10,CP[n][1]+4,n,'','pin on small'));
  ['CANH','CANL'].forEach(n=>o+=pad(CP[n][0],CP[n][1])+T(CP[n][0]-10,CP[n][1]+4,n,'end','pin on small'));
  o+=T(cx+cw/2,cy+122,'CJMCU-2551','middle','bname')+T(cx+cw/2,cy+140,'CAN · 5 V · +120 Ω','middle','chips')+'</g>';
  let top='<g class="board" pointer-events="none"><rect x="800" y="'+(CP.CRX[1]+16)+'" width="40" height="18" rx="9" fill="#15161C" stroke="'+AC.dir+'"/>'+T(820,CP.CRX[1]+29,'1k/2k','middle','pin on small')+'</g>';
  // SERVO42D
  sv.forEach(s=>{
    const op=s.on?'':' opacity=".28"';
    o+='<g class="board"'+op+'><rect x="'+(s.x-6)+'" y="'+(sy-6)+'" width="'+(sw+12)+'" height="'+(sh+12)+'" rx="16" fill="url(#ametal)" stroke="#4A4F5E"'+(s.on?'':' stroke-dasharray="6 5"')+'/>';
    o+='<rect x="'+s.x+'" y="'+sy+'" width="'+sw+'" height="'+sh+'" rx="12" fill="#101114" stroke="#30343F"/>';
    o+='<rect x="'+(s.x+sw/2-36)+'" y="'+(sy+10)+'" width="72" height="20" rx="3" fill="#1D4ED8"/>';
    for(let i=0;i<4;i++)o+='<circle cx="'+(s.x+sw/2-27+i*18)+'" cy="'+(sy+20)+'" r="4.5" fill="#C9CCD6"/>';
    o+=T(s.x+sw/2,sy+50,s.id+' · SERVO42D','middle','bname')+T(s.x+sw/2,sy+66,s.sub+' · '+s.motor,'middle','chips');
    o+='<rect x="'+(s.x+sw/2-26)+'" y="'+(sy+108)+'" width="52" height="30" rx="3" fill="#0B1530" stroke="#3B4A7A"/>'+T(s.x+sw/2,sy+127,'SR_vFOC','middle','pin on small');
    o+=T(s.x+sw/2,sy+158,'CanID '+s.cid,'middle','pin on small')+T(s.x+sw/2,sy+174,'120 Ω: '+s.jump,'middle',s.jump==='ON'?'pin on small':'chips');
    if(!s.on)o+=T(s.x+sw/2,sy+194,'(J1 com TMC2209)','middle','chips');
    o+='<rect x="'+(s.x+5)+'" y="'+(sy+86)+'" width="18" height="102" rx="3" fill="#2E8B57"/>';
    SL.forEach((n,i)=>{const p=sl(s,n);o+='<circle cx="'+p[0]+'" cy="'+p[1]+'" r="4" fill="#C9CCD6"/>'+T(p[0]+14,p[1]+4,n,'',i>=3?'pin on small':'pin small');});
    o+='<rect x="'+(s.x+sw-23)+'" y="'+(sy+76)+'" width="18" height="122" rx="3" fill="#2E8B57"/>';
    SR.forEach((n,i)=>{const p=sr(s,n);o+='<circle cx="'+p[0]+'" cy="'+p[1]+'" r="4" fill="#C9CCD6"/>'+T(p[0]-14,p[1]+4,n,'end',i<2?'pin on small':'pin small');});
    o+='</g>';
  });
  o+=T(1262,sy+sh+24,'hall de homing de cada SERVO42D vai no IN_1 dela (EVCC/EGND/IN_1)','','chips');
  // TMC2209 + motores
  tmc.forEach(t=>{
    o+='<g class="board"><rect x="'+t.x+'" y="'+ty+'" width="'+tw+'" height="'+th+'" rx="12" fill="url(#atmc)" stroke="#5B3FA0"/>';
    o+='<rect x="'+(t.x+48)+'" y="'+(ty+40)+'" width="84" height="56" rx="6" fill="#1A1B22" stroke="#3A3F4D"/>';
    for(let i=0;i<6;i++)o+='<rect x="'+(t.x+54+i*13)+'" y="'+(ty+46)+'" width="7" height="44" rx="2" fill="#2A2D36"/>';
    o+=T(t.x+tw/2,ty+24,t.id+' · TMC2209','middle','bname')+T(t.x+tw/2,ty+th-10,'UART end. '+t.addr+' · '+t.motor,'middle','chips');
    TL.forEach((n,i)=>{const p=tl(t,i);o+=pad(p[0],p[1])+T(p[0]+10,p[1]+4,n,'','pin on small');});
    TR.forEach(n=>{const p=tr(t,n);o+=pad(p[0],p[1])+T(p[0]-10,p[1]+4,n,'end',(n==='MS1'||n==='MS2'||n==='CLK')?'pin small':'pin on small');});
    o+='<g transform="translate('+(t.x+tw/2-8)+','+(ty+th-58)+')"><rect x="0" y="0" width="16" height="22" rx="3" fill="#1D3B6B" stroke="#3B6BB5"/><rect x="0" y="0" width="16" height="5" rx="2" fill="#9CB8E8"/></g>'+T(t.x+tw/2,ty+th-26,'100 µF no VM','middle','chips');
    o+='</g>';
    const mx=t.x+30,s=t.m17?96:84;
    o+='<g class="board"><rect x="'+mx+'" y="1236" width="'+s+'" height="'+(t.m17?78:70)+'" rx="10" fill="url(#ametal)" stroke="#4A4F5E"/>'+
      '<circle cx="'+(mx+s/2)+'" cy="'+(1236+(t.m17?39:35))+'" r="16" fill="#16181F" stroke="#5B6070"/><circle cx="'+(mx+s/2)+'" cy="'+(1236+(t.m17?39:35))+'" r="5" fill="#C7C9D1"/>'+
      T(mx+s+8,1262,t.id,'','pin on small')+T(mx+s+8,1278,t.motor,'','chips')+'</g>';
  });
  // tela redonda
  o+='<g class="board"><rect x="'+dx+'" y="'+dy+'" width="'+dw+'" height="'+dh+'" rx="14" fill="#9F1D2B" stroke="#7F1D1D"/>';
  o+='<circle cx="'+(dx+dw/2+12)+'" cy="'+(dy+118)+'" r="70" fill="#0B0C10" stroke="#2A2D36" stroke-width="3"/><circle cx="'+(dx+dw/2+12)+'" cy="'+(dy+118)+'" r="62" fill="url(#ascreen)"/>';
  o+='<path d="M'+(dx+dw/2+12-46)+','+(dy+118+30)+' A54,54 0 1 1 '+(dx+dw/2+12+46)+','+(dy+118+30)+'" fill="none" stroke="#F97316" stroke-width="6" stroke-linecap="round"/>';
  o+=T(dx+dw/2+12,dy+114,'J2 42°','middle','bname')+T(dx+dw/2+12,dy+132,'nemabot','middle','chips');
  DL.forEach(n=>{const p=dp(n);o+=pad(p[0],p[1])+T(p[0]+10,p[1]+4,n,'','pin on small');});
  o+=T(dx+dw/2,dy+dh-12,'GC9A01 · 1,28" · 240×240','middle','chips')+'</g>';
  // flange
  o+='<g class="board"><rect x="'+fx+'" y="'+fy+'" width="'+fw+'" height="'+fh+'" rx="14" fill="#1F140A" stroke="#7C4A1E"/>';
  o+='<circle cx="'+(fx+fw/2)+'" cy="'+(fy+92)+'" r="62" fill="#2B2E37" stroke="#5B6070"/>';
  for(let i=0;i<5;i++){const a=-Math.PI/2+i*2*Math.PI/5;o+='<circle cx="'+(fx+fw/2+46*Math.cos(a))+'" cy="'+(fy+92+46*Math.sin(a))+'" r="5" fill="#0B0C10" stroke="#C9A227"/>';}
  o+='<rect x="'+(fx+fw/2-30)+'" y="'+(fy+84)+'" width="60" height="16" rx="4" fill="#0B0C10" stroke="#5B6070"/>';
  for(let i=0;i<8;i++)o+='<circle cx="'+(fx+fw/2-24.5+i*7)+'" cy="'+(fy+92)+'" r="2.2" fill="#E5C07B"/>';
  o+=T(fx+fw/2,fy+170,'Flange · pogo 8 vias','middle','bname')+T(fx+fw/2,fy+20,'cabo passa pelo J5 (±180°)','middle','chips');
  FP.forEach((n,i)=>{const p=fp(i);o+=pad(p[0],p[1])+'<text transform="translate('+(p[0]+4)+','+(p[1]-8)+') rotate(-90)" class="pin on small">'+n+'</text>';});
  o+='</g>';

  // ===== fios → svg =====
  const path=pts=>{let d='M'+pts[0][0]+','+pts[0][1];
    for(let i=1;i<pts.length;i++){const [x,y]=pts[i];
      if(i<pts.length-1){const [px,py0]=pts[i-1],[nx,ny]=pts[i+1];
        const r=7,d1=Math.min(r,Math.hypot(x-px,y-py0)/2),d2=Math.min(r,Math.hypot(nx-x,ny-y)/2);
        const ax=x-(x-px)/(Math.hypot(x-px,y-py0)||1)*d1,ay=y-(y-py0)/(Math.hypot(x-px,y-py0)||1)*d1;
        const bx=x+(nx-x)/(Math.hypot(nx-x,ny-y)||1)*d2,by=y+(ny-y)/(Math.hypot(nx-x,ny-y)||1)*d2;
        d+=' L'+ax+','+ay+' Q'+x+','+y+' '+bx+','+by;}
      else d+=' L'+x+','+y;}
    return d;};
  let wl='';
  wires.forEach(w=>{
    const d=path(w.pts),a=w.pts[0],b=w.pts[w.pts.length-1];
    const off=(w.grp==='can'&&!w.id.startsWith('can_')&&false);
    wl+='<g class="wire g-'+w.grp+(off?' off':'')+'" data-id="'+w.id+'" data-label="'+w.label+'">'+
      '<path d="'+d+'" class="halo"/><path d="'+d+'" stroke="'+w.color+'" class="core"/>'+(w.opt.sig?'<path d="'+d+'" class="flow"/>':'')+
      '<circle cx="'+a[0]+'" cy="'+a[1]+'" r="3" fill="'+w.color+'"/><circle cx="'+b[0]+'" cy="'+b[1]+'" r="3" fill="'+w.color+'"/></g>';
  });
  const defs='<defs><linearGradient id="apcb" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#16203A"/><stop offset="1" stop-color="#0F1628"/></linearGradient>'+
    '<linearGradient id="achip" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#3A3E4A"/><stop offset="1" stop-color="#22252E"/></linearGradient>'+
    '<linearGradient id="ametal" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#2B2E37"/><stop offset="1" stop-color="#1A1C22"/></linearGradient>'+
    '<linearGradient id="atmc" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#3B2A6B"/><stop offset="1" stop-color="#271B4A"/></linearGradient>'+
    '<radialGradient id="ascreen"><stop offset="0" stop-color="#1F2937"/><stop offset="1" stop-color="#0B0C10"/></radialGradient></defs>';
  // os fios vão por cima das placas para sair direto dos bornes
  return '<svg viewBox="0 0 1800 1390" class="wsvg arm">'+defs+o+wl+top+'</svg>';
}

function armRows(){
  const tmcJ1=ARM.j1==='tmc';
  const g=(t,rows)=>'<div class="wrow wgrp"><b>'+t+'</b></div>'+rows.map(([c,a,b,d])=>'<div class="wrow"><i style="background:'+c+'"></i><b>'+a+'</b><span>'+b+'</span><span class="arr">→</span><span>'+d+'</span></div>').join('');
  return g('Potência',[[AC.v24,'Entrada','tomada → chave → fusível 5 A','fonte 24 V'],[AC.v24,'Lógica','24 V (antes da emergência)','step-down 5 V → VIN do ESP32 e VCC do CJMCU-2551'],
      [AC.v24,'24 V_DRV','saída da emergência (NF)','V+ das SERVO42D, VM dos TMC2209, step-down 6 V'],[AC.gnd,'GND comum','fonte V−','tudo (obrigatório no CAN)']])+
    g('CAN',[[AC.step,'CAN TX','D32 (GPIO 32)','CTX do CJMCU-2551'],[AC.dir,'CAN RX','CRX, por divisor 1 kΩ / 2 kΩ','D33 (GPIO 33)'],
      [AC.canH,'Barramento','CANH / CANL do transceptor',(tmcJ1?'':'J1 (01) → ')+'J3 (03) → J2 (02, jumper ON)']])+
    g('Punho (TMC2209)',[[AC.step,'J4','STEP D25 · DIR D26','TMC2209 end. 0 → NEMA 11'],[AC.step,'J5','STEP D13* · DIR D14*','TMC2209 end. 1 → NEMA 11']].concat(tmcJ1?[[AC.step,'J1','STEP D2* · DIR D15* (boot)','TMC2209 end. 2 → 17HS4401']]:[]).concat([
      [AC.ena,'EN comum','D27 (LOW = torque)','EN de todos'],[AC.uart,'UART','TX2 (17) por 1 kΩ + RX2 (16)','PDN de todos (endereço por MS1/MS2)'],[AC.v33,'Lógica','3V3 / GND','VDD / GND'],[AC.v24,'Motor','24 V_DRV (+100 µF)','VM / GND']]))+
    g('Tela',[[AC.tft,'SPI','D18 SCK · D23 SDA','SCK · SDA'],[AC.tft,'Controle','D5* CS · D4* DC','CS · A0'],[AC.tft,'Reset','EN do ESP32','RES'],[AC.v33,'Alimentação','3V3 / GND','VCC, LED / GND']])+
    g('Flange',[[AC.v6,'Potência','step-down 6 V','vias 1–2 (V+) e 3–4 (GND)'],[AC.pwm,'PWM','D19*','via 5'],[AC.pwm,'ID','via 6 (10 kΩ ao 3V3)','D34* (ADC)'],[AC.i2c,'I²C','D21 SDA · D22 SCL','vias 7 · 8']])+
    g('Sensores',[[AC.sens,'Hall J4 / J5','VP (36*) / VN (39*)','pull-up 10 kΩ ao 3V3']].concat(tmcJ1?[[AC.sens,'Hall J1','D35*','pull-up 10 kΩ']]:[[AC.sens,'Hall J1–J3','IN_1 de cada SERVO42D','EVCC / EGND']]));
}

function renderArmCard(){
  document.querySelectorAll('#j1Seg button').forEach(b=>b.classList.toggle('active',b.dataset.j===ARM.j1));
  $('armGrp').innerHTML=ARM_GRPS.map(([k,n])=>'<button class="chip'+(ARM.grp===k?' active':'')+'" data-g="'+k+'">'+n+'</button>').join('');
  $('armGrp').querySelectorAll('button').forEach(b=>b.onclick=()=>{ARM.grp=b.dataset.g;saveArm();applyArmGrp();renderArmCard();});
}
function applyArmGrp(){const box=$('wdiag');if(WIRE.drv==='braco'&&ARM.grp)box.dataset.grp=ARM.grp;else delete box.dataset.grp;}
document.querySelectorAll('#j1Seg button').forEach(b=>b.onclick=()=>{ARM.j1=b.dataset.j;saveArm();renderWiring();});
