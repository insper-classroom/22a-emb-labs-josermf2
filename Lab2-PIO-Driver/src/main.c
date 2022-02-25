/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

//defines relacionados ao led da xpld
#define LED_PIO           PIOC
#define LED_PIO_ID        ID_PIOC
#define LED_PIO_IDX       8
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)

//defines relacionados ao botão da xpld
#define BUT_PIO           PIOA
#define BUT_PIO_ID        ID_PIOA
#define BUT_PIO_IDX       11
#define BUT_PIO_IDX_MASK  (1u << BUT_PIO_IDX)

//defines relacionados ao led 1 da oled1
#define LED1_PIO           PIOA
#define LED1_PIO_ID        ID_PIOA
#define LED1_PIO_IDX       0
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)

//defines relacionados ao led 2 da oled1
#define LED2_PIO           PIOC
#define LED2_PIO_ID        ID_PIOC
#define LED2_PIO_IDX       30
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)

//defines relacionados ao led 3 da oled1
#define LED3_PIO           PIOB
#define LED3_PIO_ID        ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)

//defines relacionados ao botão1 da oled1
#define BUT1_PIO           PIOD
#define BUT1_PIO_ID        ID_PIOD
#define BUT1_PIO_IDX       28
#define BUT1_PIO_IDX_MASK  (1u << BUT1_PIO_IDX)

//defines relacionados ao botão2 da oled1
#define BUT2_PIO           PIOC
#define BUT2_PIO_ID        ID_PIOC
#define BUT2_PIO_IDX       31
#define BUT2_PIO_IDX_MASK  (1u << BUT2_PIO_IDX)

//defines relacionados ao botão3 da oled1
#define BUT3_PIO           PIOA
#define BUT3_PIO_ID        ID_PIOA
#define BUT3_PIO_IDX       19
#define BUT3_PIO_IDX_MASK  (1u << BUT3_PIO_IDX)
/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void) {
	//Inicia clock da placa
	sysclk_init();
	
	//Desativa WatchDog timer
	WDT->WDT_MR = WDT_MR_WDDIS; 
	
	//Ativa o PIO na qual o LED foi conectado para que possamos controlar
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);

	//Inicializa PC8 como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0, 0, 0);

	//Inicializa PIO do botão
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	//Configura pino ligado ao botão como entrada com um pull-up
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT2_PIO, BUT3_PIO_IDX_MASK, PIO_DEFAULT);

	//Ativa pull-up no pino
	pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, 1);
	pio_pull_up(BUT1_PIO, BUT1_PIO_IDX_MASK, 1);
	pio_pull_up(BUT2_PIO, BUT2_PIO_IDX_MASK, 1);
	pio_pull_up(BUT3_PIO, BUT3_PIO_IDX_MASK, 1);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1) {
	  
	  //tasks 13 e 14
	  //pio_set(PIOC, LED_PIO_IDX_MASK);
	  //delay_ms(3000);
	  //pio_clear(PIOC, LED_PIO_IDX_MASK);
	  //delay_ms(1000);
	  
	  //led pisca 5 vezes ao apertar o botão da xpld
	  if(!pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK)) {
	    // Pisca LED
		for (int i=0; i<5; i++) {
			pio_clear(LED_PIO, LED_PIO_IDX_MASK);  // Limpa o pino LED_PIO_PIN
			delay_ms(100);                         // delay
			pio_set(LED_PIO, LED_PIO_IDX_MASK);    // Ativa o pino LED_PIO_PIN
			delay_ms(100);                         // delay
		    }
		} else {
		    // Ativa o pino LED_IDX (par apagar)
		    pio_set(LED_PIO, LED_PIO_IDX_MASK);
	    }
	
		//led1 da oled1 pisca 5 vezes ao apertar o botão1 da oled1
		if(!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
			// Pisca LED
			for (int i=0; i<5; i++) {
				pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);  // Limpa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
				pio_set(LED1_PIO, LED1_PIO_IDX_MASK);    // Ativa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
			}
			} else {
			// Ativa o pino LED_IDX (par apagar)
			pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
		}

		//led2 da oled1 pisca 5 vezes ao apertar o botão2 da oled1
		if(!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
			// Pisca LED
			for (int i=0; i<5; i++) {
				pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);  // Limpa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
				pio_set(LED2_PIO, LED2_PIO_IDX_MASK);    // Ativa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
			}
			} else {
			// Ativa o pino LED_IDX (par apagar)
			pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
		}

		//led3 da oled1 pisca 5 vezes ao apertar o botão3 da oled1
		if(!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
			// Pisca LED
			for (int i=0; i<5; i++) {
				pio_clear(LED3_PIO, LED3_PIO_IDX_MASK);  // Limpa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
				pio_set(LED3_PIO, LED3_PIO_IDX_MASK);    // Ativa o pino LED_PIO_PIN
				delay_ms(100);                         // delay
			}
			} else {
			// Ativa o pino LED_IDX (par apagar)
			pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
		}

  }
  return 0;
}
