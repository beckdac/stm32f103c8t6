/* uartlib.h -- Interrupt driven USART 
 * Date: Tue Feb 21 20:44:52 2017   (C) Warren Gay ve3wwg
 *
 * NOTES:
 *	(1) It is assumed that the caller has configured the participating GPIO
 *          lines for the USART, as well as the GPIO RCC.
 *
 *	    For example, for USART1:
 *		GPIOA, A9 is Output (TX)
 *		GPIOA, A10 is Input (RX)
 *		GPIOA, A11 is Output (RTS, when used)
 *		GPIOA, A12 is Input (CTS, when used)
 *
 *	(2) These routines all use a "uart number", with 1 == USART1, 2==USART2
 *	    etc. This approach provided some opportunity for code optimization.
 *	(3) open_uart() will start the peripheral RCC.
 *	(4) open_uart() enables rx interrupts, when required.
 *
 */
#ifndef UARTLIB_H
#define UARTLIB_H

int open_uart(uint32_t uartno,uint32_t baud,const char *cfg,const char *mode,int rts,int cts);
void close_uart(uint32_t uartno);

int putc_uart_nb(uint32_t uartno,char ch);			/* non-blocking */
void putc_uart(uint32_t uartno,char ch);			/* blocking */
void write_uart(uint32_t uartno,const char *buf,uint32_t size); /* blocking */
void puts_uart(uint32_t uartno,const char *buf);		/* blocking */
int getc_uart_nb(uint32_t uartno);				/* non-blocking */
char getc_uart(uint32_t uartno);				/* blocking */
int getline_uart(uint32_t uartno,char *buf,uint32_t bufsiz);	/* blocking */


#endif // UARTLIB_H

/* End uartlib.h */
