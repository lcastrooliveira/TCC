/*
  Detect and Turn from Obstacle.c

  Detect obstacles in the ActivityBot's path, and turn a random direction to avoid them.

  http://learn.parallax.com/activitybot/roaming-ultrasound
*/

#include "simpletools.h"                      // Include simpletools header
#include "abdrive.h"                          // Include abdrive header
#include "ping.h"                             // Include ping header
#include "deccor.h"
#include "fdserial.h"

#define NORTE 13
#define OESTE 37
#define LESTE 59
#define SUL 29
short de,para, tam_coluna;
fdserial *xbee;
const int relogio_sentidos[4] = {LESTE,NORTE,OESTE,NORTE};
int ponteiro_relogio = 0;
int *cog_automato;

int main()                                    // main function
{
  int DO = 22, CLK = 23, DI = 24, CS = 25;
  sd_mount(DO, CLK, DI, CS);
  remove("QUADRADO.TXT");
  int SPEED = 16;
  int MIN_DIST = 7;
  //Define que a tarefa a ser executada como mapeamento
  tarefa = MAPEAMENTO;
  //Define a direcao Inicial
  de = relogio_sentidos[ponteiro_relogio];
  drive_setRampStep(SPEED);                      // 10 ticks/sec / 20 ms
  drive_setMaxSpeed(SPEED);
  pause(5000);
  cog_automato = cog_run(startLeitura,128);
  pause(500);
  while(ping_cm(11) >= 3) {
    drive_ramp(SPEED, SPEED);                       // Forward 2 RPS
    pause(100);
    while(ping_cm(11) >= MIN_DIST) pause(5);           // Wait until object in range
    atualizarSentido();
    if(tam_coluna == 0) tam_coluna = num_vertices;
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
  FILE* fp = fopen("dimen.txt", "a");          // Open a file for writing
  fwrite(&num_vertices, sizeof(num_vertices), 1, fp);      // Add contents to the file
  fwrite(&tam_coluna, sizeof(tam_coluna), 1, fp);      // Add contents to the file
  fclose(fp);
  mandarMapaGrid();
  receberCoordenadas();
}

void atualizarSentido() {
  ponteiro_relogio = (ponteiro_relogio + 1) % 4;
}

void corrigirGiro(short sentido_origem, short sentido_destino) {
  drive_ramp(0, 0);
  if(sentido_origem == sentido_destino) return;
  int diferenca = sentido_origem - sentido_destino;
  switch(diferenca) {
    case -46: //N - > L
      drive_goto(26,-25); //Virar a direita
      break;
    case -24: //N - > O
      drive_goto(-25,26); //Virar a esquerda
      break;
    case -16: //N -> S
      drive_goto(-51,51); //Dar meia volta
      break;
    case 22: //L -> O
      drive_goto(-51,51); //Dar meia volta
      break;
    case 30: //L -> S
      drive_goto(26,-25); //Virar a direita
      break;
    case 46: //L -> N
      drive_goto(-25,26); //Virar a esquerda    
      break;
    case 24: //O -> N
      drive_goto(26,-25); //Virar a direita
      break;
    case 8: //O -> S
      drive_goto(-25,26); //Virar a esquerda
      break;
    case -22: //O -> L
      drive_goto(-51,51); //Dar meia volta
      break;
    case 16: //S -> N
      drive_goto(-51,51); //Dar meia volta
      break;
    case -30: //S -> L
      drive_goto(-25,26); //Virar a esquerda
      break;
    case -8: //S -> O
      drive_goto(26,-25); //Virar a direita
      break;
  }    
}

void mandarMapaGrid() {
  //para a thread do automato para liberar recursos
  cog_end(cog_automato);
  unsigned char grid[num_vertices+2];
  FILE* fp = fopen("QUADRADO.TXT","r");
  fread(grid,1,num_vertices,fp);
  grid[num_vertices] = tam_coluna;
  grid[num_vertices+1] = 'x';
  xbee = fdserial_open(9, 8, 0, 9600);
  //dprint(xbee, "llafjajfa");
  for(int i = 0; i < num_vertices+2; i++) {
    fdserial_txChar(xbee,grid[i]);
  } 
}


void receberCoordenadas() {
  unsigned short c;
  unsigned short coord[20];
  for(short i = 0 ; i < 20; i++)
  {
    c = fdserial_rxChar(xbee);
    if(c != -1)
    {
      coord[i] = c;
    }
  }
  fdserial_close(xbee); 
  for(short i = 0; i < 20; i++) {
    if(coord[i] == 29) {
       //freqout(4, 250, 4000);
    }
    pause(250);      
  }
  voltar(coord);
}
 
void voltar(unsigned short coord[]) {
  //startar automato para saber quando avancou um quadrado
  drive_goto(-4,-4);
  cog_automato = cog_run(startLeitura,128);
  //Muda a tarefa para percurso
  tarefa = PERCURSO;
  for(short i = 0; i < 20; i++) {
    if(coord[i] == 0) break;
    para = coord[i]; 
    corrigirGiro(de,para);
    de = para;
    if(!avancar1Quadrado()) {
      break;
    }
    //pause(100);
  }
}