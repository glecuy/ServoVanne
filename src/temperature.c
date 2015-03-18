/*****************************************************************************
//  File Name    : temperature.c
//  Version      : 1.0
//
*****************************************************************************/
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#ifdef TEMP_NO_INTERRUPT
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

#else

volatile unsigned short ADCval;

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

/*
 *  -20° -> 90K 
 *    0° -> 30K
 *   25° -> 10K
 *   30° -> 8K
 *   40° -> 5K
 * NTC TDC310 
 *  5K -> 122 (100)-> 40
 * 10K -> 218 (218)-> 25° 
 * 20K -> 350 (350)-> 10°
 * 50K -> 552 (535)-> -10°
 *****************************************/
  int TEST;
 
int TemperatureRead( void )
{
    long int  adc, Temp;

	/* TODO log scale ? */
	// Pente  = -0.122
	// OffSet = 500	

	adc = (long int)(ADCval/100);
	if ( adc < 100 )
		Temp = 390;
	else if ( (adc >= 100) && (adc < 550) )
	{
		Temp = ((-113 * adc) + 48100) / 100;
	}
	else
	{
		Temp = -180;
	}
	
    return (int)Temp;
}

#endif



/* EOF  */
