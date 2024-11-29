#include "asf.h"
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/
//defines relacionados ao led da xlpd
#define LED_PIO           PIOC
#define LED_PIO_ID        ID_PIOC
#define LED_PIO_IDX       8
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)

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
/* flags                                                            */
/************************************************************************/	
volatile char play_pause_flag;
volatile char change_frequency_flag;
volatile char decrease_flag;

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void init(void);
void pisca_led(int n, int t);
int change_frequency(int freq);

/************************************************************************/
/* functions                                                              */
/************************************************************************/
void edge_frequency_callback(void){
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		change_frequency_flag = 1;
	}
	else {
		change_frequency_flag = 0;
	}
}

void play_pause_callback(){
	play_pause_flag = 1;
}

void decrease_callback(){
	decrease_flag = 1;
}

int change_frequency(int frequency){
	if(decrease_flag){
		if (frequency >= 100) {
			frequency -= 100;
			char str[128];
			sprintf(str, "freq: %d", frequency);
			gfx_mono_draw_string(str, 10, 16, &sysfont);
			decrease_flag = 0;
		}
		return frequency;
	}

	for(double i = 0; i < 2500000; i++){
		if(!change_frequency_flag){
			frequency += 100;
			char str[128];
			sprintf(str, "freq: %d", frequency);
			gfx_mono_draw_string(str, 10, 16, &sysfont);
			return frequency;
		}
	}
	if (frequency >= 100) {
		change_frequency_flag = 0;
		frequency -= 100;
		char str[128];
		sprintf(str, "freq: %d", frequency);
		gfx_mono_draw_string(str, 10, 16, &sysfont);
	}
	return frequency;
}

void io_init(void){
	
	//faz inits
	delay_init();
	board_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();	
	
	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO_ID, PIO_OUTPUT_0, LED_PIO_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao	
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	//Configura Debounce
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);
	
	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_EDGE,
	edge_frequency_callback);
	
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	play_pause_callback);
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	decrease_callback);
	
	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);

	pio_get_interrupt_status(BUT1_PIO);	
	pio_get_interrupt_status(BUT2_PIO);
	pio_get_interrupt_status(BUT3_PIO);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_EnableIRQ(BUT3_PIO_ID);

	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	NVIC_SetPriority(BUT2_PIO_ID, 5); // Prioridade 5
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

void pisca_led(int n, int t){
	int counter = 0;
	for (int i=0; i<n; i++) {
		if(play_pause_flag) {
			pio_set(LED_PIO, LED_PIO_IDX_MASK);
			play_pause_flag = 0;
			break;
		}
		
		gfx_mono_generic_draw_vertical_line(45 + counter, 10, 5, GFX_PIXEL_SET);
		pio_clear(LED_PIO, LED_PIO_IDX_MASK);
		delay_ms(t/2);
		pio_set(LED_PIO, LED_PIO_IDX_MASK);
		delay_ms(t/2);
		counter += 1;
	}
	
	gfx_mono_generic_draw_filled_rect(45, 10, 30, 5, GFX_PIXEL_CLR);
}

int main (void)
{
	// Inicializa clock
	sysclk_init();
	
	// Chama funcao init
	io_init();
	
	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Cria variavel da frequencia
	int frequency = 400;
	char str[128];
	sprintf(str, "freq: %d", frequency);
	gfx_mono_draw_string(str, 10, 16, &sysfont);

	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if (change_frequency_flag || play_pause_flag || decrease_flag){
			if (change_frequency_flag){
				frequency = change_frequency(frequency);
			}
			else if (play_pause_flag){
				play_pause_flag = 0;
				pisca_led(30, frequency);
			}
			else if (decrease_flag){
				frequency = change_frequency(frequency);
			}
			change_frequency_flag = 0;
			play_pause_flag = 0;
			decrease_flag = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}