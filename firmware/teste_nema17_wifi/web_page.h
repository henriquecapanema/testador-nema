// web_page.h — a página inteira (HTML + CSS + JS), servida direto da flash.
// Sem nada externo: funciona também na rede própria "nemabot", sem internet.
// GERADO por tools/build_web.py a partir de web/src. Não edite à mão.
#pragma once
#include <Arduino.h>

const char PAGE[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="pt-br"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>nemabot</title>
<style>
:root{
  --bg-base:#09090B;--bg-surface:#111115;--bg-elevated:#1A1A22;--bg-hover:#22222C;
  --border-subtle:#24242F;--border-active:#3C3C4E;
  --text-primary:#F4F4F6;--text-secondary:#9898A8;--text-muted:#5E5E70;
  --accent-primary:#93A5FF;--accent-primary-dim:rgba(147,165,255,.14);
  --accent-success:#6EE7B7;--accent-warning:#FCD34D;--accent-danger:#FB7185;--accent-cyan:#67E8F9;
  --radius-sm:6px;--radius-md:10px;--radius-lg:14px;--ease:cubic-bezier(.2,.8,.2,1);
}
*{box-sizing:border-box}
html,body{margin:0;background:var(--bg-base);color:var(--text-primary)}
body{font:14px/1.45 -apple-system,BlinkMacSystemFont,"Inter","Segoe UI",sans-serif;-webkit-font-smoothing:antialiased}
.num,.tnum{font-variant-numeric:tabular-nums}
button{font:inherit;color:inherit}
svg{display:block}

/* ---------- header ---------- */
.top{position:sticky;top:0;z-index:20;height:56px;display:grid;grid-template-columns:1fr auto 1fr;align-items:center;gap:16px;
  padding:0 20px;background:rgba(9,9,11,.86);backdrop-filter:blur(10px);-webkit-backdrop-filter:blur(10px);border-bottom:1px solid var(--border-subtle)}
.brand{display:flex;align-items:center;gap:8px;font-weight:650;font-size:16px;letter-spacing:-.01em}
.badge{display:inline-flex;align-items:center;gap:5px;height:20px;padding:0 7px;border-radius:999px;font-size:11px;font-weight:600;
  background:var(--bg-elevated);border:1px solid var(--border-subtle);color:var(--text-secondary);white-space:nowrap}
.badge.ok{color:var(--accent-success);border-color:rgba(110,231,183,.3);background:rgba(110,231,183,.08)}
.badge.warn{color:var(--accent-warning);border-color:rgba(252,211,77,.3);background:rgba(252,211,77,.08)}
.badge.pri{color:var(--accent-primary);border-color:rgba(147,165,255,.35);background:var(--accent-primary-dim)}
.badge.err{color:var(--accent-danger);border-color:rgba(251,113,133,.3);background:rgba(251,113,133,.08)}
.tabs{display:flex;padding:3px;gap:2px;border-radius:999px;background:var(--bg-surface);border:1px solid var(--border-subtle)}
.tab{position:relative;height:30px;padding:0 16px;border:0;border-radius:999px;background:transparent;color:var(--text-secondary);cursor:pointer;font-weight:550;
  transition:background 120ms var(--ease),color 120ms var(--ease)}
.tab:hover{color:var(--text-primary)}
.tab.active{background:var(--bg-elevated);color:var(--text-primary);box-shadow:inset 0 0 0 1px var(--border-active)}
.tdot{position:absolute;top:6px;right:7px;width:6px;height:6px;border-radius:50%;background:var(--accent-warning);display:none}
.hright{display:flex;justify-content:flex-end;align-items:center;gap:10px}
.pill{display:flex;align-items:center;gap:8px;height:30px;padding:0 12px;border-radius:999px;background:var(--bg-surface);border:1px solid var(--border-subtle);
  color:var(--text-secondary);font-size:12.5px;white-space:nowrap}
.pdot{width:8px;height:8px;border-radius:50%;background:var(--text-muted);flex:none}
.pdot.ok{background:var(--accent-success);animation:pulse 2s infinite}
.pdot.warn{background:var(--accent-warning)}
.pdot.err{background:var(--accent-danger)}
@keyframes pulse{0%{box-shadow:0 0 0 0 rgba(110,231,183,.5)}70%{box-shadow:0 0 0 6px rgba(110,231,183,0)}100%{box-shadow:0 0 0 0 rgba(110,231,183,0)}}
.stop{display:flex;align-items:center;gap:8px;height:34px;padding:0 10px 0 12px;border:0;border-radius:var(--radius-md);cursor:pointer;
  background:var(--accent-danger);color:#18080B;font-weight:750;letter-spacing:.04em;transition:filter 120ms var(--ease),transform 80ms var(--ease)}
.stop:hover{filter:brightness(1.08)}
.stop:active{transform:scale(.97)}
.stop kbd{font:600 10px/1 -apple-system,sans-serif;padding:3px 5px;border-radius:4px;background:rgba(24,8,11,.16);letter-spacing:0}

/* ---------- layout ---------- */
main{max-width:1600px;margin:0 auto;padding:18px 20px 40px}
.grid{display:grid;grid-template-columns:360px minmax(0,1fr);gap:14px;align-items:start}
.col{display:grid;gap:14px;min-width:0}
.sys{max-width:720px;margin:0 auto;display:grid;gap:14px}
[hidden]{display:none!important}
@media(max-width:960px){.grid{grid-template-columns:1fr}.top{grid-template-columns:auto 1fr;height:auto;padding:10px 14px;row-gap:8px}
  .tabs{grid-column:1/-1;grid-row:2;justify-self:center}.pill span.txt{display:none}}

.card{min-width:0;background:var(--bg-surface);border:1px solid var(--border-subtle);border-radius:var(--radius-lg);padding:16px;transition:border-color .3s var(--ease)}
.card.flash{border-color:var(--accent-success)}
.ch{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:10px;margin-bottom:12px;min-height:24px}
.ct{display:flex;align-items:center;gap:8px;font-size:12px;font-weight:650;letter-spacing:.06em;text-transform:uppercase;color:var(--text-secondary)}
.row{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.row+.row{margin-top:10px}
.sub{font-size:12px;color:var(--text-muted)}
.lbl{display:block;font-size:12px;color:var(--text-secondary);margin-bottom:6px}

/* ---------- buttons ---------- */
.btn{display:inline-flex;align-items:center;justify-content:center;gap:7px;height:36px;padding:0 14px;border-radius:var(--radius-md);cursor:pointer;
  background:var(--bg-elevated);border:1px solid var(--border-subtle);color:var(--text-primary);font-weight:550;white-space:nowrap;
  transition:background 120ms var(--ease),border-color 120ms var(--ease),transform 80ms var(--ease),box-shadow 160ms var(--ease)}
.btn:hover{background:var(--bg-hover);border-color:var(--border-active)}
.btn:active{transform:scale(.97)}
.btn:disabled{opacity:.6;cursor:default}
.btn.pri{background:var(--accent-primary);border-color:var(--accent-primary);color:#09090B}
.btn.pri:hover{filter:brightness(1.07)}
.btn.danger{color:var(--accent-danger)}
.btn.danger.confirm{background:var(--accent-danger);border-color:var(--accent-danger);color:#18080B}
.btn.sm{height:30px;padding:0 10px;font-size:13px;border-radius:var(--radius-sm)}
.btn.icon{width:30px;padding:0}
.btn.block{width:100%}
.btn.on-pri{border-color:var(--accent-primary);color:var(--accent-primary);background:var(--accent-primary-dim)}
.btn.on-warn{border-color:var(--accent-warning);color:var(--accent-warning);background:rgba(252,211,77,.08)}
.btn.ack{box-shadow:0 0 0 2px rgba(110,231,183,.55)}
.spin{width:14px;height:14px;animation:rot .8s linear infinite}
@keyframes rot{to{transform:rotate(360deg)}}

/* ---------- fields ---------- */
.field{position:relative;display:flex;min-width:0;align-items:center;height:36px;border-radius:var(--radius-md);background:var(--bg-base);border:1px solid var(--border-subtle);
  transition:border-color 120ms var(--ease),box-shadow 120ms var(--ease)}
.field:focus-within{border-color:var(--accent-primary);box-shadow:0 0 0 2px var(--accent-primary-dim)}
.field.ackf{border-color:var(--accent-success);box-shadow:0 0 0 2px rgba(110,231,183,.18)}
.field input,.field select{flex:1;min-width:0;width:100%;height:100%;padding:0 10px;border:0;outline:0;background:transparent;color:var(--text-primary);font:inherit}
.field input[type=number]{font-variant-numeric:tabular-nums}
.field .unit{padding-right:10px;color:var(--text-muted);font-size:12.5px;white-space:nowrap;pointer-events:none}
.field select{appearance:none;-webkit-appearance:none;cursor:pointer;padding-right:28px}
.field.sel::after{content:"";position:absolute;right:12px;top:50%;width:6px;height:6px;margin-top:-5px;border-right:1.5px solid var(--text-secondary);border-bottom:1.5px solid var(--text-secondary);transform:rotate(45deg);pointer-events:none}
.field .eye{width:34px;height:100%;border:0;background:transparent;color:var(--text-muted);cursor:pointer;display:flex;align-items:center;justify-content:center}
.field .eye:hover{color:var(--text-primary)}
input[type=number]{-moz-appearance:textfield}
input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0;display:none}
select option{background:var(--bg-elevated);color:var(--text-primary)}

/* segmented */
.seg{display:inline-flex;padding:3px;gap:2px;border-radius:var(--radius-md);background:var(--bg-base);border:1px solid var(--border-subtle)}
.seg button{height:28px;padding:0 12px;border:0;border-radius:7px;background:transparent;color:var(--text-secondary);cursor:pointer;font-weight:550;font-size:13px;
  transition:background 120ms var(--ease),color 120ms var(--ease)}
.seg button.active{background:var(--bg-elevated);color:var(--text-primary);box-shadow:inset 0 0 0 1px var(--border-active)}

/* switch */
.switch{display:inline-flex;align-items:center;gap:8px;cursor:pointer;font-size:13px;color:var(--text-secondary);user-select:none}
.switch input{position:absolute;opacity:0;pointer-events:none}
.switch .trk{position:relative;width:32px;height:18px;border-radius:999px;background:var(--bg-hover);border:1px solid var(--border-active);transition:background 140ms var(--ease),border-color 140ms var(--ease)}
.switch .trk::after{content:"";position:absolute;top:2px;left:2px;width:12px;height:12px;border-radius:50%;background:var(--text-secondary);transition:transform 160ms var(--ease),background 140ms var(--ease)}
.switch input:checked+.trk{background:var(--accent-primary-dim);border-color:var(--accent-primary)}
.switch input:checked+.trk::after{transform:translateX(14px);background:var(--accent-primary)}

/* tooltip "?" */
.q{position:relative;display:inline-flex;align-items:center;justify-content:center;width:17px;height:17px;border-radius:50%;border:1px solid var(--border-active);
  color:var(--text-secondary);font-size:11px;font-weight:700;cursor:help;text-transform:none;letter-spacing:0;outline:0}
.q:hover,.q:focus{color:var(--text-primary);border-color:var(--text-secondary)}
.q::after{content:attr(data-tip);position:absolute;left:50%;top:calc(100% + 8px);transform:translateX(-50%);width:max-content;max-width:280px;padding:9px 11px;
  border-radius:var(--radius-md);background:var(--bg-elevated);border:1px solid var(--border-active);color:var(--text-primary);font-size:12.5px;font-weight:450;line-height:1.5;
  white-space:pre-line;text-align:left;opacity:0;pointer-events:none;transition:opacity 120ms var(--ease);z-index:30;box-shadow:0 10px 30px rgba(0,0,0,.5)}
.q:hover::after,.q:focus::after{opacity:1}
.q.right::after{left:auto;right:-6px;transform:none}

/* ---------- telemetria ---------- */
.tele{display:flex;gap:16px;align-items:center}
#dial{flex:none}
#dial .rot{transition:transform .12s linear}
.read{font-size:36px;font-weight:650;letter-spacing:-.02em;line-height:1.1;font-variant-numeric:tabular-nums}
.read small{font-size:20px;color:var(--text-secondary);font-weight:500;margin-left:2px}
.kv{display:grid;grid-template-columns:auto minmax(0,1fr);gap:4px 12px;margin-top:8px;font-size:13px;font-variant-numeric:tabular-nums;white-space:nowrap}
.kv dt{color:var(--text-secondary);display:flex;align-items:center;gap:6px}
.kv dd{margin:0;min-width:0}
.moderow{margin-top:10px;min-width:0}
.moderow .badge{display:inline-block;max-width:100%;overflow:hidden;text-overflow:ellipsis;line-height:18px;height:22px;padding:2px 9px}
.sw-tri{width:0;height:0;border-left:5px solid transparent;border-right:5px solid transparent;border-top:8px solid var(--accent-warning)}
.sw-line{width:12px;height:3px;border-radius:2px}

/* ---------- gráfico ---------- */
#chart{position:relative}
canvas{width:100%;display:block;touch-action:none;cursor:crosshair}
.ctitle{display:flex;align-items:center;gap:14px;font-size:12.5px;color:var(--text-secondary);margin:2px 0 4px}
.leg{display:inline-flex;align-items:center;gap:6px;font-size:12px;color:var(--text-secondary)}
.leg i{display:inline-block;width:16px;height:0}
#tip{position:absolute;pointer-events:none;display:none;background:var(--bg-surface);border:1px solid var(--border-active);border-radius:var(--radius-md);padding:7px 10px;
  font-size:12px;line-height:1.55;color:var(--text-primary);font-variant-numeric:tabular-nums;white-space:nowrap;z-index:5;box-shadow:0 8px 24px rgba(0,0,0,.45)}
#tip i{display:inline-block;width:12px;height:0;margin-right:6px;vertical-align:middle}
.pop{position:absolute;right:16px;top:52px;z-index:6;width:max-content;padding:12px 14px;border-radius:var(--radius-md);background:var(--bg-elevated);border:1px solid var(--border-active);
  font-size:12.5px;line-height:1.7;color:var(--text-primary);box-shadow:0 12px 30px rgba(0,0,0,.5)}
.pop div{display:flex;gap:14px;white-space:nowrap}
.pop b{color:var(--text-secondary);font-weight:500;min-width:132px}

/* ---------- barra ---------- */
.chips{display:flex;gap:6px;flex-wrap:wrap}
.chip{height:28px;padding:0 10px;border-radius:999px;border:1px solid var(--border-subtle);background:var(--bg-base);color:var(--text-secondary);cursor:pointer;font-size:12.5px;
  font-variant-numeric:tabular-nums;transition:background 120ms var(--ease),border-color 120ms var(--ease),color 120ms var(--ease),transform 80ms var(--ease),box-shadow 160ms var(--ease)}
.chip:hover{color:var(--text-primary);border-color:var(--border-active)}
.chip:active{transform:scale(.96)}
.chip.ack{box-shadow:0 0 0 2px rgba(110,231,183,.5)}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:28px;background:transparent;margin:10px 0 2px;cursor:pointer}
input[type=range]::-webkit-slider-runnable-track{height:6px;border-radius:999px;background:linear-gradient(90deg,var(--bg-hover),var(--bg-hover))}
input[type=range]::-moz-range-track{height:6px;border-radius:999px;background:var(--bg-hover)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;margin-top:-6px;border-radius:50%;background:var(--accent-primary);border:3px solid var(--bg-surface);box-shadow:0 0 0 1px var(--accent-primary)}
input[type=range]::-moz-range-thumb{width:14px;height:14px;border-radius:50%;background:var(--accent-primary);border:3px solid var(--bg-surface)}
input[type=range]:focus{outline:0}
input[type=range]:focus::-webkit-slider-thumb{box-shadow:0 0 0 1px var(--accent-primary),0 0 0 5px var(--accent-primary-dim)}
.ticks{display:flex;justify-content:space-between;font-size:11px;color:var(--text-muted);padding:0 4px;font-variant-numeric:tabular-nums}
.slv{font-size:22px;font-weight:650;min-width:78px;font-variant-numeric:tabular-nums}

/* ---------- log ---------- */
.lhead{display:flex;align-items:center;gap:10px;cursor:pointer;user-select:none}
.chev{width:16px;height:16px;color:var(--text-secondary);transition:transform 180ms var(--ease);flex:none}
.lopen .chev{transform:rotate(90deg)}
#logPrev{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font:12px ui-monospace,SFMono-Regular,Menlo,monospace;color:var(--text-muted)}
.lopen #logPrev{visibility:hidden}
#logwrap{display:grid;grid-template-rows:0fr;transition:grid-template-rows 220ms var(--ease)}
.lopen #logwrap{grid-template-rows:1fr}
#logwrap>div{overflow:hidden}
#log{height:220px;margin-top:12px;overflow-y:auto;background:var(--bg-base);border:1px solid var(--border-subtle);border-radius:var(--radius-md);padding:8px 12px;
  font:12px/1.65 ui-monospace,SFMono-Regular,Menlo,monospace;color:var(--text-secondary);white-space:pre-wrap;word-break:break-word}
#log .t{color:var(--text-muted);margin-right:12px;font-variant-numeric:tabular-nums}
#log .err{color:var(--accent-danger)}
#log .sep{color:var(--text-muted);font-style:italic;margin:4px 0}

/* ---------- wi-fi ---------- */
.netinfo{display:flex;align-items:center;gap:10px;flex-wrap:wrap;font-size:13px;color:var(--text-secondary);font-variant-numeric:tabular-nums}
.netinfo b{color:var(--text-primary);font-weight:600}
#netlist{margin-top:12px;max-height:240px;overflow-y:auto;border:1px solid var(--border-subtle);border-radius:var(--radius-md);background:var(--bg-base)}
.net{display:flex;align-items:center;gap:10px;width:100%;height:42px;padding:0 12px;border:0;border-bottom:1px solid var(--border-subtle);background:transparent;cursor:pointer;text-align:left;
  transition:background 120ms var(--ease)}
.net:last-child{border-bottom:0}
.net:hover{background:var(--bg-elevated)}
.net.sel{background:var(--accent-primary-dim);box-shadow:inset 3px 0 0 var(--accent-primary)}
.net .nm{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:flex;align-items:center;gap:8px}
.net .db{width:58px;text-align:right;font-size:12px;color:var(--text-muted);font-variant-numeric:tabular-nums}
.fgrid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:14px}
@media(max-width:600px){.fgrid{grid-template-columns:1fr}}
.copy{display:inline-flex;align-items:center;gap:8px;height:30px;padding:0 10px;border-radius:var(--radius-sm);border:1px dashed var(--border-active);background:transparent;
  color:var(--text-primary);cursor:pointer;font:12.5px ui-monospace,SFMono-Regular,Menlo,monospace}
