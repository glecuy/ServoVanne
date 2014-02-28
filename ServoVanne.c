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
#include <avr/pgmspace.h>

#include "uart_printf.h"
#include "timer.h"
#include "lcd_5110.h"

#define TEMP_WITH_INTERRUPT 1

#ifdef TEMP_WITH_INTERRUPT

volatile unsigned short ADCval;

#define TemperatureRead() (ADCval)

void TemperatureInit( void )
{
    ADMUX = 0;                // use ADC0
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX |= (1 << ADLAR);    // Right adjust for 8 bit resolution
    //ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescale for 16Mhz
    ADCSRA |= (1 << ADATE);   // Set ADC Auto Trigger Enable

    ADCSRB = 0;               // 0 for free running mode

    ADCSRA |= (1 << ADEN);    // Enable the ADC
    ADCSRA |= (1 << ADIE);    // Enable Interrupts

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion
}

ISR(ADC_vect)
{
    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion
}

#else

void TemperatureInit( void )
{
    ADMUX = 0 | (1 << REFS0);  // use #1 ADC use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC
}


/* https://sites.google.com/site/qeewiki/books/avr-guide/analog-input */
unsigned short TemperatureRead( void )
{
    unsigned short ADCval;

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC))
       ;      // Thanks T, this line waits for the ADC to finish


    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}

#endif




int main(void)
{
    unsigned long Tick=0;

    timer1_init();

    timerO_PWM_Init();

    TemperatureInit();

    //  Enable global interrupts
    sei();

        //ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    // Initial UART Peripheral
    uart_printf_init();

    // Init LCD device
    LcdInitialise();

    // Initial the Wiznet W5100
    printf("ServoVanneInit\n\n");

    DDRB |= (1 << 5); // Set LED as output

    while ( 1 )
    {
        if ( timer1_GetTicks() >= (Tick + 10) )
        {
            Tick = timer1_GetTicks();
            printf_P(PSTR("TimerTicks=%lu\n"), Tick );

            printf_P(PSTR("Temp=%u\n"), TemperatureRead() );
            //timerO_PWM_SetValue( V++ );
            //printf("Counter = %d\n\n", V++);
        }
        //LcdString( "Hello World\n");
        //printf_P(PSTR("\n") );
    }

  return 0;
}

/* EOF  */
