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

#define TEMP_8_UNDEF -128
#pragma pack(1)
struct
{
    signed char Values[256];
    int N_max;
    int N;
    unsigned char Index;
}History;
#pragma pack()

/* Global variables */
static char TempString[8];
static unsigned char PwmVanne;

int OutdoorTemp;

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

void HistoryInit( long int Depth )
{
    int i;

    for ( i=0 ; i< 256 ; i++ )
    {
        History.Values[i] = TEMP_8_UNDEF;
    }
    History.Index = 0;
    History.N_max = (Depth+128L)/256;
    History.N = 0;
}

void HistoryAddValue( int T )
{
    long int Value;

    Value  = (long int)History.Values[History.Index] * 4;
    Value *= (long int)History.N;
    Value += (long int)T;
    History.N++;
    History.Values[History.Index] = (signed char)((Value/(long int)History.N)/4);
    if ( History.N >= History.N_max )
    {
        History.Index++;
        History.N = 0;
    }
}


int HistoryGetMax( void )
{
    int i;
    int max = -128;

    //printf( "N= %d Index = %d\n", History.N, History.Index );

    for ( i=0 ; i< 256 ; i++ )
    {
        if ( (History.Values[i] != TEMP_8_UNDEF) && (History.Values[i] > max) )
        {
            max = History.Values[i];
        }
    }

    return ((int)max * 4);
}

int HistoryGetMin( void )
{
    int i;
    int min = +127;

    //printf( "N= %d Index = %d\n", History.N, History.Index );

    for ( i=0 ; i< 256 ; i++ )
    {
        if ( (History.Values[i] != TEMP_8_UNDEF) && (History.Values[i] < min) )
        {
            min = History.Values[i];
        }
    }

    return ((int)min * 4);
}

//void DisplayMinMax( void )
//{
//	printf_P( PSTR("Min=%+d Max=%+d\n"), MinOutdoorTemp1.Temp, MaxOutdoorTemp1.Temp );
//}

int DbgTemperatureRead(void)
{
	static int temp;
	static int dt;

	if ( dt == 0 )
		dt = 5;
	temp+=dt;
	if ( (temp > 220) || (temp < -150) )
		dt = -dt;

	return temp;
}


int main(void)
{
    unsigned long Tick=0;
    int temp;

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

    // 24 H history
	//HistoryInit( 3600L*24 );

	HistoryInit( 5000 );

    printf("ServoVanneInit\n\n");



    DDRB |= (1 << PINB2) | (1 << PINB1); // Set LEDs as output

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
			Lcd_DrawStringXY( TempString, 2, 0 );

			OutdoorTemp = TemperatureRead();
			HistoryAddValue( OutdoorTemp );

			//DisplayMinMax();
			sprintf_P(TempString, PSTR("%+01d.%d` "),  OutdoorTemp/10, OutdoorTemp%10 );
			Lcd_DrawStringLargeXY( TempString, 12, 3 );

			temp = HistoryGetMin();
			printf("Min=%d\n", temp);
			sprintf_P(TempString, PSTR("%+01d.%d` "),  temp/10, temp%10 );
			Lcd_DrawStringXY( TempString, 0, 5 );

			temp = HistoryGetMax();
			printf("Max=%d\n", temp);
			sprintf_P(TempString, PSTR("%+01d.%d` "),  temp/10, temp%10 );
			Lcd_DrawStringXY( TempString, 50, 5 );

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


