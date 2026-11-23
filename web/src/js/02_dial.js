// ---------- Mostrador ----------
(function(){                                   // escala: traço a cada 5°, médio a cada 15°, grande a cada 90°
  const g=$('dticks');let h='';
  for(let a=0;a<360;a+=5){
    const maj=a%90===0,mid=a%15===0,r1=88,r0=maj?72:mid?80:84;
    const s=Math.sin(a*Math.PI/180),c=-Math.cos(a*Math.PI/180);
    h+='<line x1="'+(r0*s).toFixed(1)+'" y1="'+(r0*c).toFixed(1)+'" x2="'+(r1*s).toFixed(1)+'" y2="'+(r1*c).toFixed(1)+
       '" stroke="'+(maj?'#9898A8':mid?'#3C3C4E':'#24242F')+'" stroke-width="'+(maj?2:1.2)+'" stroke-linecap="round"/>';
  }
  [[0,'0'],[90,'90'],[180,'180'],[270,'-90']].forEach(([a,t])=>{
    const s=Math.sin(a*Math.PI/180),c=-Math.cos(a*Math.PI/180);
    h+='<text x="'+(54*s).toFixed(1)+'" y="'+(54*c+4).toFixed(1)+'" text-anchor="middle" font-size="12" fill="#9898A8">'+t+'</text>';
  });
  g.innerHTML=h;
})();
function arcPath(r,a0,a1){
  const p=a=>[(r*Math.sin(a*Math.PI/180)).toFixed(2),(-r*Math.cos(a*Math.PI/180)).toFixed(2)];
  const [x0,y0]=p(a0),[x1,y1]=p(a1);
  return 'M'+x0+','+y0+' A'+r+','+r+' 0 '+(Math.abs(a1-a0)>180?1:0)+' '+(a1>a0?1:0)+' '+x1+','+y1;
}
function dial(pos,tgt){
  $('dneedle').style.transform='rotate('+pos+'deg)';
  $('dtgt').style.transform='rotate('+tgt+'deg)';
  const turns=Math.trunc(pos/360),rem=pos-turns*360;
  $('dfull').setAttribute('d',turns?'M0,-78 A78,78 0 1 1 0,78 A78,78 0 1 1 0,-78':'');
  $('darc').setAttribute('d',Math.abs(rem)<0.5?'':arcPath(78,0,rem));
  $('dturn').textContent=turns?(turns>0?'+':'')+turns+(Math.abs(turns)===1?' volta':' voltas'):'';
}
dial(0,0);
