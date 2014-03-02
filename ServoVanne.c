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
#include <avr/eeprom.h>

#include "uart_printf.h"
#include "timer.h"
#include "lcd_5110.h"
#include "temperature.h"

/* Global variable */
static char TempString[8];
static int OutdoorTemp;
static unsigned char PwmVanne;


unsigned char Thermostat( void )
{
	if ( PINB & (1<<PINB0) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/* Pomp_cmd connected to Pin C2 (25) */
#define PompInit() (DDRC |= (1 << PINC2)) // Pomp_cmd LED as output
#define PompOn()   (PORTC |= (1 << PINC2))
#define PompOff()  (PORTC &= ~(1 << PINC2))

/* Return a value from 0 to 100 
 * corresponding to the desired aperture for the valve
 */
#define MAX_APERTURE 80
#define MIN_APERTURE 0
#define T_FOR_MAX_APERTURE -15
#define T_FOR_MIN_APERTURE +22
unsigned char TempToValve( int temperature )
{	
	if ( temperature > T_FOR_MIN_APERTURE )
		return 0;
	else if ( temperature < T_FOR_MAX_APERTURE )
		return 80;
	else 
	{
		int a, b;
		a = ((MAX_APERTURE-MIN_APERTURE)/(T_FOR_MAX_APERTURE-T_FOR_MIN_APERTURE));
		b = (MIN_APERTURE - a * T_FOR_MIN_APERTURE);
		return ( temperature * a + b); 
	}	
}

//#undef TemperatureRead
//#define TemperatureRead() (5)


int main(void)
{
    unsigned long Tick=0;
    

    timer1_init();

    timerO_PWM_Init();

    TemperatureInit();

    //  Enable global interrupts
    sei();

    // Initialize UART print
    uart_printf_init();

    // Init LCD device
    LcdInitialise();
    
    PompInit();

    printf("ServoVanneInit\n\n");



    DDRB |= (1 << PINB2) | (1 << PINB1); // Set LEDs as output
    

// 	int t;
//	for ( t= -25 ; t<+25 ; t+=2 )
//	{
//		printf_P(PSTR("T=%+d > Valve=%d\n"), t, TempToValve( t ) );
//	}

//	while( 1 )
//	{
//		Lcd_DrawString( "Hello" );
//		MsSleep(2);		
//	}
//{
//	long int R1, R2, RR, RT;
//	
//	R1= 56L;
//	R2= 68L;
//	
//	RT=100L;	
//	RR= (long)(RT*R2)/(RT+R2);
//	printf_P(PSTR("R1=%ld R2=%ld RR=%ld, V=%ld\n"), R1, R2, RR, (120*RR)/(RR+R1) );
//	RT=5L;	
//	RR= (long)(RT*R2)/(RT+R2);
//	printf_P(PSTR("R1=%ld R2=%ld RR=%ld, V=%ld\n"), R1, R2, RR, (120*RR)/(RR+R1) );	
//}

    while ( 1 )
    {		
        if ( timer1_GetTicks() >= (Tick + 10) )
        {
			Tick = timer1_GetTicks();
			
			if ( Thermostat() != 0 )
			{
				sprintf_P(TempString, PSTR("Marche"));
				PORTB |= (1 << PINB2);
			}
			else
			{
				sprintf_P(TempString, PSTR("Arret "));
				PORTB &= ~(1 << PINB2);
			}			
			Lcd_DrawStringXY( TempString, 0, 0 );			
			
			OutdoorTemp = TemperatureRead();
			sprintf_P(TempString, PSTR("%+d.%d`"),  OutdoorTemp/10, OutdoorTemp%10 );
            printf_P(PSTR("Temp=%s\n"), TempString );
            //puts( TempString );
			//Lcd_DrawString( TempString );
			Lcd_DrawStringLargeXY( TempString, 12, 4 );
			timerO_PWM_SetValue( PwmVanne );
			PwmVanne+=10;
        }

    }

  return 0;
}

/* EOF  */

int main__(void)
{
    unsigned long Tick=0;

    timer1_init();

    timerO_PWM_Init();

    TemperatureInit();

    //  Enable global interrupts
    sei();

    // Initialize UART print
    uart_printf_init();

    // Init LCD device
    LcdInitialise();

    printf("ServoVanneInit\n\n");

    DDRB |= (1 << 1) | (1 << 2); // Set LED as output
	PORTB ^= (1 << 2);
    while ( 1 )
    {
        if ( timer1_GetTicks() >= (Tick + 10) )
        {
			PORTB ^= (1 << 1) | (1 << 2);
            Tick = timer1_GetTicks();
            printf_P(PSTR("TimerTicks=%lu\n"), Tick );

            printf_P(PSTR("Temp=%u\n"), TemperatureRead() );
            //timerO_PWM_SetValue( V++ );
            //printf("Counter = %d\n\n", V++);
            //LcdString( "Hello World\n");
        }
        Lcd_DrawStringXY( "Hello World\n", 0, 0);
        //printf_P(PSTR("\n") );
        MsSleep( 5 );
    }

  return 0;
}


