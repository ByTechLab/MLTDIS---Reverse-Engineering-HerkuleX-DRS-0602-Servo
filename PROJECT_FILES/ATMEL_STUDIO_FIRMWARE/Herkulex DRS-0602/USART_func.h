/*
 * USAR_func.h
 *
 * Created: 2016-01-14 13:37:04
 *  Author: Andrzej
 */ 


#ifndef USAR_FUNC_H_
#define USAR_FUNC_H_

#include    "Includes.h"

#define BAUD_9600 0xCF // 207
#define BAUD_57600 0x23 //34

// BSEL = (32000000 / (2^0 * 16*9600) -1 = 207.333 -> BSCALE = 0
// FBAUD = ( (32000000)/(2^0*16(207+1)) = 9615.384 -> it's alright
//#define CAT(A, B) A B
//// #define BSEL(baudrate)  (32000000 / (16 * baudrate) -1 )

#define TX_NEWLINE {sendChar('\r',&USARTC0); sendChar('\n',&USARTC0);}

void init_USART_D0(void);
void sendChar(char c, USART_t *_register);
void sendString(char *text);
//----------------------------------------------
void send_uint32t(uint32_t int_to_send);
void send_int32t(int32_t int_to_send);
void sendfloat(float int_to_send,int frac_num,int tot_num );
void sendString_PGM(char* string);


#endif /* USAR_FUNC_H_ */