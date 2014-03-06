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


/*************
 * History:
 * 256 table values (8bit)
 * Each table entry is the mean value: Sigma / N
 * Index loop for ever modulo 256
 * N_max depends on the rate of values (Typ 1 per Sec)
 *******************************/
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
static char TempString[12];
static unsigned short PwmVanne;
static unsigned short PwmRampUp;
// Heating Cycle estimation
static unsigned short PompPowerRatio;

int OutdoorTemp;

/* Pomp_cmd connected to Pin C2 (25) */
#define PompInit() (DDRC |= (1 << PINC2)) // Pomp_cmd LED as output
#define PompOn()   (PORTC |= (1 << PINC2))
#define PompOff()  (PORTC &= ~(1 << PINC2))


#define MAX_OPENING_TIME (30*60)

/* Return a value from 0 to 100
 * corresponding to the desired opening ratio for the valve
 */
#define MAX_OPENING 80
#define MIN_OPENING 0
#define T_FOR_MAX_OPENING -100
#define T_FOR_MIN_OPENING +220
unsigned short TempToValve( int temperature )
{
	if ( temperature > T_FOR_MIN_OPENING )
		return 0;
	else if ( temperature < T_FOR_MAX_OPENING )
		return 80;
	else
	{
		short int a, b;
		a = (100*(MAX_OPENING-MIN_OPENING)/(T_FOR_MAX_OPENING-T_FOR_MIN_OPENING));
		b = (MIN_OPENING - a * T_FOR_MIN_OPENING);
		return (unsigned short)(((temperature * a) + b)/100);
	}
}

unsigned char Thermostat( void )
{
	if ( PINB & (1<<PINB0) )
	{
		return 0;
	}
	else
	{
		return 1;
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

/* Add value to history
 * Params T Temp in 10th degrees
 *        IsOn Non zero if Heating is turn ON
 **********************************************/
void HistoryAddValue( int T )
{
    long int Value;

	/* Get current value, if N==0, Value is reset */
    Value  = (long int)History.Values[History.Index] * 4;
    Value *= (long int)History.N;
    Value += (long int)T;
    History.N++;
    /* Calculate the mean value */
    History.Values[History.Index] = (signed char)((Value/(long int)History.N)/4);
    /* Test for next table entry */
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

    for ( i=0 ; i< 256 ; i++ )
    {
        if ( (History.Values[i] != TEMP_8_UNDEF) && (History.Values[i] < min) )
        {
            min = History.Values[i];
        }
    }

    return ((int)min * 4);
}


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
    unsigned short RampUp;
    int temp;

    timer1_init();
    timerO_PWM_Init(0);
    TemperatureInit();

    //  Enable global interrupts
    sei();

    // Initialize UART print
    uart_printf_init();
    // Init LCD device
    LcdInitialise();

    PompInit();

    // 24 H history => about 6 Min for each interval
	HistoryInit( 3600L*24 );
    
    printf("ServoVanneInit\n\n");

    DDRB |= (1 << PINB2) | (1 << PINB1); // Set LEDs as output


    while ( 1 )
    {
		/* Test One second elapse */
        if ( timer1_GetTicks() >= (Tick + 10) )
        {
			Tick = timer1_GetTicks();
			
			//Lcd_DrawStringXY( "ABCDEFGHIJKLM", 0, 1 );

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
			
			// Convert outdoor temp to opening (%)
			PwmVanne = TempToValve( OutdoorTemp );
			
			sprintf_P(TempString, PSTR("Vanne: "));
			Lcd_DrawStringXY( TempString, 2, 0 );
			if ( Thermostat() != 0 && (PwmVanne > 0) )
			{
				PompOn();
				RampUp = PwmRampUp/256;
				sprintf_P(TempString, PSTR("%u/%u %%  "), RampUp, PwmVanne );
				PORTB |= (1 << PINB2);  // Red Led ON
				if ( RampUp < PwmVanne )
					PwmRampUp += ((MAX_OPENING*256)/MAX_OPENING_TIME);
			}
			else
			{
				PompOff();
				PwmRampUp = 0;
				sprintf_P(TempString, PSTR("Arret "));
				PORTB &= ~(1 << PINB2);  // Red Led OFF
			}
			
			Lcd_DrawStringXY( TempString, 48, 0 );

			/* Set pwm value (Range 0-256) */
			timerO_PWM_SetValue( PwmRampUp / 100 );
        }

    }

  return 0;
}

/* EOF  */



