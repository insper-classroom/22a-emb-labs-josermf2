#include "asf.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/**
 *  Informacoes para o RTC
 *  poderia ser extraida do __DATE__ e __TIME__
 *  ou ser atualizado pelo PC.
 */
typedef struct  {
  uint32_t year;
  uint32_t month;
  uint32_t day;
  uint32_t week;
  uint32_t hour;
  uint32_t minute;
  uint32_t second;
} calendar;

/************************************************************************/
/* DEFINES                                                              */
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
/* VAR globais                                                          */
/************************************************************************/
volatile char flag_rtc_alarm = 0;


/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void LED_init(Pio *pio, uint32_t mask, int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void pin_toggle(Pio *pio, uint32_t mask);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
void io_init(void);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
		flag_rtc_alarm = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}


/**
*  Interrupt handler for TC1 interrupt.
*/
void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED_PIO, LED_PIO_IDX_MASK);  
}

/**
*  Interrupt handler for TC2 interrupt.
*/
void TC2_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 2);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED1_PIO, LED1_PIO_IDX_MASK);  
}

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		RTT_init(0.25, 0, RTT_MR_RTTINCIEN);
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		pin_toggle(LED2_PIO, LED2_PIO_IDX_MASK);    // BLINK Led
	}

}

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

/**
* @Brief Inicializa o pino do LED
*/
void LED_init(Pio *pio, uint32_t mask, int estado) {
	pmc_enable_periph_clk(pio);
	pio_set_output(pio, mask, estado, 0, 0);
};

/**
* @Brief Inverte o valor do pino 0->1/ 1->0
*/
void pin_toggle(Pio *pio, uint32_t mask) {
  if(pio_get_output_data_status(pio, mask))
    pio_clear(pio, mask);
  else
    pio_set(pio,mask);
}

/**
* Configura TimerCounter (TC) para gerar uma interrupcao no canal (ID_TC e TC_CHANNEL)
* na taxa de especificada em freq.
* O TimerCounter é meio confuso
* o uC possui 3 TCs, cada TC possui 3 canais
*	TC0 : ID_TC0, ID_TC1, ID_TC2
*	TC1 : ID_TC3, ID_TC4, ID_TC5
*	TC2 : ID_TC6, ID_TC7, ID_TC8
*
**/
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
  	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}



static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

/** 
 * Configura RTT
 *
 * arg0 pllPreScale  : Frequência na qual o contador irá incrementar
 * arg1 IrqNPulses   : Valor do alarme 
 * arg2 rttIRQSource : Pode ser uma 
 *     - 0: 
 *     - RTT_MR_RTTINCIEN: Interrupção por incremento (pllPreScale)
 *     - RTT_MR_ALMIEN : Interrupção por alarme
 */
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

  uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
  rtt_sel_source(RTT, false);
  rtt_init(RTT, pllPreScale);
  
  if (rttIRQSource & RTT_MR_ALMIEN) {
	uint32_t ul_previous_time;
  	ul_previous_time = rtt_read_timer_value(RTT);
  	while (ul_previous_time == rtt_read_timer_value(RTT));
  	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
  }

  /* config NVIC */
  NVIC_DisableIRQ(RTT_IRQn);
  NVIC_ClearPendingIRQ(RTT_IRQn);
  NVIC_SetPriority(RTT_IRQn, 4);
  NVIC_EnableIRQ(RTT_IRQn);

  /* Enable RTT interrupt */
  if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
		rtt_enable_interrupt(RTT, rttIRQSource);
  else
		rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
		  
}

void set_alarm(void) {
	/* Leitura do valor atual do RTC */
	uint32_t current_hour, current_min, current_sec;
	uint32_t current_year, current_month, current_day, current_week;
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
	
	/* configura alarme do RTC para daqui 20 segundos */
	rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
	rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 5);
}

void but_callback(void) {
	set_alarm();
}

void io_init(void){
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pio_set_input(BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_DEBOUNCE);
	pio_pull_up(BUT1_PIO, BUT1_PIO_IDX_MASK, 1);
	
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but_callback);
	
	calendar rtc_initial = {2020, 03, 19, 11, 15, 21 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
	
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Init OLED
	gfx_mono_ssd1306_init();
  	
	/* Configura Leds */
	LED_init(LED_PIO, LED_PIO_IDX_MASK, 1);
	LED_init(LED1_PIO, LED1_PIO_IDX_MASK, 1);
	LED_init(LED2_PIO, LED2_PIO_IDX_MASK, 1);
	LED_init(LED3_PIO, LED3_PIO_IDX_MASK, 1);


	/**
	 * Configura timer TC0, canal 1 
	 * e inicializa contagem 
	 */
	TC_init(TC0, ID_TC1, 1, 5);
	tc_start(TC0, 1);
	
	/**
	 * Configura timer TC0, canal 2 
	 * e inicializa contagem 
	 */
	TC_init(TC0, ID_TC2, 2, 4);
	tc_start(TC0, 2);
	
	io_init();
	

	uint32_t current_hour_draw, current_min_draw, current_sec_draw;

	/* 
	* Ativa RTT para trabalhar por alarme
	* gerando uma interrupção em 4 s:
	* aguarda 4 segundos
	* tempo[s] = 0.25 * 16 = 4s
	*/
	RTT_init(0.25, 0, RTT_MR_RTTINCIEN);
		
  /* Insert application code here, after the board has been initialized. */
	while(1) {
		char hour[128];
		char minute[128];
		char second[128];
			
		rtc_get_time(RTC, &current_hour_draw, &current_min_draw, &current_sec_draw);
				
		sprintf(hour, "%d", current_hour_draw);
		sprintf(minute, "%d", current_min_draw);
		if (current_sec_draw < 10) {
			sprintf(second, "0%d", current_sec_draw);
		} else {
			sprintf(second, "%d", current_sec_draw);
		}
		
		gfx_mono_draw_string("Hora atual:", 10, 0, &sysfont);
		gfx_mono_draw_string(hour, 10, 15, &sysfont);
		gfx_mono_draw_string(":", 30, 15, &sysfont);
		gfx_mono_draw_string(minute, 40, 15, &sysfont);
		gfx_mono_draw_string(":", 60, 15, &sysfont);
		gfx_mono_draw_string(second, 70, 15, &sysfont);
		
		if (flag_rtc_alarm) {
			pio_clear(LED3_PIO, LED3_PIO_IDX_MASK);
			delay_ms(1000);
			pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
			flag_rtc_alarm = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
