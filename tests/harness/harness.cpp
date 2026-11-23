#include "motion.h"
#include "log.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "esp_timer.h"
// internos de motion.cpp usados pelo teste
extern int lastDir; extern uint32_t nextStepUs; void applyMotorCfg();
#include <deque>
#include <vector>
SerialT Serial; EspT ESP; WiFiT WiFi; MDNST MDNS; OTAT ArduinoOTA;
uint64_t simUs=0; uint32_t millis(){return simUs/1000;} uint32_t micros(){return (uint32_t)simUs;}
size_t strlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){size_t c=l<n-1?l:n-1;memcpy(d,s,c);d[c]=0;}return l;}
struct Q{int sz;std::deque<std::vector<uint8_t>> q;};
QueueHandle_t xQueueCreate(int,int s){auto*q=new Q;q->sz=s;return q;}
BaseType_t xQueueSend(QueueHandle_t h,const void*p,int){auto*q=(Q*)h;q->q.emplace_back((const uint8_t*)p,(const uint8_t*)p+q->sz);return 1;}
BaseType_t xQueueReceive(QueueHandle_t h,void*p,int){auto*q=(Q*)h;if(q->q.empty())return 0;memcpy(p,q->q.front().data(),q->sz);q->q.pop_front();return 1;}
int uxQueueMessagesWaiting(QueueHandle_t h){return ((Q*)h)->q.size();}
void vTaskDelay(int){} void xTaskCreatePinnedToCore(void(*)(void*),const char*,int,void*,int,void*,int){}
int esp_timer_create(const esp_timer_create_args_t*,esp_timer_handle_t*){return 0;} int esp_timer_start_periodic(esp_timer_handle_t,uint64_t){return 0;} int64_t esp_timer_get_time(){return simUs;}

int fails=0; void ok(bool c,const char*m){printf("%s %s\n",c?"ok  ":"FALHOU",m);if(!c)fails++;}
void tick(){motionService();if(vel==0&&pos==target)simUs+=1000;else if((int32_t)(micros()-nextStepUs)<0)simUs=nextStepUs;}
void runFor(double s){uint64_t e=simUs+(uint64_t)(s*1e6);while(simUs<e)tick();}
bool runToIdle(double maxS){uint64_t e=simUs+(uint64_t)(maxS*1e6);while(simUs<e){tick();if(mode==M_IDLE&&vel==0&&pos==target&&uxQueueMessagesWaiting(cmdQueue)==0)return true;}return false;}
void reset(){pos=target=0;vel=0;mode=M_IDLE;rpm=5;acc=4000;rampMode=0;jerk=40000;limEn=false;enaUse=false;enaOn=true;MICROSTEP=8;RATIO=20;applyMotorCfg();lastDir=0;}
long gotoOver(float deg,double&t){long tg=degToSteps(deg);int dir=(tg>pos)-(tg<pos);sendCmd(C_GOTO,deg);uint64_t t0=simUs;long over=0;
  while(simUs-t0<60e6){tick();long o=dir*(pos-tg);if(o>over)over=o;if(mode==M_IDLE&&vel==0&&pos==target&&uxQueueMessagesWaiting(cmdQueue)==0)break;}t=(simUs-t0)/1e6;return over;}