.copy:hover{border-color:var(--text-secondary)}

/* ---------- toasts ---------- */
#toasts{position:fixed;right:18px;bottom:18px;z-index:50;display:flex;flex-direction:column;gap:8px;pointer-events:none}
.toast{display:flex;align-items:center;gap:10px;min-width:220px;max-width:380px;padding:10px 14px;border-radius:var(--radius-md);background:var(--bg-elevated);
  border:1px solid var(--border-active);box-shadow:0 12px 30px rgba(0,0,0,.5);font-size:13px;color:var(--text-primary);
  animation:tin 180ms var(--ease);transition:opacity 180ms var(--ease),transform 180ms var(--ease)}
.toast::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--c);flex:none}
.toast.out{opacity:0;transform:translateY(6px)}
@keyframes tin{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:none}}

/* ---------- v3 ---------- */
.deck{display:grid;gap:14px}
.tdot.live{background:var(--accent-primary)}
.btn.jog{flex:1;height:58px;font-size:15px;touch-action:none;user-select:none;-webkit-user-select:none}
.btn.big{height:44px;font-size:15px}
.btn kbd.k,.kbd{font:600 10px/1 -apple-system,sans-serif;padding:3px 5px;border-radius:4px;border:1px solid currentColor;opacity:.7;margin-left:4px}
.btn.ena{gap:6px}
.btn.ena.locked{color:var(--accent-success);border-color:rgba(110,231,183,.35)}
.btn.ena.free{color:var(--accent-warning);border-color:rgba(252,211,77,.45);background:rgba(252,211,77,.08)}
.badge.lim{cursor:pointer;height:26px;padding:0 9px;color:var(--accent-danger);border-color:rgba(251,113,133,.35);background:rgba(251,113,133,.07);font-variant-numeric:tabular-nums}
.badge.lim:hover{border-color:var(--accent-danger)}
input[type=range]::-webkit-slider-runnable-track{background:var(--trk,var(--bg-hover))}
input[type=range]::-moz-range-track{background:var(--trk,var(--bg-hover))}
.chip:disabled{opacity:.3;cursor:not-allowed}
.tfields{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:14px}
.tfields:empty{display:none}
.wphead,.wprow{display:grid;grid-template-columns:1.1fr 1fr 1fr 30px;gap:8px;align-items:center}
.wphead{margin-top:14px;margin-bottom:6px;font-size:12px;color:var(--text-secondary)}
.wprow+.wprow{margin-top:8px}
.wprow .field{height:32px}
.tres{max-height:220px;overflow-y:auto;border:1px solid var(--border-subtle);border-radius:var(--radius-md);background:var(--bg-base)}
.res{display:flex;gap:12px;align-items:baseline;padding:8px 12px;border-bottom:1px solid var(--border-subtle);font:12px/1.5 ui-monospace,SFMono-Regular,Menlo,monospace;color:var(--text-primary);font-variant-numeric:tabular-nums}
.res:last-child{border-bottom:0}
.res .n{color:var(--accent-danger);font-weight:700;min-width:26px}
.res .m{color:var(--text-secondary)}
.srow{display:flex;align-items:center;justify-content:space-between;gap:12px}
hr.sep{border:0;border-top:1px solid var(--border-subtle);margin:16px 0}

