/*****************************************************************************
//  File Name    : ServoVanne.c
//  Version      : 1.0
//  
*****************************************************************************/
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart_printf.h"
#include "timer.h"


int main(void)
{
	timer1_init();
	
	timerO_PWM_Init();
	
    //  Enable global interrupts
    sei();
    
	// Initial UART Peripheral
	uart_printf_init();
	// Initial the Wiznet W5100
	printf("ServoVanneInit\n\n");

	DDRB |= (1 << 5); // Set LED as output
    
    //DDRD=0xFF;
    //PORTD = 0x55;
	unsigned char V=0; 
    while ( 1 )
	{
		MsSleep(10);
		//printf("TimerTicks=%lu\n", timer1_GetTicks() );
		timerO_PWM_SetValue( V++ );
	}
  

  return 0;
}

/* EOF  */
