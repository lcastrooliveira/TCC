/*
  Detect and Turn from Obstacle.c

  Detect obstacles in the ActivityBot's path, and turn a random direction to avoid them.

  http://learn.parallax.com/activitybot/roaming-ultrasound
*/

#include "simpletools.h"                      // Include simpletools header
#include "abdrive.h"                          // Include abdrive header
#include "ping.h"                             // Include ping header
#include "deccor.h"

#define NORTE 13
#define OESTE 37
#define LESTE 59
#define SUL 29

int turn;                                     // Navigation variable
int left_turn_pause = 5000;
int de,para;

const int relogio_sentidos[4] = {NORTE,OESTE,SUL,OESTE};
int ponteiro_relogio = 0;
int main()                                    // main function
{
  int DO = 22, CLK = 23, DI = 24, CS = 25;
  sd_mount(DO, CLK, DI, CS);
  remove("QUADRADO.TXT");
  int SPEED = 8;
  int MIN_DIST = 7;
  //Define a direcao Inicial
  de = relogio_sentidos[ponteiro_relogio];
  drive_setRampStep(SPEED);                      // 10 ticks/sec / 20 ms
  drive_setMaxSpeed(SPEED);
  pause(5000);
  cog_run(startLeitura,128);
  pause(500);
  while(ping_cm(11) >= 3) {
    drive_ramp(SPEED, SPEED);                       // Forward 2 RPS
    pause(100);
    while(ping_cm(11) >= MIN_DIST) pause(5);           // Wait until object in range
    atualizarSentido();
    para = relogio_sentidos[ponteiro_relogio];
    corrigirGiro(de,para);
    de = para;
    if(!avancar1Quadrado()) {
      break;
    }      
    atualizarSentido();
    para = relogio_sentidos[ponteiro_relogio];
    corrigirGiro(de,para);
    de = para;
  }
  drive_ramp(0, 0);
}

void atualizarSentido() {
  ponteiro_relogio = (ponteiro_relogio + 1) % 4;
}

void corrigirGiro(int sentido_origem, int sentido_destino) {
  drive_ramp(0, 0);
  if(sentido_origem == sentido_destino) return;
  int diferenca = sentido_origem - sentido_destino;
  switch(diferenca) {
    case -46: //N - > L
      virarDireita();
      break;
    case -24: //N - > O
      virarEsquerda();
      break;
    case -16: //N -> S
      darMeiaVolta();
      break;
    case 22: //L -> O
      darMeiaVolta();
      break;
    case 30: //L -> S
      virarDireita();
      break;
    case 46: //L -> N
      virarEsquerda();    
      break;
    case 24: //O -> N
      virarDireita();
      break;
    case 8: //O -> S
      virarEsquerda();
      break;
    case -22: //O -> L
      darMeiaVolta();
      break;
    case 16: //S -> N
      darMeiaVolta();
      break;
    case -30: //S -> L
      virarEsquerda();
      break;
    case -8: //S -> O
      virarDireita();
      break;
  }    
}    

void virarDireita() {
  drive_goto(26,-25);
}

void virarEsquerda() {
  drive_goto(-25,26);
}    

void darMeiaVolta() {
  drive_goto(-51,51);
}  