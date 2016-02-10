/*
Navegação Simple.c - Objetivo: Controlar a locomação do robô para fazer um zigue-zague
mandar e receber instruções do computador para realizar o trajeto de volta,
sem passar em cima dos quadrados pretos

Autor: Lucas de Castro Oliveira
Data: 22/01/2016
*/

#include "simpletools.h" // Biblioteca padrão
#include "abdrive.h" // Biblioteca para controlar os motores
#include "ping.h" // Biblioteca para controlar o sensor de ultrassom
#include "deccor.h" // Biblioteca criada, máquina de estados
#include "fdserial.h" // Biblioteca responsável pela comunicação serial

//Códigos de cada uma das quadro direções possíveis que o robô pode seguir
#define NORTE 13
#define OESTE 37
#define LESTE 59
#define SUL 29

//variáveis globais para navegação
short de,para;
//variável que armazenz uma das dimimensões da matriz
short tam_coluna;
//variável que representa a conexão serial através do XBee
fdserial *xbee;
//vetor que marca a sequência de sentidos a serem seguidos pelo robô
const int relogio_sentidos[4] = {LESTE,NORTE,OESTE,NORTE};
//index para navegar no relógio de sentidos
int ponteiro_relogio = 0;
// referência ao processo da maquina de estados correndo em outro núcleo
int *cog_automato;

int main()                                    // main function
{
  //códigos que representam os pinos para leitura do cartão de memória
  int DO = 22, CLK = 23, DI = 24, CS = 25;
  //montar o cartão de memória
  sd_mount(DO, CLK, DI, CS);
  //apagar qualquer vestígio do arquivo gerado de um escaneamento anterior
  remove("QUADRADO.TXT");
  // velócidade máxima = 16 ticks por segundo
  int SPEED = MSPEED;
  // distância mínima tolerada em relação a parade(em centímetros)
  int MIN_DIST = 7;
  //define que a tarefa a ser executada como mapeamento
  tarefa = MAPEAMENTO;
  //define a direcao Inicial
  de = relogio_sentidos[ponteiro_relogio];
  drive_setRampStep(SPEED);                     
  drive_setMaxSpeed(SPEED);
  //espera 5 segundos para que a pessoa possa posicionar o robô
  //alinhado corretamente no cenário
  pause(5000);
  //inicializa máquina de estados e começa o trabalho de detecção
  //de cores
  cog_automato = cog_run(startLeitura,128);
  while(TRUE) {
    //andar em linha reta
    drive_ramp(SPEED, SPEED);
    //espera um 1/10 segundo
    pause(100);
    //consulta sensor de ultrassom a cada 5 milisegundos para saber se
    //está perto da parde
    while(ping_cm(11) >= MIN_DIST) pause(5);
    //arruma o seu sentido conforme o relógio de sentidos
    atualizarSentido();
    //captura uma das dimensões do cenário
    if(tam_coluna == 0) tam_coluna = num_vertices;
    //atualiza o sentido destino e faz a rotação apropriada
    para = relogio_sentidos[ponteiro_relogio];
    corrigirGiro(de,para);
    de = para;
    //se puder avançar o espaço de 1 quadrado é por que
    //não se chegou ao fim do cenário ainda
    if(!avancar1Quadrado()) {
      break;
    }
    //novamente atualiza o sentido de destino e faz a rotação
    //correspondente      
    atualizarSentido();
    para = relogio_sentidos[ponteiro_relogio];
    corrigirGiro(de,para);
    de = para;
  }
  //pára o robô
  drive_ramp(0, 0);
  //manda as informações das cores de cada quadrado
  mandarMapaGrid();
  //recebe as coordenadas do compuador
  receberCoordenadas();
}

//determina qual o proximo valor será lido do relógio
//dos sentidos. fórmula semelhante a da fila circular.
void atualizarSentido() {
  ponteiro_relogio = (ponteiro_relogio + 1) % 4;
}

//função que recebe um sentido de origem e um sentido de destino
//e faz a rotação em graus apropriada que parte da origem para o
//destino.
void corrigirGiro(short sentido_origem, short sentido_destino) {
  //para o robô(para garantir)
  drive_ramp(0, 0);
  //se o sentido de origem e destino forem iguais faz nada.
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

//função que le as informações de cores no arquivo
//QUADRADO.TXT e manda os caracteres para o computador via XBee
//o formato dos dados é igual ao pacote discutido no TCC.
void mandarMapaGrid() {
  unsigned char grid[num_vertices+2];
  FILE* fp = fopen("QUADRADO.TXT","r");
  fread(grid,1,num_vertices,fp);
  grid[num_vertices] = tam_coluna;
  grid[num_vertices+1] = 'x';
  xbee = fdserial_open(9, 8, 0, 9600);
  for(int i = 0; i < num_vertices+2; i++) {
    fdserial_txChar(xbee,grid[i]);
  } 
}

//recebe as coordenadas de volta do computador e armazena
//as instruções num array contendo o código de cada sentido
//a ser tomado pelo robô. 0 significa que as coordenadas acabaram
void receberCoordenadas() {
  unsigned short c;
  unsigned short coord[num_vertices];
  for(short i = 0 ; i < num_vertices; i++)
  {
    c = fdserial_rxChar(xbee);
    if(c != -1)
    {
      coord[i] = c;
    }
  }
  fdserial_close(xbee);
  //faz um beep para cada coordenada recebida
  for(short i = 0; i < num_vertices; i++) {
    if(coord[i] != 0) {
      freqout(4, 250, 4000);
      pause(250);          
    } else break;
  }
  voltar(coord);
}

//funcao responsavel por fazer o robo voltar
//ao seu estado de origem 
void voltar(unsigned short coord[]) {
  //Anda um pouco para tras para abrir mais espaco
  if(ping_cm(11) > 5) {
    drive_goto(6,6);
  }
  else if(ping_cm(11) < 2) {
    drive_goto(-6,-6);
  }
  //muda a tarefa para percurso nao e mais escaneamento
  tarefa = PERCURSO;
  for(short i = 0; i < num_vertices; i++) {
    if(coord[i] == 0) break;
    para = coord[i]; 
    corrigirGiro(de,para);
    de = para;
    //Se avancar1Quadrado retornar FALSE
    //quer dizer que o robo esta muito perto
    //da parede, e pode estar muito
    //perto do seu destino    
    if(!avancar1Quadrado()) break;
  }
}