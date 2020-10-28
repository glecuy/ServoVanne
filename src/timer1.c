/*
	timer.c

	Copyright 2014 GLecuyer <glecuyer207@gmail.com>


 */


#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>


volatile unsigned long TimerTicks;

int timer1_init(void)
{
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode

    // Set CTC compare value to 0.1Hz at 16MHz AVR clock, with a prescaler of 64 (16 bit value)
    OCR1A   = F_CPU/64/10 - 1;
    //OCR1A   = 249;

    TCCR1B |= ((0 << CS12) | (1 << CS11) | (1 << CS10)); // Start timer at Fcpu/64

    TimerTicks=0;
    // Enable Compare A Match Interrupt
    TIMSK1  |= (1 << OCIE1A );
    
    return 0;
}

ISR(TIMER1_COMPA_vect)
{
    TimerTicks++;
    
    //PORTB ^= (1<<5);
}

void UsSleep( unsigned long Delay )
{
	unsigned char TCNT_Prev;
	unsigned long Counts;
	
	/* 1 Lsb = 64/(F_CPU) Sec. */
	Counts = (F_CPU / (1000U * 64U));
	Counts *= Delay;
	Counts /= 1000;
	
	TCNT_Prev=0;
	while( Counts )
	{
		if ( TCNT_Prev != TCNT1L )
		{
			Counts--;
			TCNT_Prev = TCNT1L;
		}
	}
}

unsigned long timer1_GetTicks(void)
{
	unsigned long Ret;
	
	TIMSK1  &= ~(1 << OCIE1A );
	Ret = TimerTicks;
	TIMSK1  |= (1 << OCIE1A );
	
	return Ret;
}

