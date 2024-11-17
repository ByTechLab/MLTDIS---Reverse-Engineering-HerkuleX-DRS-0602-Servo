/*
 * USART_func.c
 *
 * Created: 2016-01-14 13:37:33
 *  Author: Andrzej
 */ 

#include    "Includes.h"


void init_USART_D0(void)
{
	USARTD0.BAUDCTRLB = 0; //BSCALE is 0
	USARTD0.BAUDCTRLA = BAUD_9600; // Set baud rate to 9600
	USARTD0.CTRLA = USART_RXCINTLVL_MED_gc;  //Enable interrupts
	USARTD0.CTRLC = USART_CHSIZE_8BIT_gc; // 8 data bits, no parity and 1 stop bit
	USARTD0.CTRLB = USART_TXEN_bm | USART_RXEN_bm; // Enable receive and transmit
	PORTD.DIRSET = PIN3_bm; 
}
void sendChar(char c, USART_t *_register)
{
	while( !(_register->STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	_register->DATA = c;
}
void sendString(char *text)
{
	while(*text)
	{
		sendChar(*text++,&USARTD0);
	}
}
//---------------------------------------
void sendString_PGM(char* string)
{
	while (pgm_read_byte(&(*string)))
	{
		sendChar(pgm_read_byte(&(*string++)),&USARTD0);
	}
}

// debug functions
void send_uint32t(uint32_t int_to_send)
{
	char usart_bufor[30];
	ultoa(int_to_send, usart_bufor, 10);
	//insert_newline(&usart_bufor);
	sendString(usart_bufor);
}

void send_int32t(int32_t int_to_send)
{
	char usart_bufor[30];
	ltoa(int_to_send, usart_bufor, 10);
	//insert_newline(&usart_bufor);
	sendString(usart_bufor);
}

void sendfloat(float int_to_send,int frac_num,int tot_num )
{
	char usart_bufor[30];
	dtostrf(int_to_send,tot_num,frac_num, usart_bufor);
	//insert_newline(&usart_bufor);
	sendString(usart_bufor);
}
//end debug func