int main(){
  cmdQueue=xQueueCreate(32,sizeof(Cmd));char m[200];double t;
  reset();{long o=gotoOver(90,t);snprintf(m,200,"trapezoidal 90° chega exato sem passar (%.2f s)",t);ok(pos==degToSteps(90)&&o<=0,m);}
  float combos[][4]={{5,4000,40000,90},{30,4000,40000,360},{30,20000,40000,45},{2,1000,5000,10},{30,4000,1e6,180},{15,8000,20000,-270}};
  for(auto&c:combos){reset();rpm=c[0];acc=c[1];rampMode=1;jerk=c[2];long o=gotoOver(c[3],t);snprintf(m,200,"curva S rpm%.0f acc%.0f jerk%.0f %.0f°: passou %ld passos (%.2f s)",c[0],c[1],c[2],c[3],o,t);ok(pos==degToSteps(c[3])&&o<=2,m);}
  reset();sendCmdN(C_LOOP,90,3,100,0);ok(runToIdle(120)&&cycles==3&&pos==0,"loop 3 ciclos termina em 0°");
  reset();sendCmdN(C_SWEEP,90,10,30,5);ok(runToIdle(300)&&pos==0&&fabsf(sweepRpm-35)<0.01f,"varredura rpm termina em 0°");
  reset();sendCmdN(C_ACC_SWEEP,45,1000,9000,5);ok(runToIdle(300)&&pos==0,"varredura de aceleração termina em 0°");
  reset();{sendCmdN(C_BACKLASH,10,500,0,0);bool hp=false,hm=false;uint64_t t0=simUs;char b[24];while(simUs-t0<60e6){tick();subModeStr(b,24);if(!strcmp(b,"hold+"))hp=true;if(!strcmp(b,"hold-"))hm=true;if(mode==M_IDLE&&vel==0&&pos==target)break;}ok(hp&&hm&&pos==0,"folga passa por hold+ e hold- e termina em 0°");}
  reset();sendCmdN(C_DRIFT,90,5,50,0);ok(runToIdle(120)&&pos==0&&cycles==5,"repetibilidade 5 ciclos termina em 0°");
  reset();sendCmd(C_GOTO,30);runToIdle(30);{long base=pos,mk=0;sendCmdN(C_STICTION,0.5,4,200,0);uint64_t t0=simUs;while(simUs-t0<60e6){tick();if(pos-base>mk)mk=pos-base;if(mode==M_IDLE&&vel==0&&pos==target&&simUs-t0>1000)break;}ok(pos==base&&mk==degToSteps(2),"atrito estático sobe 4×0,5° e volta");}
  reset();wpBuf[0]={45,10,100};wpBuf[1]={-30,3,100};wpBuf[2]={0,20,0};wpCount=3;wpLoop=false;sendCmd(C_WAYPOINTS,0);ok(runToIdle(120)&&pos==0,"trajetória de 3 pontos termina no último ponto");
  reset();{sendCmdN(C_RESONANCE,2,10,3,1);uint64_t t0=simUs;bool r=runToIdle(30);double d=(simUs-t0)/1e6;snprintf(m,200,"ressonância roda ~3 s e para (%.2f s)",d);ok(r&&d>3&&d<6,m);}
  reset();{rpm=10;uint64_t t0=simUs,lr=0;bool first=true;while(simUs-t0<1e6){if(first||simUs-lr>=150000){sendCmdN(C_JOG,1,1,0,0);lr=simUs;first=false;}tick();}
    while(mode==M_JOG)tick();double d=(simUs-lr)/1e3;snprintf(m,200,"jog para sozinho ~450 ms após a última renovação (%.0f ms)",d);ok(d>440&&d<470,m);runToIdle(10);}
  reset();{sendCmdN(C_LIMITS,1,-30,60,0);double tt;gotoOver(200,tt);ok(pos==degToSteps(60),"goto 200° com batente em 60° para em 60°");
    rpm=20;uint64_t t0=simUs,lr=0;while(simUs-t0<15e6){if(simUs-lr>=150000){sendCmdN(C_JOG,-1,1,0,0);lr=simUs;}tick();}ok(pos==degToSteps(-30),"jog com batente para em −30°");
    sendCmdN(C_RESONANCE,2,10,3,1);tick();ok(mode!=M_RESONANCE,"ressonância recusada com batentes");}
  reset();{sendCmdN(C_LIMITS,0,-120,120,1|2);tick();ok(enaUse&&enaOn,"ENA ligado com torque");sendCmd(C_GOTO,90);for(int i=0;i<3000;i++)tick();sendCmd(C_ENA,0);tick();ok(enaOn,"pedir soltar em movimento só solta depois de parar");
    runToIdle(30);tick();ok(!enaOn,"soltou o torque depois de parar");sendCmd(C_GOTO,0);tick();ok(enaOn,"movimento religa o torque");}
  reset();{sendCmdN(C_JOG,1,1,0,0);runFor(0.3);ok(stepsToDeg(shownTarget())==stepsToDeg(pos),"no jog o alvo mostrado é a própria posição");sendCmd(C_STOP,0);runToIdle(10);}
  printf(fails?"%d falhas\n":"tudo certo\n",fails);return fails;}
