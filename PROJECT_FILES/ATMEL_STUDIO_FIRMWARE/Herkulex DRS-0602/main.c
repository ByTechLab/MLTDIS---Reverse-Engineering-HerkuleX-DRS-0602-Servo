/*
 * Herkulex DRS-0602.c
 *
 * Created: 2019-06-11 11:36:17
 * Author : Andrzej Laczewski
 * Website: ByTechLab.com
 */ 
#define  F_CPU    32000000UL // 32MHz
#include    "Includes.h"
#define true 1
#define false 0
#define SERWO_POS_OFFSET 25

volatile uint8_t   RX_BUFF [100]; // UART data receive global variables
volatile uint8_t * RX_BUFF_POINTER = &RX_BUFF[0];
volatile uint8_t   RX_DATA_RDY = 0;

uint16_t   UART_SERWO_ANGLE = 200;
uint16_t   UART_SERWO_PWM =  900;
uint8_t	   START_MANEUVER = false; 

volatile uint8_t send_buff [500];

void init_OSC_PLL(uint8_t pllfactor)
{
	OSC.XOSCCTRL =      OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc; // Set oscillator frequency and time to start
	OSC.CTRL     =    OSC_XOSCEN_bm;       // Start 16Mhz crystal
	while(!(OSC.STATUS & OSC_XOSCRDY_bm)); // Wait for crystal to stabilize                                  
	
	CPU_CCP      =    CCP_IOREG_gc;        // Unlock system registers (write)
	CLK.CTRL     =    CLK_SCLKSEL_XOSC_gc; // Change clock source to crystal
	
	CPU_CCP      =    CCP_IOREG_gc;	      // Unlock system registers (write)s
	CLK.PSCTRL   =    CLK_PSADIV_1_gc;    // Set system clock prescaler to 1
	
	OSC.CTRL     &=   ~OSC_PLLEN_bm;      // Disable PLL
	OSC.PLLCTRL  =    OSC_PLLSRC_XOSC_gc | pllfactor; // Select crystal as PLL clock source
	OSC.CTRL     =    OSC_PLLEN_bm;       // Enable PLL
	while(!(OSC.STATUS & OSC_PLLRDY_bm)); // Wait for PLL to stabilize
	
	CPU_CCP      =    CCP_IOREG_gc;       // Unlock system registers (write)
	CLK.CTRL     =    CLK_SCLKSEL_PLL_gc; // Change clock source to PLL
}

void init_ADC(void)
{
	ADCA.CTRLA = ADC_ENABLE_bm; // Enable ADC
	ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc; // Set clock prescaler
	ADCA.REFCTRL = ADC_REFSEL_AREFA_gc; // Use external reference voltage (PA0) - 3V
	ADCA.CH0.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc; // Single ended mode, unsigned measurement
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc; // Take measurement from PA2 pin
	ADCA.CH0.INTCTRL = 0 ; // No interrupt
}

uint16_t ReadADC(uint8_t Channel)
{
	ADCA.CH0.CTRL |= ADC_CH_START_bm; // Start conversion
	while (ADCA.INTFLAGS==0);  // Wait for measurement to complete
	return ADCA.CH0RES ; // return result
}

void init_PMIC(void)
{
	sei(); // enable global interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm|PMIC_MEDLVLEN_bm;//|PMIC_HILVLEN_bm;  // Enable low and med piority interrupts
}

void init_TIM_0C(void)
{
	TCC0.CTRLB        =    TC_WGMODE_DS_TB_gc|TC0_CCAEN_bm|TC0_CCBEN_bm; // DS BOTH Dual slope PWM mode,Enable outputs
	TCC0.PER          =    1000; // Set timer period
	TCC0.CCA          =    0; // CHA duty cycle value form 0 to 1000, Enable one at a time, sets direction
	TCC0.CCB          =    0; // CHB duty cycle value form 0 to 1000,
	TCC0.CTRLA        =    TC_CLKSEL_DIV1_gc;    // Start timer and set clock prescaler to 1 , PWM freq = 32KHz
}

