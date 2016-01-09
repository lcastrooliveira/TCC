#include "simpletools.h"

#define BRANCO 42
#define PRETO 43



// Porcentagem de variacao tolerada que sera utilizada para
// os valores limites de deteccao de cor.
float fator = 1.0;
  
// Valor da leitura
int leitura_atual;
int cor_atual;
  
int nova_leitura;

int lerValorSensor(void);

int getCorAtual(void);

int lerValorSensor() {
  high(3);
  pause(1);
  return rc_time(3, 1);
}  

int getCorAtual() {
  return cor_atual;
}  

int main()                    
{  
  leitura_atual = lerValorSensor();  
  cor_atual = BRANCO;
  
  while(1)
  {
    nova_leitura = lerValorSensor();
    if (cor_atual == BRANCO && nova_leitura >= 200000) {
      cor_atual = PRETO;
    } 
    else if (cor_atual == PRETO && nova_leitura < 200000) {
      cor_atual = BRANCO;
    }
    leitura_atual = nova_leitura;
    print("%ccor = %d, valor= %d",
           HOME, cor_atual , leitura_atual , CLREOL);
    pause(200);
  }
}
