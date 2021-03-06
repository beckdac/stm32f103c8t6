/* Task based UART demo, using queued communication.
 *
 * 38400 baud, 8N1, no flow control.
 */
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#define mainECHO_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName);

static QueueHandle_t uart_txq;				// TX queue for UART

void
vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName) {
	(void)pxTask;
	(void)pcTaskName;
	for(;;);
}

/*********************************************************************
 * Configure and initialize GPIO Interfaces
 *********************************************************************/

static void
gpio_setup(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz();	// CPU clock is 72 MHz
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
}

/*********************************************************************
 * Configure and initialize USART1:
 *********************************************************************/

static void
uart_setup(void) {

	/*************************************************************
	 *	RX:	A9  <====> TX of TTL serial
	 *	TX:	A10 <====> RX of TTL serial
	 *	CTS:	A11 (not used)
	 *	RTS:	A12 (not used)
	 *	Baud:	38400
	 * Caution:
	 *	Not all GPIO pins are 5V tolerant, so be careful to
	 *	get the wiring correct.
	 *************************************************************/
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);

	/* GPIO_USART1_TX/GPIO13 on GPIO port A for tx */
	gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO_USART1_TX);

	usart_set_baudrate(USART1,38400);
	usart_set_databits(USART1,8);
	usart_set_stopbits(USART1,USART_STOPBITS_1);
	usart_set_mode(USART1,USART_MODE_TX);
	usart_set_parity(USART1,USART_PARITY_NONE);
	usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
	usart_enable(USART1);

	/*************************************************************
	 * Create a queue for data to transmit from UART
	 *************************************************************/
	uart_txq = xQueueCreate(256,sizeof(char));
}

/*********************************************************************
 * USART Task: 
 *********************************************************************/

static void
uart_task(void *args) {
	char ch;

	(void)args;

	for (;;) {
		/* Receive char to be TX */
		if ( xQueueReceive(uart_txq,&ch,500) == pdPASS )
			usart_send_blocking(USART1,ch);	/* blocking call */
		/* Toggle LED to show signs of life */
		gpio_toggle(GPIOC,GPIO13);
	}
}

/*********************************************************************
 * Queue a string of characters to be TX
 *********************************************************************/

static inline void
uart_puts(const char *s) {
	
	for ( ; *s; ++s )
		xQueueSend(uart_txq,s,portMAX_DELAY); /* blocks when queue is full */
}

static void
put_rccmsg(void) {
	uint32_t rcc = RCC_CFGR;
	int x, n;
	char s[2];
	s[1] = 0;
	
	uart_puts("0x");
	for ( x=0; x<8; ++x ) {
		n = (rcc & 0xF0000000) >> 28;
		rcc <<= 4;
		if ( n <= 9 )
			n += '0';
		else	n = n - 10 + 'A';
		s[0] = n;
		uart_puts(s);
	}
	uart_puts(";\r\n");
}

/*********************************************************************
 * Demo Task:
 *	Simply queues up two line messages to be TX, one seconds
 *	apart.
 *********************************************************************/

static void
demo_task(void *args) {

	(void)args;

	for (;;) {
		uart_puts("Now this is a message..\n\r");
		uart_puts("  sent via FreeRTOS queues.\n\n\r");
put_rccmsg();
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

/*********************************************************************
 * Main program & scheduler:
 *********************************************************************/

int
main(void) {

	gpio_setup();
	uart_setup();

	xTaskCreate(uart_task,"UART",100,NULL,configMAX_PRIORITIES-1,NULL);	/* Highest priority */
	xTaskCreate(demo_task,"DEMO",100,NULL,configMAX_PRIORITIES-2,NULL);	/* Lower priority */

	vTaskStartScheduler();
	for (;;)
		;
	return 0;
}

/* End */