int main(void) {
	
	init_OSC_PLL(2);
	init_USART_D0();
	init_PMIC();
	init_ADC();
	init_TIM_0C();
	
	PORTB.DIR = 0b00000011; //Set ports directions
	PORTA.DIR = 0b10000000;
	PORTC.DIR = 0b00000111;  
	PORTC.OUTSET = PIN2_bm;  // Set SD to 1 , enable H bridge   
	
	uint16_t ADC_result = 0;

	while(1) 
	{
			if (START_MANEUVER == true)
			{
				PORTB.OUTCLR    =   PIN0_bm; // Turn on GREEN LED
				ADC_result = ReadADC(0);
				if (ADC_result > UART_SERWO_ANGLE + SERWO_POS_OFFSET )
				{
					TCC0.CCB =  UART_SERWO_PWM;
					TCC0.CCA =  0;
				}
				else if (ADC_result < UART_SERWO_ANGLE - SERWO_POS_OFFSET)
				{
					TCC0.CCA =  UART_SERWO_PWM;
					TCC0.CCB =  0;
				}
				else
				{
					TCC0.CCA =  0;
					TCC0.CCB =  0;
				}
			}
			else 
			{
				PORTB.OUTSET    =   PIN0_bm; // Turn off GREEN LED
			}
	
			if (RX_DATA_RDY == 1)
			{
				if ((RX_BUFF[0] == '$') && (RX_BUFF[1] != '$') && (RX_BUFF[2] != '$')) // Set current angle
				{
					UART_SERWO_ANGLE = atoi(&RX_BUFF[1]);
					if (UART_SERWO_ANGLE < 200)
					{
						UART_SERWO_ANGLE = 200;
					}
					if (UART_SERWO_ANGLE > 4000)
					{
						UART_SERWO_ANGLE = 4000;
					}
					sprintf(send_buff,"#%d-[OK]-ANG VAL\r\n",UART_SERWO_ANGLE);
					sendString(send_buff);
				} 
				else if ((RX_BUFF[0] == '$') && (RX_BUFF[1] == '$') && (RX_BUFF[2] != '$')) // Set current PWM value
				{
					UART_SERWO_PWM = atoi(&RX_BUFF[2]);
					if (UART_SERWO_PWM < 100)
					{
						UART_SERWO_PWM = 100;
					}
					if (UART_SERWO_PWM  > 990)
					{
						UART_SERWO_PWM = 990;
					}
					sprintf(send_buff,"#%d-[OK]-PWM VAL\r\n",UART_SERWO_PWM);
					sendString(send_buff);
				}
				else if ((RX_BUFF[0] == '$') && (RX_BUFF[1] == '$') && (RX_BUFF[2] == '$')) // Other commands
				{
					if (RX_BUFF[3] == 'A') // Send current position
					{
						sprintf(send_buff,"#%d-[OK]-CURR ANGLE\r\n",ReadADC(0));
						sendString(send_buff);
					}
					else if (RX_BUFF[3] == 'E') // Enable servo
					{
						START_MANEUVER = true;
						sprintf(send_buff,"#%d-[OK]-MOVE ENABLED\r\n",START_MANEUVER);
						sendString(send_buff);
					}
					else if (RX_BUFF[3] == 'D') // Disable servo
					{
						START_MANEUVER = false;
						sprintf(send_buff,"#%d-[OK]-MOVE DISABLED\r\n",START_MANEUVER);
						sendString(send_buff);
					}
				}
				RX_DATA_RDY = 0 ;
			}
			PORTB.OUTSET    =   PIN1_bm; // Turn off BLUE LED

	}
}

ISR (USARTD0_RXC_vect){
	uint8_t rcv_char = USARTD0.DATA;
	PORTB.OUTCLR    =   PIN1_bm; // Turn on BLUE LED
	if (rcv_char == '\n') // detect last character
	{
		*RX_BUFF_POINTER = rcv_char;
		RX_BUFF_POINTER= &RX_BUFF[0];
		RX_DATA_RDY = 1;
	}
	else
	{
		*RX_BUFF_POINTER = rcv_char;
		RX_BUFF_POINTER++;
	}

}