/* ---------- ligações ---------- */
.wgrid{display:grid;grid-template-columns:minmax(0,1fr) 340px;gap:14px;align-items:start}
@media(max-width:1100px){.wgrid{grid-template-columns:1fr}}
.wdiag{position:relative;border-radius:var(--radius-md);background:radial-gradient(ellipse at 50% 40%,#12121A 0%,#0A0A0D 75%);border:1px solid var(--border-subtle);transition:opacity .12s var(--ease)}
.wdiag.fade{opacity:0}
.wsvg{width:100%;height:auto;display:block}
.wsvg text{font-family:-apple-system,BlinkMacSystemFont,"Inter",sans-serif}
.wsvg .pin{font-size:11px;fill:#5E5E70}.wsvg .pin.on{fill:#D4D4DC;font-weight:600}.wsvg .small{font-size:9.5px}
.wsvg .bname{font-size:14px;font-weight:650;fill:#F4F4F6}.wsvg .big{font-size:20px;letter-spacing:.02em}
.wsvg .chipt{font-size:13px;font-weight:700;fill:#C9CCD6}.wsvg .chips{font-size:10.5px;fill:#7A7A8C}
.wsvg .dipt{font-size:11px;font-weight:700;fill:#FDE2E2}.wsvg .dipt.sm{font-size:9px;font-weight:600;fill:#FCA5A5}
.wsvg .dip{transition:y .25s var(--ease)}
.wire{cursor:pointer;transition:opacity .2s var(--ease)}
.wire .halo{fill:none;stroke:#09090B;stroke-width:8;stroke-linecap:round;pointer-events:stroke}
.wire .core{fill:none;stroke-width:3;stroke-linecap:round;transition:stroke-width .15s var(--ease)}
.wire .flow{fill:none;stroke:rgba(255,255,255,.8);stroke-width:1.6;stroke-linecap:round;stroke-dasharray:1.5 15;animation:flow 1.3s linear infinite;pointer-events:none}
@keyframes flow{to{stroke-dashoffset:-33}}
.wire.off .core{stroke-dasharray:5 6;opacity:.45}
.dim .wire{opacity:.15}.dim .wire.hl{opacity:1}.wire.hl .core{stroke-width:4.5}
.wtip{position:absolute;pointer-events:none;z-index:5;padding:6px 10px;border-radius:var(--radius-sm);background:var(--bg-elevated);border:1px solid var(--border-active);font-size:12.5px;white-space:nowrap;box-shadow:0 8px 24px rgba(0,0,0,.5)}
.wtable{display:grid;grid-template-columns:1fr 1fr;gap:2px 18px;margin-top:14px}
@media(max-width:800px){.wtable{grid-template-columns:1fr}}
.wrow{display:grid;grid-template-columns:10px 110px 1fr 14px 1fr;gap:8px;align-items:center;padding:6px 0;border-bottom:1px solid var(--border-subtle);font-size:12.5px;color:var(--text-secondary)}
.wrow i{width:10px;height:10px;border-radius:3px}
.wrow b{color:var(--text-primary);font-weight:600}
.wrow .arr{color:var(--text-muted)}
.diprow{display:flex;gap:6px;margin-top:14px;padding:12px;border-radius:var(--radius-md);background:#2A0E10;border:1px solid #5B1A1E}
.dsw{flex:1;display:flex;flex-direction:column;align-items:center;gap:4px}
.dsw span{position:relative;width:20px;height:40px;border-radius:4px;background:#5B1418}
.dsw span::after{content:"";position:absolute;left:3px;top:3px;width:14px;height:16px;border-radius:2px;background:#F4F4F6;transition:top .25s var(--ease)}
.dsw.on span::after{top:21px}
.dsw b{font-size:11px;color:#FDE2E2}.dsw em{font-style:normal;font-size:10px;color:#FCA5A5}

.tnow{margin-bottom:12px;padding:12px 14px;border-radius:var(--radius-md);background:var(--bg-base);border:1px solid var(--border-subtle)}
.tval{font-size:26px;font-weight:650;letter-spacing:-.01em;font-variant-numeric:tabular-nums;margin-top:2px}
.tlive{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:10px;font-variant-numeric:tabular-nums}
.tlive div{display:flex;flex-direction:column;padding:8px 10px;border-radius:var(--radius-sm);background:var(--bg-surface);border:1px solid var(--border-subtle);min-width:0}
.tlive span{font-size:11px;color:var(--text-muted);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.tlive b{font-size:17px;font-weight:650;color:var(--accent-cyan);margin-top:2px}
.tlive b.acc{color:#C4B5FD}
.tlive em{font-style:normal;font-size:11px;color:var(--text-secondary)}
.tbarw{height:5px;border-radius:999px;background:var(--bg-hover);margin-top:10px;overflow:hidden}
.tbar{height:100%;width:0;border-radius:999px;background:var(--accent-warning);transition:width .3s var(--ease)}

.btn.running{background:var(--accent-warning);border-color:var(--accent-warning);color:#1A1405;font-weight:650;animation:runglow 1.6s ease-in-out infinite}
.btn.running:hover{filter:brightness(1.05)}
.btn .rdot{width:9px;height:9px;border-radius:50%;background:#1A1405;animation:rdot 1s ease-in-out infinite}
@keyframes rdot{50%{opacity:.25}}
@keyframes runglow{0%,100%{box-shadow:0 0 0 0 rgba(252,211,77,.0)}50%{box-shadow:0 0 0 4px rgba(252,211,77,.22)}}
.btn kbd.k.dark{border-color:rgba(26,20,5,.5)}

.leg.ghost{color:var(--text-secondary)}.leg.ghost b{color:var(--text-primary);font-weight:600;max-width:240px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.gx{border:0;background:transparent;color:var(--text-muted);cursor:pointer;font-size:14px;padding:0 2px}.gx:hover{color:var(--text-primary)}
.runs{max-height:260px;overflow-y:auto;border:1px solid var(--border-subtle);border-radius:var(--radius-md);background:var(--bg-base)}
.run{display:flex;align-items:center;gap:6px;padding:8px 10px;border-bottom:1px solid var(--border-subtle)}
.run:last-child{border-bottom:0}
.run.on{background:rgba(152,152,168,.08);box-shadow:inset 3px 0 0 #9898A8}
.run .ri{flex:1;min-width:0;font-size:12.5px}.run .ri>span{color:var(--text-secondary)}
.run .ri b{font-variant-numeric:tabular-nums}
.run .btn.sm{height:26px;padding:0 8px;font-size:12px}.run .cmp{min-width:86px}

.tinfo{display:grid;gap:8px;margin-top:12px;padding:12px;border-radius:var(--radius-md);background:var(--bg-base);border:1px solid var(--border-subtle)}
.tinfo div{display:grid;grid-template-columns:64px 1fr;gap:10px;font-size:12.5px;line-height:1.5;color:var(--text-secondary)}
.tinfo b{font-size:11px;font-weight:650;letter-spacing:.05em;text-transform:uppercase;color:var(--text-muted);padding-top:2px}
.tinfo div:nth-child(2) b{color:var(--accent-primary)}
.tinfo div:nth-child(2) span{color:var(--text-primary)}
</style></head><body>
<header class="top">
  <div class="brand">nemabot <span class="badge pri" id="rbadge">J2</span></div>
  <nav class="tabs">
    <button class="tab active" data-tab="ctl">Controle</button>
    <button class="tab" data-tab="tst">Testes<span class="tdot live" id="tstdot"></span></button>
    <button class="tab" data-tab="wire">Ligações</button>
    <button class="tab" data-tab="sys">Sistema<span class="tdot" id="sysdot"></span></button>
  </nav>
  <div class="hright">
    <button class="badge lim" id="limBadge" hidden title="Batentes virtuais ativos · clique para configurar">
      <svg width="12" height="12" viewBox="0 0 12 12" fill="none" stroke="currentColor" stroke-width="1.6"><path d="M1.5 1v10M10.5 1v10M4 6h4"/></svg><span id="limTxt">−120°…120°</span></button>
    <button class="btn sm ena" id="enaBtn" hidden></button>
    <div class="pill" id="pill"><span class="pdot" id="dot"></span><span class="txt" id="conn">Conectando…</span></div>
    <button class="stop" id="stop" title="Parar com rampa (Esc ou espaço)">
      <svg width="12" height="12" viewBox="0 0 12 12"><rect width="12" height="12" rx="2" fill="currentColor"/></svg>PARAR<kbd>ESC</kbd>
    </button>
  </div>
</header>

<!-- ================= CONTROLE / TESTES (mesma coluna direita) ================= -->
<main id="tab-main"><div class="grid">
<div class="col">

 <!-- ---------- deck Controle ---------- -->
 <div class="deck" id="deck-ctl">
  <section class="card">
    <div class="ch"><span class="ct">Telemetria</span>
      <button class="btn sm" id="zero" title="Define a posição atual como 0°">
        <svg width="14" height="14" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.6"><circle cx="8" cy="8" r="5"/><path d="M8 1v3M8 12v3M1 8h3M12 8h3"/></svg>Zerar 0°</button>
    </div>
    <div class="tele">
      <svg id="dial" viewBox="-100 -100 200 200" width="148" height="148">
        <circle r="97" fill="#09090B" stroke="#24242F" stroke-width="1.5"/>
        <circle r="90" fill="#0E0E12"/>
        <g id="dticks"></g>
        <path id="dlim" fill="none" stroke="#2A0C14" stroke-opacity=".9" stroke-width="18" d=""/>
        <path id="dlimE" fill="none" stroke="#FB7185" stroke-width="2" stroke-linecap="round" d=""/>
        <circle r="78" fill="none" stroke="#1E1E28" stroke-width="6"/>
        <path id="dfull" fill="none" stroke="#93A5FF" stroke-opacity=".22" stroke-width="6" d=""/>
        <path id="darc" fill="none" stroke="#93A5FF" stroke-opacity=".7" stroke-width="6" stroke-linecap="round" d=""/>
        <text id="dturn" y="38" text-anchor="middle" font-size="12" fill="#9898A8"></text>
        <g id="dtgt" class="rot"><path d="M0,-80 L-6,-92 L6,-92 Z" fill="#FCD34D" stroke="#09090B" stroke-width="1.5"/></g>
        <g id="dneedle" class="rot"><path d="M-4.5,14 L4.5,14 L1.4,-72 L-1.4,-72 Z" fill="#93A5FF"/><circle cy="-72" r="1.4" fill="#93A5FF"/></g>
        <circle r="9" fill="#F4F4F6"/><circle r="3.5" fill="#111115"/>
      </svg>
      <div style="min-width:0">
        <div class="read"><span id="pos">0.0</span><small>°</small></div>
        <dl class="kv">
          <dt><span class="sw-tri"></span>Alvo</dt><dd id="tgt">0.0°</dd>
          <dt><span class="sw-line" style="background:#67E8F9"></span>Velocidade</dt><dd id="spd">0.0 rpm</dd>
          <dt><span class="sw-line" style="background:#C4B5FD"></span>Aceleração</dt><dd id="accNow">0 p/s²</dd>
        </dl>
        <div class="moderow"><span class="badge" id="mode">parado</span></div>
      </div>
    </div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Movimento</span>
      <div class="seg" id="mseg"><button class="active" data-m="abs">Absoluto</button><button data-m="rel">Relativo</button><button data-m="jog">Jog</button></div>
    </div>
    <div id="m-abs" class="row" style="flex-wrap:nowrap">
      <label class="field" style="flex:1"><input type="number" id="ga" value="90" step="any"><span class="unit">°</span></label>
      <button class="btn pri" id="goto" style="min-width:84px">Ir</button>
    </div>
    <div id="m-rel" hidden>
      <label class="field"><input type="number" id="ra" value="45" step="any"><span class="unit">° por clique</span></label>
      <div class="row" style="flex-wrap:nowrap;margin-top:10px">
        <button class="btn" id="rm" style="flex:1">− Girar</button>
        <button class="btn" id="rp" style="flex:1">+ Girar</button>
      </div>
    </div>
    <div id="m-jog" hidden>
      <div class="row" style="flex-wrap:nowrap">
        <button class="btn jog" id="jogL">◀ Segurar</button>
        <button class="btn jog" id="jogR">Segurar ▶</button>
      </div>
      <div class="row" style="justify-content:space-between;margin-top:10px">
        <span class="sub">Escala da velocidade</span>
        <div class="seg" id="jseg"><button data-s="0.2">Fino 20%</button><button class="active" data-s="1">Normal 100%</button></div>
      </div>
    </div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Parâmetros</span></div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px">
      <div><span class="lbl">Velocidade <span class="sub" id="rpmMax">máx. 30</span></span>
        <label class="field" id="f-rpm"><input type="number" id="rpm" value="5" step="0.5" min="0.1"><span class="unit" id="rpmUnit">rpm</span></label></div>
      <div><span class="lbl">Aceleração</span>
        <label class="field" id="f-acc"><input type="number" id="acc" value="4000" step="500" min="100"><span class="unit">p/s²</span></label></div>
    </div>
    <div class="row" style="justify-content:space-between;margin-top:12px">
      <span class="lbl" style="margin:0;display:flex;align-items:center;gap:6px">Rampa <span class="q" tabindex="0" data-tip="Trapezoidal: acelera e freia com aceleração constante.&#10;Curva S: a aceleração cresce e diminui aos poucos (limitada pelo jerk). Mais suave, um pouco mais lenta.">?</span></span>
      <div class="seg" id="rseg"><button class="active" data-r="0">Trapezoidal</button><button data-r="1">Curva S</button></div>
    </div>
    <div id="jerkRow" hidden style="margin-top:10px">
      <label class="field" id="f-jerk"><input type="number" id="jerk" value="40000" step="1000" min="1000"><span class="unit">jerk p/s³</span></label>
    </div>
  </section>
 </div>

 <!-- ---------- deck Testes ---------- -->
 <div class="deck" id="deck-tst" hidden>
  <section class="card">
    <div class="ch"><span class="ct">Rotina</span></div>
    <label class="field sel"><select id="tsel">
      <option value="loop">Endurance (loop)</option>
      <option value="sweep">Limite de rpm</option>
      <option value="accsweep">Limite de aceleração</option>
      <option value="resonance">Ressonância</option>
      <option value="backlash">Folga (backlash)</option>
      <option value="drift">Repetibilidade</option>
      <option value="stiction">Atrito estático</option>
      <option value="waypoints">Trajetória (waypoints)</option>
    </select></label>
    <div class="tinfo" id="tdesc"></div>
    <div class="tfields" id="tfields"></div>
    <div id="wpEd" hidden>
      <div class="wphead"><span>Ângulo</span><span>rpm</span><span>Pausa</span><span></span></div>
      <div id="wpRows"></div>
      <div class="row" style="justify-content:space-between;margin-top:10px">
        <button class="btn sm" id="wpAdd">+ Adicionar ponto</button>
        <label class="switch"><input type="checkbox" id="wpLoop"><span class="trk"></span>Repetir</label>
      </div>
    </div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Execução</span><span class="sub" id="tprog"></span></div>
    <div class="tnow" id="tnow" hidden>
      <div class="sub" id="tstep">Degrau 1 de 10</div>
      <div class="tval" id="tval">1.000 p/s²</div>
      <div class="tlive">
        <div><span>velocidade</span><b id="tvNow">0,0</b><em>rpm</em></div>
        <div><span title="pico de velocidade neste degrau">pico</span><b id="tvPk">0,0</b><em>rpm</em></div>
        <div><span>aceleração</span><b class="acc" id="taNow">0</b><em>p/s²</em></div>
      </div>
      <div class="tbarw" id="tbarw"><div class="tbar" id="tbar"></div></div>
    </div>
    <button class="btn pri block big" id="tgo" title="Atalho: I">Iniciar teste<kbd class="k">I</kbd></button>
    <button class="btn danger block" id="tmark" style="margin-top:10px" disabled title="Para o motor, congela o gráfico e registra o ponto">
      <svg width="14" height="14" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M3 2v12M3 3h8l-2 3 2 3H3"/></svg>Marcar falha<kbd class="k">M</kbd></button>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Gravações <span class="q" tabindex="0" data-tip="Cada teste fica gravado. Ao iniciar um teste, a execução anterior da mesma rotina aparece em cinza por baixo do gráfico, alinhada pelo início, com as falhas marcadas nela. Use Comparar para escolher outra.">?</span></span></div>
    <div id="runs" class="runs"></div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Marcações</span>
      <div class="row" style="gap:6px"><button class="btn sm" id="rcopy">Copiar</button><button class="btn sm" id="rclear">Limpar</button></div></div>
    <div id="tres" class="tres"><div class="sub" style="padding:12px">Nenhuma marcação ainda. Durante um teste, aperte M quando o motor falhar.</div></div>
  </section>
 </div>

</div>
<div class="col">

  <section class="card" id="chart">
    <div class="ch"><span class="ct">Movimento</span>
      <div class="row" style="gap:6px">
        <label class="field sel" style="height:30px;width:86px"><select id="win"><option value="5">5 s</option><option value="10" selected>10 s</option><option value="30">30 s</option><option value="60">60 s</option></select></label>
        <button class="btn sm" id="pause">Pausar</button>
        <button class="btn sm" id="reset" title="Enquadra o último teste inteiro">Ajustar</button>
        <button class="btn sm" id="clear">Limpar</button>
        <button class="btn sm" id="csv">CSV</button>
        <button class="btn sm icon" id="help" title="Atalhos do gráfico">?</button>
      </div>
    </div>
    <div class="pop" id="pop" hidden>
      <div><b>Arrastar retângulo</b>ampliar a região</div>
      <div><b>Rodinha / pinça</b>tempo</div>
      <div><b>Shift + rodinha</b>altura (ou rodinha no eixo)</div>
      <div><b>Dois dedos pro lado</b>andar no tempo</div>
      <div><b>Duplo clique / Ajustar</b>enquadra o último teste inteiro</div>
      <div><b>Continuar</b>volta ao tempo real</div>
      <div><b>I</b>iniciar / parar o teste (aba Testes)</div>
      <div><b>M</b>marcar falha (durante um teste)</div>
    </div>
    <div class="ctitle"><span id="ptitle">Posição (graus na saída)</span>
      <span class="leg"><i style="border-top:2px solid #93A5FF"></i>posição</span>
      <span class="leg"><i style="border-top:2px dashed #FCD34D"></i>alvo</span>
      <span class="leg ghost" id="ghostLeg" hidden><i style="border-top:2px solid rgba(152,152,168,.6)"></i>comparando com <b id="ghostName">anterior</b><button class="gx" id="ghostX" title="Parar de comparar">×</button></span></div>
    <canvas id="cp" style="height:340px"></canvas>
    <div class="ctitle" style="margin-top:8px" id="vtitle">Velocidade (rpm na saída)</div>
    <canvas id="cv" style="height:170px"></canvas>
    <div id="tip"></div>
  </section>

  <section class="card">
    <div class="row" style="justify-content:space-between;flex-wrap:nowrap;gap:12px">
      <span class="slv"><span id="slv">0</span>°</span>
      <div class="chips">
        <button class="chip" data-g="-180">-180°</button><button class="chip" data-g="-90">-90°</button><button class="chip" data-g="-45">-45°</button>
        <button class="chip" data-g="0">0°</button><button class="chip" data-g="45">45°</button><button class="chip" data-g="90">90°</button><button class="chip" data-g="180">180°</button>
      </div>
      <label class="switch" title="Desligado: o motor só vai quando você solta a barra"><input type="checkbox" id="live" checked><span class="trk"></span>Ao vivo</label>
    </div>
    <input type="range" id="sl" min="-360" max="360" step="1" value="0">
    <div class="ticks"><span>-360</span><span>-270</span><span>-180</span><span>-90</span><span>0</span><span>90</span><span>180</span><span>270</span><span>360</span></div>
  </section>

  <section class="card" id="logcard">
    <div class="lhead" id="lhead">
      <svg class="chev" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M6 3l5 5-5 5"/></svg>
      <span class="ct">Log</span>
      <span id="logPrev"></span>
      <button class="btn sm" id="lcopy">Copiar</button>
      <button class="btn sm" id="lclear">Limpar</button>
    </div>
    <div id="logwrap"><div><div id="log"></div></div></div>
  </section>

</div>
</div></main>

<!-- ================= LIGAÇÕES ================= -->
<main id="tab-wire" hidden><div class="wgrid">
  <section class="card">
    <div class="ch"><span class="ct">Ligações · <span id="wname">TB6600</span></span>
      <div class="seg" id="dseg"><button data-d="tb6600">TB6600</button><button data-d="tmc2209">TMC2209</button></div></div>
    <div class="wdiag" id="wdiag"></div><div class="wtip" id="wtip" hidden></div>
    <div class="wtable" id="wtable"></div>
  </section>
  <div class="col">
    <section class="card" id="cfg-tb">
      <div class="ch"><span class="ct">Chaves DIP</span><span class="sub">ON é para baixo neste driver</span></div>
      <div class="fgrid" style="margin-top:0;grid-template-columns:1fr">
        <div><span class="lbl">Micropasso</span><label class="field sel"><select id="tbMs"></select></label></div>
        <div><span class="lbl">Corrente</span><label class="field sel"><select id="tbCur"></select></label></div>
      </div>
      <div class="diprow" id="dipRow"></div>
      <div class="sub" id="tbInfo" style="margin-top:10px;line-height:1.5"></div>
      <div class="sub" style="margin-top:6px">17HS4401 (1,7 A): use <b style="color:var(--text-secondary)">1,5 A</b>, que dá pico de 1,7 A. As cores das bobinas são as do seu cabo na foto; confira com um multímetro (cada par tem poucos ohms entre si).</div>
    </section>
    <section class="card" id="cfg-tmc" hidden>
      <div class="ch"><span class="ct">TMC2209</span><span class="sub">MS1/MS2 e Vref</span></div>
      <div class="fgrid" style="margin-top:0;grid-template-columns:1fr">
        <div><span class="lbl">Micropasso <span class="sub">interpola para 1/256</span></span><label class="field sel"><select id="tmcMs"><option value="8">1/8</option><option value="16">1/16</option><option value="32">1/32</option><option value="64">1/64</option></select></label></div>
        <div><span class="lbl">Corrente RMS</span><label class="field"><input type="number" id="irms" step="0.05" min="0.1" max="2"><span class="unit">A</span></label></div>
      </div>
      <div class="sub" id="tmcInfo" style="margin-top:10px;line-height:1.6"></div>
      <div class="sub" style="margin-top:6px">Vref pela fórmula da BTT v1.3 (Vref = I × 1,41). Confira no manual da sua placa antes de ajustar o trimpot.</div>
    </section>
    <section class="card">
      <div class="ch"><span class="ct">Micropasso no ESP32</span></div>
      <div class="row" id="msSync"></div>
      <div class="sub" style="margin-top:10px;line-height:1.5">Os sinais do ESP32 são de 3,3 V. A maioria dos TB6600 aceita; se ele perder passos sem carga, use um conversor de nível para 5 V. Ligue sempre o GND da fonte, do driver e do ESP32 juntos.</div>
    </section>
  </div>
</div></main>

<!-- ================= SISTEMA ================= -->
<main id="tab-sys" hidden><div class="sys">

  <section class="card">
    <div class="ch"><span class="ct">Wi-Fi</span><span class="badge" id="netBadge">…</span></div>
    <div class="netinfo" id="netinfo">…</div>
    <div class="row" style="margin-top:14px"><button class="btn" id="scan">
      <svg width="15" height="15" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.6"><circle cx="7" cy="7" r="4.5"/><path d="M10.5 10.5L14 14"/></svg>Procurar redes</button></div>
    <div id="netlist" hidden></div>
    <div class="fgrid">
      <div><span class="lbl">Rede</span><label class="field"><input type="text" id="ssid" placeholder="nome da rede" autocomplete="off" spellcheck="false"></label></div>
      <div><span class="lbl">Senha</span><div class="field"><input type="password" id="pass" placeholder="senha" autocomplete="off"><button class="eye" type="button" data-for="pass" title="Mostrar senha"></button></div></div>
    </div>
    <div class="row" style="margin-top:14px">
      <button class="btn pri" id="save">Salvar e reiniciar</button>
      <button class="btn danger" id="forget">Esquecer rede</button>
    </div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Motor</span><span class="badge" id="mcur">…</span></div>
    <div class="fgrid" style="margin-top:0">
      <div><span class="lbl" style="display:flex;align-items:center;gap:6px">Micropasso <span class="q" tabindex="0" data-tip="Deve coincidir com as chaves físicas S1–S3 do driver: este campo não muda o driver, só informa ao ESP32.&#10;Ao aplicar, o motor para e a posição volta a 0°.">?</span></span>
        <label class="field sel"><select id="mms"><option value="1">Passo cheio</option><option value="2">1/2</option><option value="4">1/4</option><option value="8">1/8</option><option value="16">1/16</option><option value="32">1/32</option><option value="64">1/64</option><option value="128">1/128</option><option value="256">1/256</option></select></label></div>
      <div><span class="lbl">Redução <span class="sub">1 = motor solto</span></span>
        <label class="field"><input type="number" id="mratio" value="20" step="any" min="0.1"><span class="unit">:1</span></label></div>
    </div>
    <div class="row" style="margin-top:14px"><button class="btn pri" id="msave">Aplicar</button></div>
  </section>

  <section class="card" id="safety">
    <div class="ch"><span class="ct">Segurança e hardware</span></div>
    <div class="srow"><div><div>Batentes virtuais</div><div class="sub">o motor nunca passa destes ângulos</div></div>
      <label class="switch"><input type="checkbox" id="limEn"><span class="trk"></span>Ativar</label></div>
    <div class="fgrid" style="margin-top:10px">
      <div><span class="lbl">Mínimo</span><label class="field"><input type="number" id="limMin" value="-120" step="any"><span class="unit">°</span></label></div>
      <div><span class="lbl">Máximo</span><label class="field"><input type="number" id="limMax" value="120" step="any"><span class="unit">°</span></label></div>
    </div>
    <hr class="sep">
    <div class="srow"><div><div style="display:flex;align-items:center;gap:6px">Controle de torque (ENA) <span class="q" tabindex="0" data-tip="TB6600: ENA+ no GPIO 27 e ENA− no GND.&#10;Com a lógica invertida (padrão), LOW = motor com torque e HIGH = solto.&#10;Soltar o torque sempre para o motor com rampa antes.">?</span></div><div class="sub">pino GPIO 27</div></div>
      <label class="switch"><input type="checkbox" id="enaUse"><span class="trk"></span>Usar</label></div>
    <div class="srow" style="margin-top:10px"><div class="sub">Lógica invertida (ativo em LOW)</div>
      <label class="switch"><input type="checkbox" id="enaInv" checked><span class="trk"></span></label></div>
    <div class="row" style="margin-top:14px"><button class="btn pri" id="safeSave">Salvar</button></div>
  </section>

  <section class="card">
    <div class="ch"><span class="ct">Atualização pelo Wi-Fi <span class="q" tabindex="0" data-tip="No Arduino IDE: Ferramentas → Porta → “nemabot at …” (porta de rede) e Upload normal. O IDE pede esta senha.&#10;Partition Scheme precisa ter OTA (o padrão serve).&#10;Na primeira vez o macOS pergunta se permite conexões: aceite.">?</span></span><span class="badge" id="otaBadge">…</span></div>
    <div class="row"><span class="sub">Endereço</span><button class="copy" id="otaAddr" title="Copiar">nemabot.local</button></div>
    <div class="row" style="margin-top:14px;flex-wrap:nowrap">
      <div class="field" style="flex:1"><input type="password" id="otap" placeholder="nova senha do OTA (mín. 6)" autocomplete="new-password"><button class="eye" type="button" data-for="otap" title="Mostrar senha"></button></div>
      <button class="btn" id="otas">Salvar e reiniciar</button>
    </div>
  </section>

</div></main>

<div id="toasts"></div>
<script>
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
const MOVES=['goto','move','loop','sweep','accsweep','resonance','backlash','drift','stiction','waypoints','jog'];
async function send(c,v,btn,p2,p3,p4){
  if(MOVES.includes(c))resumeLive();       // movimento novo: gráfico volta ao tempo real
  let u='/cmd?c='+c+'&v='+(v===undefined||v===''?0:v);
  [p2,p3,p4].forEach((p,i)=>{if(p!==undefined&&p!=='')u+='&p'+(i+2)+'='+p;});
  const j=await api(u);
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
    if(n&&t/1000<G.t[n-1]-1){for(const k in G)G[k].length=0;G.marks.length=0;G.steps.length=0;}   // ESP32 reiniciou
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
async function poll(){
  try{
    const ac=new AbortController(),to=setTimeout(()=>ac.abort(),1500);
    const s=await (await fetch('/status?since='+seq+'&lsince='+lseq,{signal:ac.signal,cache:'no-store'})).json();
    clearTimeout(to);
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
    else setPill('ok','Conectado','Conectado · '+up);
  }catch(e){setPill('err','Offline','Sem resposta do ESP32');}
  setTimeout(poll,100);
}
poll();
</script></body></html>
)rawliteral";
